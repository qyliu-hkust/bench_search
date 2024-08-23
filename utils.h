//
//  utils.h
//  bench_search
//
//  Created by Liu Qiyu on 2024/8/19.
//

#ifndef utils_h
#define utils_h

#include <random>
#include <numeric>

namespace benchmark {
static uint64_t timing(std::function<void()> fn) {
    const auto start = std::chrono::high_resolution_clock::now();
    fn();
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


// Loads values from binary file into vector.
template <typename T>
static std::vector<T> load_data(const std::string& filename, bool print = true, size_t sample_size = 0) {
    std::vector<T> data;
    const uint64_t ns = timing([&] {
        std::ifstream in(filename, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "unable to open " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
        // Read size.
        uint64_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
        data.resize(size);
        // Read values.
        in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));
        in.close();
    });
    const uint64_t ms = ns / 1e6;

    if (print) {
    std::cout << "read " << data.size() << " values from " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
    }
    
    if (sample_size > 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::vector<T> sample;
        std::sample(data.begin(), data.end()-1, std::back_inserter(sample), sample_size, gen);
        return sample;
    } else {
        return data;
    }
}

template <typename K>
auto get_data_stats(const std::vector<K>& data) {
    std::vector<K> gaps;
    for (auto i=1; i<data.size(); ++i) {
        gaps.emplace_back(data[i]-data[i-1]);
    }
    const auto n = gaps.size();
    double mean = std::accumulate(gaps.begin(), gaps.end(), 0.0)/n;
    double sq_sum = std::accumulate(gaps.begin(), gaps.end(), 0.0, [mean](double acc, double val) {
        return acc + (val - mean) * (val - mean);
    });
    double var = sq_sum/n;
    struct data_stats {double mean; double var;};
    return data_stats {mean, var};
}


template<typename K>
std::vector<K> gen_random_keys(const size_t& n, const K& max) {
    std::vector<K> data(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<K> dis(0, max);
    for (auto i=0; i<data.size(); ++i) {
        data[i] = dis(gen);
    }
    return data;
}

template<typename K>
std::vector<K> gen_random_queries(const std::vector<K>& data, const size_t& nq) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<K> sample;
    std::sample(data.begin(), data.end()-1, std::back_inserter(sample), nq, gen);
    return sample;
}

}


#endif /* utils_h */
