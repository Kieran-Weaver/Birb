#ifndef PRNG_HPP
#define PRNG_HPP
#include <random>
#include <functional>
#include <algorithm>
#include <iterator>
template<class T = std::mt19937, std::size_t N = T::state_size * T::word_size>
inline auto SeedRNG () -> typename std::enable_if<N, T>::type {
    uint32_t random_data[((N - 1) / 4) + 1];
    std::random_device source;
    std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    return T(seeds);
}
#endif
