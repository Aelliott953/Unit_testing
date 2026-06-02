#include <gtest/gtest.h>
#include "Channel.h"

// ---------------------------------------------------------------------------
// Test Fixture
// ---------------------------------------------------------------------------
struct ChannelFixture : public testing::Test {
    ChannelFixture() {
        // You can do set-up work for each test here.
    }
    ~ChannelFixture() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }
    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Objects declared here can be used by all tests in the test suite for Channel.
    std::vector<double> inputs { 1., 2., 3., 4., 1.5, 1.99, 2.5, 3.5, 4.5, 4.99 };
    int bits { 16 };
    std::chrono::milliseconds clock_rate { 100 };
    double rangeMin { -5. }, rangeMax { 5. };
    double sensitivity { 1. }, trigger_level { 2. };
    ADC<short, 4> adc { bits, clock_rate, rangeMin, rangeMax, sensitivity, trigger_level };
    Channel<short, 4> channel { std::move(adc) };
};

// ===========================================================================
// Channel Tests
// ===========================================================================

// ---------------------------------------------------------------------------
// 1. Initialization / Constructor
// ---------------------------------------------------------------------------
TEST_F(ChannelFixture, Initialization) {
    EXPECT_EQ(bits,          channel.get_adc().bits);
    EXPECT_EQ(rangeMin,      channel.get_adc().rangeMin);
    EXPECT_EQ(rangeMax,      channel.get_adc().rangeMax);
    EXPECT_EQ(sensitivity,   channel.get_adc().sensitivity);
    EXPECT_EQ(trigger_level, channel.get_adc().trigger_level);
}

TEST_F(ChannelFixture, InitializationClockRate) {
    EXPECT_EQ(clock_rate, channel.get_adc().clock_rate);
}

TEST_F(ChannelFixture, InitialSamplesEmpty) {
    // Before run(), there should be no collected samples.
    EXPECT_TRUE(channel.get_adc().samples.empty());
}

// ---------------------------------------------------------------------------
// 2. Channel::run  (delegates to ADC::read_input)
// ---------------------------------------------------------------------------

// Normal case – values above trigger_level should be stored.
TEST_F(ChannelFixture, RunCountsAboveTrigger) {
    // From inputs {1,2,3,4,1.5,1.99,2.5,3.5,4.5,4.99}, values >= 2.0:
    // 2, 3, 4, 2.5, 3.5, 4.5, 4.99  => 7 samples
    size_t count = channel.run(inputs);
    EXPECT_EQ(7u, count);
}

TEST_F(ChannelFixture, RunSamplesSizeMatchesCount) {
    size_t count = channel.run(inputs);
    EXPECT_EQ(count, channel.get_adc().samples.size());
}

TEST_F(ChannelFixture, RunEmptyInput) {
    std::vector<double> empty;
    EXPECT_EQ(0u, channel.run(empty));
    EXPECT_TRUE(channel.get_adc().samples.empty());
}

TEST_F(ChannelFixture, RunAllBelowTrigger) {
    std::vector<double> belowTrigger { -5.0, -3.0, 0.0, 1.99 };
    EXPECT_EQ(0u, channel.run(belowTrigger));
    EXPECT_TRUE(channel.get_adc().samples.empty());
}

TEST_F(ChannelFixture, RunAllAboveTrigger) {
    std::vector<double> aboveTrigger { 2.0, 2.5, 3.0, 4.0, 5.0 };
    EXPECT_EQ(5u, channel.run(aboveTrigger));
}

TEST_F(ChannelFixture, RunExactlyAtTriggerLevel) {
    std::vector<double> atTrigger { 2.0 };
    EXPECT_EQ(1u, channel.run(atTrigger));
}

TEST_F(ChannelFixture, RunJustBelowTriggerLevel) {
    std::vector<double> justBelow { 1.999 };
    EXPECT_EQ(0u, channel.run(justBelow));
}

TEST_F(ChannelFixture, RunMaxRangeValue) {
    std::vector<double> maxVal { rangeMax };
    EXPECT_EQ(1u, channel.run(maxVal));
}

// ---------------------------------------------------------------------------
// 3. Channel::get_values
// ---------------------------------------------------------------------------

TEST_F(ChannelFixture, GetValuesEmptyBeforeRun) {
    EXPECT_TRUE(channel.get_values().empty());
}

TEST_F(ChannelFixture, GetValuesSizeMatchesSamples) {
    channel.run(inputs);
    auto vals = channel.get_values();
    EXPECT_EQ(channel.get_adc().samples.size(), vals.size());
}

// Values should be within [rangeMin*sensitivity, rangeMax*sensitivity].
TEST_F(ChannelFixture, GetValuesWithinRange) {
    channel.run(inputs);
    auto vals = channel.get_values();
    for (double v : vals) {
        EXPECT_GE(v, rangeMin * sensitivity);
        EXPECT_LE(v, rangeMax * sensitivity);
    }
}

// Single known input: 5.0 V (max range) -> value should be ~5.0 * sensitivity = 5.0.
TEST_F(ChannelFixture, GetValuesSingleMaxInput) {
    std::vector<double> single { rangeMax };
    channel.run(single);
    auto vals = channel.get_values();
    ASSERT_EQ(1u, vals.size());
    EXPECT_NEAR(rangeMax * sensitivity, vals[0], 0.01);
}

// ---------------------------------------------------------------------------
// 4. Channel::to_string
// ---------------------------------------------------------------------------

TEST_F(ChannelFixture, ToStringNotEmpty) {
    EXPECT_FALSE(channel.to_string().empty());
}

TEST_F(ChannelFixture, ToStringContainsBits) {
    std::string s = channel.to_string();
    EXPECT_NE(std::string::npos, s.find(std::to_string(bits)));
}

TEST_F(ChannelFixture, ToStringAfterRun) {
    channel.run(inputs);
    std::string s = channel.to_string();
    EXPECT_FALSE(s.empty());
    // After run(), samples size (7) should appear somewhere in the string.
    EXPECT_NE(std::string::npos, s.find("7"));
}

// ===========================================================================
// ADC Tests  (tested through ChannelFixture's adc member)
// ===========================================================================

// ---------------------------------------------------------------------------
// ADC::convert
// ---------------------------------------------------------------------------

// Convert minimum voltage -> should give 0 (or near 0).
TEST_F(ChannelFixture, ConvertMinVoltage) {
    short result = channel.get_adc().convert(rangeMin);
    EXPECT_EQ(0, result);
}

// Convert maximum voltage -> should give max ADC code (2^bits - 1).
TEST_F(ChannelFixture, ConvertMaxVoltage) {
    short maxCode = static_cast<short>((1 << bits) - 1);
    short result = channel.get_adc().convert(rangeMax);
    EXPECT_EQ(maxCode, result);
}

// Convert mid-range voltage -> should give ~half the ADC range.
TEST_F(ChannelFixture, ConvertMidVoltage) {
    double midV = (rangeMin + rangeMax) / 2.0;   // 0.0 V for [-5, 5]
    short result = channel.get_adc().convert(midV);
    short expected = static_cast<short>(((1 << bits) - 1) / 2);
    // Allow ±1 for integer rounding.
    EXPECT_NEAR(expected, result, 1);
}

// Voltage below range should be clamped to minimum code.
TEST_F(ChannelFixture, ConvertBelowRangeClamped) {
    short result = channel.get_adc().convert(rangeMin - 10.0);
    EXPECT_EQ(0, result);
}

// Voltage above range should be clamped to maximum code.
TEST_F(ChannelFixture, ConvertAboveRangeClamped) {
    short maxCode = static_cast<short>((1 << bits) - 1);
    short result = channel.get_adc().convert(rangeMax + 10.0);
    EXPECT_EQ(maxCode, result);
}

// ---------------------------------------------------------------------------
// ADC::read_input  (directly)
// ---------------------------------------------------------------------------

TEST_F(ChannelFixture, ReadInputReturnsCorrectCount) {
    std::vector<double> inp { 3.0, 1.5, 4.5 }; // 3.0 and 4.5 are >= 2.0
    size_t count = channel.get_adc().read_input(inp);
    EXPECT_EQ(2u, count);
}

TEST_F(ChannelFixture, ReadInputAppendsToSamples) {
    std::vector<double> first  { 2.5, 3.5 };
    std::vector<double> second { 4.0 };
    channel.get_adc().read_input(first);
    channel.get_adc().read_input(second);
    EXPECT_EQ(3u, channel.get_adc().samples.size());
}

TEST_F(ChannelFixture, ReadInputEmptyVector) {
    std::vector<double> empty;
    EXPECT_EQ(0u, channel.get_adc().read_input(empty));
}

TEST_F(ChannelFixture, ReadInputBoundaryAtTrigger) {
    std::vector<double> inp { trigger_level };
    EXPECT_EQ(1u, channel.get_adc().read_input(inp));
}

TEST_F(ChannelFixture, ReadInputBoundaryJustBelowTrigger) {
    std::vector<double> inp { trigger_level - 0.001 };
    EXPECT_EQ(0u, channel.get_adc().read_input(inp));
}

// ---------------------------------------------------------------------------
// ADC::value
// ---------------------------------------------------------------------------

TEST_F(ChannelFixture, ValueRoundTrip) {
    // A voltage above trigger is converted and then recovered via value().
    double inputVoltage = 3.0;
    std::vector<double> inp { inputVoltage };
    channel.get_adc().read_input(inp);
    // value() returns voltage * sensitivity; with sensitivity=1, should ~= inputVoltage.
    double recovered = channel.get_adc().value(0);
    EXPECT_NEAR(inputVoltage * sensitivity, recovered, 0.1);
}

TEST_F(ChannelFixture, ValueOutOfBoundsReturnsZero) {
    EXPECT_DOUBLE_EQ(0.0, channel.get_adc().value(0));
}

TEST_F(ChannelFixture, ValueWithMaxRangeInput) {
    std::vector<double> inp { rangeMax };
    channel.get_adc().read_input(inp);
    double recovered = channel.get_adc().value(0);
    EXPECT_NEAR(rangeMax * sensitivity, recovered, 0.1);
}

TEST_F(ChannelFixture, ValueWithMinRangeInput) {
    std::vector<double> inp { trigger_level };
    channel.get_adc().read_input(inp);
    double recovered = channel.get_adc().value(0);
    EXPECT_NEAR(trigger_level * sensitivity, recovered, 0.1);
}
