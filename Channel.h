#pragma once

#include "Adc.h"

template <typename T = short, int N = 1024>
class Channel {
    ADC<T, N>& adc;
public:
    Channel(ADC<T, N>&& adc) : adc(adc) {
    }
    virtual ~Channel() = default;
    virtual size_t run(std::vector<double>& inputs);
    virtual std::vector<double> get_values();
    virtual std::string to_string();
    virtual ADC<T, N>& get_adc() {
        return adc;
    }
};

#include "Channel.cpp"
