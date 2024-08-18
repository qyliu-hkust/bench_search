//
//  main.cpp
//  bench_search
//
//  Created by Liu Qiyu on 2024/8/17.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include "pgm_index.h"
#include "search_algo.h"


void gen_random_keys_uint64(std::vector<uint64_t>& data) {
    std::generate(data.begin(), data.end(), std::rand);
}


auto bench(size_t n, size_t nq) {
    std::cout << "===========================================" << std::endl;
    std::cout << "Generate and sort " << n << " uniform keys." << std::endl;
    std::vector<uint64_t> data(n);
    gen_random_keys_uint64(data);
    std::sort(data.begin(), data.end());
    
    std::cout << "Generate " << nq << " random search keys." << std::endl;
    std::vector<uint64_t> queries(nq);
    gen_random_keys_uint64(queries);
    
    volatile uint64_t res = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        res = *search::lower_bound_linear(data.begin(), data.end(), q);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_linear = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Query latency (linear) " << duration_linear / nq << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        res = *search::upper_bound_branchless(data.begin(), data.end(), q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchless = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Query latency (branchless) " << duration_branchless / nq << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        res = *std::lower_bound(data.begin(), data.end(), q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchy = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Query latency (branchy) " << duration_branchy / nq << std::endl;
    
    pgm::PGMIndex<uint64_t> index(data);
    
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        auto approx = index.search(q);
        res = *std::lower_bound(data.begin()+approx.lo, data.begin()+approx.hi, q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_pgm = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "PGM levels " << index.height()
              << " LLS " << index.segments_count()
              << " ILS " << index.internal_segments_count() << std::endl;
    std::cout << "Query latency (pgm index) " << duration_pgm / nq << std::endl;
    
    
    struct timer {size_t brl; size_t br; size_t linear;};
    return timer {duration_branchless/nq, duration_branchy/nq, duration_linear/nq};
}


int main(int argc, const char * argv[]) {
    size_t n_list[21] = {10, 20, 30, 60, 130, 260, 510, 1020, 2050, 4100, 8190, 16380, 32770, 65540, 131070, 262140, 524290, 1048580, 2097150, 4194300, 8388610};
    size_t brl_list[21] = {};
    size_t br_list[21] = {};
    size_t linear_list[21] = {};
    
    std::ofstream ofs("/Users/liuqiyu/Desktop/bench_search_result.csv");
    ofs << "round,n,brl,br,linear" << std::endl;
    for (auto n_round=0; n_round<20; ++n_round) {
        for (auto i=0; i<21; ++i) {
            auto [brl, br, linear] = bench(n_list[i], 100);
            brl_list[i] = brl;
            br_list[i] = br;
            linear_list[i] = linear;
        }
        for (auto i=0; i<21; ++i) {
            ofs << n_round << ","
                << n_list[i] << ","
                << brl_list[i] << ","
                << br_list[i] << ","
                << linear_list[i] << std::endl;
        }
    }
    ofs.close();
    
//    std::vector<uint64_t> data(10000, 1);
//    for (auto i=0; i<10000; ++i) {
//        data.emplace_back(10);
//    }
//    std::vector<uint64_t> queries(1000, 10000);
//    std::generate(data.begin(), data.end(), std::rand);
//    std::generate(queries.begin(), queries.end(), std::rand);
//    test_lower_bound(data, queries);
//    test_upper_bound(data, queries);

    
    return 0;
}
