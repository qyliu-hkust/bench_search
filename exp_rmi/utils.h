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
//从二进制文件中加载数据到一个 std::vector中：data
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
        //从文件中读取一个 uint64_t 大小的数据，并将其存储在 size 变量中。这通常表示文件中首先存储了数据的大小。
        in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
        data.resize(size);//根据读取的大小调整 data 向量的大小。
        // Read values.
        //从文件中读取 size 个 T 类型的数据，并将其存储在 data 向量中。
        in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));
        in.close();
    });
    const uint64_t ms = ns / 1e6;

    if (print) {
    std::cout << "read " << data.size() << " values from " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
    }
    
    //如果 sample_size 大于 0，则返回一个数据样本，否则返回完整数据
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
void save_data(const std::vector<K>& data, const std::string& filename, bool print = true) {
    const size_t n = data.size();
    const uint64_t ns = timing([&] {
        std::ofstream out(filename, std::ios::binary | std::ios::out);
        // write data size
        out.write(reinterpret_cast<const char*>(&n), sizeof(size_t));
        for (auto d : data) {
            out.write(reinterpret_cast<const char*>(&d), sizeof(K));
        }
        out.close();
    });
    const uint64_t ms = ns / 1e6;
    if (print) {
    std::cout << "save " << data.size() << " values to " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
    }
}


template <typename K>
auto get_data_stats(const std::vector<K>& data) {
    std::vector<K> gaps;
    for (auto i=1; i<data.size()-1; ++i) {
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

std::vector<uint64_t> gen_random_keys_on_gaps(const size_t& n, const uint64_t& min, const uint64_t& max) {
    std::vector<uint64_t> data(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(min, max);
    
    data[0] = 0;
    for (auto i=1; i<data.size(); ++i) {
        data[i] = data[i-1] + dis(gen);
    }
    return data;
}

template<typename K> 
//从给定的数据集中生成随机查询样本，输入：数据集和 样本大小nq
//从数据集中随机抽取 nq 个元素，返回一个包含这些随机查询样本的向量
std::vector<K> gen_random_queries(const std::vector<K>& data, const size_t& nq) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<K> sample;
    std::sample(data.begin(), data.end()-1, std::back_inserter(sample), nq, gen);
    return sample;
}

}


#endif /* utils_h */
