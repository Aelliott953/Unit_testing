
template <typename T, int N>
size_t Channel<T, N>::run(std::vector<double>& inputs) {
    return adc.read_input(inputs);
}

template <typename T, int N>
std::vector<double> Channel<T, N>::get_values() {
    return adc.values();
}

template <typename T, int N>
std::string Channel<T, N>::to_string() {
    return adc.to_string();
}
