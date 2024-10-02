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
    
    uint64_t res = 0;
    
    size_t duration_linear = 0;
    for (auto q : queries) {
        auto start = std::chrono::high_resolution_clock::now();
        res = *search::lower_bound_linear(data.begin(), data.end(), q);
        auto end = std::chrono::high_resolution_clock::now();
        duration_linear += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
    std::cout << "Search result " << res << std::endl;
    std::cout << "Query latency (linear) " << duration_linear / nq << std::endl;
    
    std::vector<uint64_t> data_cpy1(data);
    std::vector<uint64_t> queries_cpy1(queries);
    
    size_t duration_branchless = 0;
    for (auto q : queries_cpy1) {
        auto start = std::chrono::high_resolution_clock::now();
        res = *search::upper_bound_branchless(data_cpy1.begin(), data_cpy1.end(), q);
        auto end = std::chrono::high_resolution_clock::now();
        duration_branchless += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
    std::cout << "Search result " << res << std::endl;
    std::cout << "Query latency (branchless) " << duration_branchless / nq << std::endl;
    
    std::vector<uint64_t> data_cpy2(data);
    std::vector<uint64_t> queries_cpy2(queries);
    
    size_t duration_branchy = 0;
    for (auto q : queries_cpy2) {
        auto start = std::chrono::high_resolution_clock::now();
        res = *std::lower_bound(data_cpy2.begin(), data_cpy2.end(), q);
        auto end = std::chrono::high_resolution_clock::now();
        duration_branchy += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
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
    
    // make hard copy of data and queries
    // to avoid influence of hot cache
    std::vector<uint64_t> queries_cpy(queries);
    
    std::cout << "Construct PGM index eps_l=" << Epsilon << " eps_i=" << EpsilonRecursive << std::endl;
    pgm::PGMIndex<uint64_t, Epsilon, EpsilonRecursive, true, 8, float> index_branchless(data.begin(), data.end()-1);
    
    uint64_t res = 0;
    // branchless PGM without last-mile search
    size_t duration_branchless = 0;
    for (auto q : queries_cpy) {
        auto start = std::chrono::high_resolution_clock::now();
        res = index_branchless.search(q).pos;
        auto end = std::chrono::high_resolution_clock::now();
        duration_branchless += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
    // branchless PGM with last-mile search
    size_t duration_branchless_l = 0;
    for (auto q : queries_cpy) {
        auto start = std::chrono::high_resolution_clock::now();
        res = *index_branchless.search_data(data.begin(), q);
        auto end = std::chrono::high_resolution_clock::now();
        duration_branchless_l += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
    std::cout << "Search result: " << res << std::endl;
    std::cout << "PGM levels " << index_branchless.height()
              << " bytes " << index_branchless.size_in_bytes()
              << " LLS " << index_branchless.segments_count()
              << " ILS " << index_branchless.internal_segments_count() << std::endl;
    for (auto offset : index_branchless.get_levels_offsets()) {
        std::cout << offset << " ";
    }
    std::cout << std::endl;
    for (auto cnt : index_branchless.get_levels_segment_count()) {
        std::cout << cnt << " ";
    }
    std::cout << std::endl;
    std::cout << "Query latency (pgm index branchless) " << duration_branchless / nq << std::endl;
    std::cout << "Query latency all (pgm index branchless) " << duration_branchless_l / nq << std::endl;
    

    queries_cpy.clear();
    queries_cpy.shrink_to_fit();
    queries_cpy = queries;
    
    std::cout << "Construct PGM index eps_l=" << Epsilon << " eps_i=" << EpsilonRecursive << std::endl;
    pgm::PGMIndex<uint64_t, Epsilon, EpsilonRecursive, false, 0, float> index(data.begin(), data.end()-1);
    // branchy PGM without last-mile search
    size_t duration_branchy = 0;
    for (auto q : queries_cpy) {
        auto start = std::chrono::high_resolution_clock::now();
        res = index.search(q).pos;
        auto end = std::chrono::high_resolution_clock::now();
        duration_branchy += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
    
    // branchy PGM with last-mile search
    size_t duration_branchy_l = 0;
    for (auto q : queries_cpy) {
        auto start = std::chrono::high_resolution_clock::now();
        res = *index.search_data(data.begin(), q);
        auto end = std::chrono::high_resolution_clock::now();
        duration_branchy_l += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
    
    std::cout << "Search result: " << res << std::endl;
    std::cout << "PGM levels " << index.height()
              << " bytes " << index.size_in_bytes()
              << " LLS " << index.segments_count()
              << " ILS " << index.internal_segments_count() << std::endl;
    std::cout << "Query latency internal (pgm index branchy) " << duration_branchy / nq << std::endl;
    std::cout << "Query latency all (pgm index branchy) " << duration_branchy_l / nq << std::endl;
    
    
    return stats {Epsilon, EpsilonRecursive, index.height(), index.size_in_bytes(), index.segments_count(), index.internal_segments_count(), duration_branchy/nq, duration_branchless/nq, duration_branchy_l/nq, duration_branchless_l/nq};
}


int main(int argc, const char * argv[]) {
    const std::string fname = argv[1];
    const size_t nq = 5000;
    const size_t repeat = 10;
    
    std::cout << "Load data from " << fname << std::endl;
    auto data = benchmark::load_data<uint64_t>(fname);
    std::sort(data.begin(), data.end());
    
    std::vector<std::pair<size_t, stats>> bench_results;
    
    for (auto i=0; i<repeat; ++i) {
        std::cout << "Round " << i << std::endl;
        std::cout << "Generate " << nq << " random search keys." << std::endl;
        auto queries = benchmark::gen_random_queries(data, nq);
        
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
        
        bench_results.emplace_back(i, bench_pgm<4, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 256>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 256>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 512>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 512>(data, queries));
        
        bench_results.emplace_back(i, bench_pgm<4, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<8, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<16, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<32, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<64, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<128, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<256, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<512, 1024>(data, queries));
        bench_results.emplace_back(i, bench_pgm<1024, 1024>(data, queries));
    }
    
    std::ofstream ofs(argv[2]);
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
    
    ofs.close();

    return 0;
}
