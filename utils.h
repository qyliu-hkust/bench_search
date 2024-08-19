//
//  utils.h
//  bench_search
//
//  Created by Liu Qiyu on 2024/8/19.
//

#ifndef utils_h
#define utils_h


namespace benchmark {
static uint64_t timing(std::function<void()> fn) {
    const auto start = std::chrono::high_resolution_clock::now();
    fn();
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


// Loads values from binary file into vector.
template <typename T>
static std::vector<T> load_data(const std::string& filename,
                                bool print = true) {
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

    return data;
}

template<typename K>
std::vector<K> gen_random_keys(const size_t& n) {
    std::vector<uint64_t> data(n);
    std::generate(data.begin(), data.end(), std::rand);
    return data;
}

}


#endif /* utils_h */
