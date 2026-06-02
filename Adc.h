#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>

template <typename T = short, int N = 1024>
struct ADC {
    int bits;                                   // number of bits of the ADC
    std::chrono::milliseconds clock_rate;        // clock rate in ms
    double rangeMin, rangeMax;                  // range of the ADC in volts
    double sensitivity;                         // sensitivity in unit/volts
    double trigger_level;                       // trigger level in volts
    std::vector<T> samples;

public:
    // convert the input voltage to the corresponding ADC value
    // Uses long to avoid overflow when bits == 16 and T == short.
    T convert(double voltage) {
        if (voltage < rangeMin) voltage = rangeMin;
        if (voltage > rangeMax) voltage = rangeMax;
        double normalized = (voltage - rangeMin) / (rangeMax - rangeMin);
        long maxVal = (1L << bits) - 1;
        return static_cast<T>(static_cast<long>(normalized * maxVal));
    }

    // read input values and if any of them is above
    // the trigger_level, append it to the 'samples'
    size_t read_input(std::vector<double>& inputs) {
        size_t count = 0;
        for (double v : inputs) {
            if (v >= trigger_level) {
                samples.push_back(convert(v));
                ++count;
            }
        }
        return count;
    }

    // evaluate the sample true value of the measured quantity
    double value(size_t i) {
        if (i >= samples.size()) return 0.0;
        long maxVal = (1L << bits) - 1;
        // Treat the stored sample as an unsigned code in [0, maxVal].
        long code = static_cast<long>(static_cast<unsigned short>(samples[i]));
        double voltage = (static_cast<double>(code) / maxVal)
                         * (rangeMax - rangeMin) + rangeMin;
        return voltage * sensitivity;
    }

    // evaluate true values for all collected samples
    std::vector<double> values() {
        std::vector<double> result;
        for (size_t i = 0; i < samples.size(); ++i) {
            result.push_back(value(i));
        }
        return result;
    }

    // generate string representing the ADC data
    std::string to_string() {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        oss << "ADC[bits=" << bits
            << ", clock=" << clock_rate.count() << "ms"
            << ", range=[" << rangeMin << "," << rangeMax << "]"
            << ", sensitivity=" << sensitivity
            << ", trigger=" << trigger_level
            << ", samples=" << samples.size() << "]";
        return oss.str();
    }
};
