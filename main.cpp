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
#include <limits>
#include "pgm_index.h"
#include "search_algo.h"
#include "utils.h"


auto bench_search(const size_t& n, const size_t& nq) {
    std::cout << "====== n=" << n << " nq=" << nq << " ======" << std::endl;
    auto data = benchmark::gen_random_keys<uint64_t>(n, std::numeric_limits<uint64_t>::max());
    std::sort(data.begin(), data.end());
    auto queries = benchmark::gen_random_keys<uint64_t>(nq, std::numeric_limits<uint64_t>::max());
    
    volatile uint64_t res = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        res = *search::lower_bound_linear(data.begin(), data.end(), q);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_linear = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Search result " << res << std::endl;
    std::cout << "Query latency (linear) " << duration_linear / nq << std::endl;
    
    std::vector<uint64_t> data_cpy1(data);
    std::vector<uint64_t> queries_cpy1(queries);
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries_cpy1) {
        res = *search::upper_bound_branchless(data_cpy1.begin(), data_cpy1.end(), q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchless = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Search result " << res << std::endl;
    std::cout << "Query latency (branchless) " << duration_branchless / nq << std::endl;
    
    std::vector<uint64_t> data_cpy2(data);
    std::vector<uint64_t> queries_cpy2(queries);
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries_cpy2) {
        res = *std::lower_bound(data_cpy2.begin(), data_cpy2.end(), q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchy = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Search result " << res << std::endl;
    std::cout << "Query latency (branchy) " << duration_branchy / nq << std::endl;
    
    struct timer {size_t brl; size_t br; size_t linear;};
    return timer {duration_branchless/nq, duration_branchy/nq, duration_linear/nq};
}

void bench_search_repeat(const size_t& repeat, const size_t& nq, const std::string& ouput_fname) {
    const size_t n_max = 21;
    size_t n_list[n_max];
    for (auto i=1; i<=21; ++i) {
        n_list[i-1] = pow(2, i);
    }
    
    std::ofstream ofs(ouput_fname);
    ofs << "n,round,brl,br,linear" << std::endl;
    
    for (auto n : n_list) {
        for (auto i=0; i<repeat; ++i) {
            auto res = bench_search(n, nq);
            ofs << n << "," << i << "," << res.brl << "," << res.br << "," << res.linear << std::endl;
        }
    }
    
    ofs.close();
}

struct stats {
    size_t eps_l;
    size_t eps_i;
    size_t levels;
    size_t bytes;
    size_t lls;
    size_t ils;
    size_t latency_branchy_i;
    size_t latency_branchless_i;
    size_t latency_branchy_l;
    size_t latency_branchless_l;
};


template<size_t Epsilon, size_t EpsilonRecursive>
auto bench_pgm(const std::vector<uint64_t>& data, const std::vector<uint64_t>& queries) {
    std::cout << "===========================================" << std::endl;
    auto nq = queries.size();
    
    volatile uint64_t res = 0;
    
    std::cout << "Construct PGM index eps_l=" << Epsilon << " eps_i=" << EpsilonRecursive << std::endl;
    pgm::PGMIndex<uint64_t, Epsilon, EpsilonRecursive, true, 16, float> index_branchless(data.begin(), data.end()-1);
    
    // branchless PGM without last-mile search
    auto start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        res = index_branchless.search(q).pos;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_branchless = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
    // branchless PGM with last-mile search
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries) {
        res = *index_branchless.search_data(data.begin(), q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchless_l = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "Search result " << res << std::endl;
    std::cout << "PGM levels " << index_branchless.height()
              << " bytes " << index_branchless.size_in_bytes()
              << " LLS " << index_branchless.segments_count()
              << " ILS " << index_branchless.internal_segments_count() << std::endl;
    std::cout << "Query latency (pgm index branchless) " << duration_branchless / nq << std::endl;
    std::cout << "Query latency all (pgm index branchless) " << duration_branchless_l / nq << std::endl;
    
    // make hard copy of data and queries
    // to avoid influence of hot cache
    std::vector<uint64_t> data_cpy(data);
    std::vector<uint64_t> queries_cpy(queries);
    std::cout << "Construct PGM index eps_l=" << Epsilon << " eps_i=" << EpsilonRecursive << std::endl;
    pgm::PGMIndex<uint64_t, Epsilon, EpsilonRecursive, false, 16, float> index(data_cpy.begin(), data_cpy.end()-1);
    // branchy PGM without last-mile search
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries_cpy) {
        res = index.search(q).pos;
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchy = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    // branchy PGM with last-mile search
    start = std::chrono::high_resolution_clock::now();
    for (auto q : queries_cpy) {
        res = *index.search_data(data_cpy.begin(), q);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_branchy_l = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "Search result " << res << std::endl;
    std::cout << "PGM levels " << index.height()
              << " bytes " << index.size_in_bytes()
              << " LLS " << index.segments_count()
              << " ILS " << index.internal_segments_count() << std::endl;
    std::cout << "Query latency internal (pgm index branchy) " << duration_branchy / nq << std::endl;
    std::cout << "Query latency all (pgm index branchy) " << duration_branchy_l / nq << std::endl;
    
    
    return stats {Epsilon, EpsilonRecursive, index.height(), index.size_in_bytes(), index.segments_count(), index.internal_segments_count(), duration_branchy/nq, duration_branchless/nq, duration_branchy_l/nq, duration_branchless_l/nq};
}


int main(int argc, const char * argv[]) {
//    bench_search_repeat(20, 500, "/Users/liuqiyu/Desktop/bench_search_result_new.csv");
//    exit(0);
    
    const std::string fname = "/Users/liuqiyu/Desktop/SOSD_data/books_800M_uint64";
    const size_t nq = 200;
    const size_t repeat = 10;
    
    std::cout << "Load data from " << fname << std::endl;
    auto data = benchmark::load_data<uint64_t>(fname);
    std::sort(data.begin(), data.end()-1);
    
    std::vector<std::pair<size_t, stats>> bench_results;
    
    for (auto i=0; i<repeat; ++i) {
        std::cout << "Round " << i << std::endl;
        std::cout << "Generate " << nq << " random search keys." << std::endl;
        auto queries = benchmark::gen_random_queries(data, 500);
        
        bench_results.emplace_back(i, bench_pgm<4, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 4>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 4>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 8>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 8>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 16>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 16>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 32>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 32>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 64>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 64>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 128>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 128>(data, queries));
    }
    
    // start from 7 cold cache config
    std::ofstream ofs("/Users/liuqiyu/Desktop/bench_pgm_result_books_repeat_10_0250.csv");
    ofs << "round,eps_l,eps_i,levels,lls,ils,latency_branchy_i,latency_branchy_l,latency_branchless_i,latency_branchless_l" << std::endl;
    
    for (auto br : bench_results) {
        ofs << br.first << ","
            << br.second.eps_l << ","
            << br.second.eps_i << ","
            << br.second.levels << ","
            << br.second.lls << ","
            << br.second.ils << ","
            << br.second.latency_branchy_i << ","
            << br.second.latency_branchy_l << ","
            << br.second.latency_branchless_i << ","
            << br.second.latency_branchless_l << std::endl;
    }
    

    return 0;
}
