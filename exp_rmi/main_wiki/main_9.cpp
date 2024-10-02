
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <limits>
#include "search_algo.h"
#include "utils.h"
#include "wiki_ts_200M_uint64_9.h"
#include "wiki_ts_200M_uint64_9_data.h"
#include <cassert> 


bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}


double rand_val(int seed) {
    static bool first = true; 
    if (first) {
        srand(seed == 0 ? time(NULL) : seed);
        first = false;
    }
    return ((double) rand() / (double) RAND_MAX);
}

int zipf(double alpha, int n) {
    static bool first = true;     
    static double c = 0;          // Normalization constant
    static double *sum_probs;     // Pre-calculated sum of probabilities
    double z;                     // Uniform random number (0 < z < 1)
    int zipf_value;               // Computed exponential value to be returned
    int i;                        // Loop counter
    int low, high, mid;           // Binary-search bounds

    // Compute normalization constant on first call only
    if (first) {
        for (i = 1; i <= n; i++) {
            c = c + (1.0 / pow((double) i, alpha));
        }
        c = 1.0 / c;

        sum_probs = (double *)malloc((n + 1) * sizeof(*sum_probs));
        sum_probs[0] = 0;
        for (i = 1; i <= n; i++) {
            sum_probs[i] = sum_probs[i - 1] + c / pow((double) i, alpha);
        }
        first = false;
    }

    // Pull a uniform random number (0 < z < 1)
    do {
        z = rand_val(0);
    } while ((z == 0) || (z == 1));

    // Map z to the value
    low = 1, high = n, mid = 0;
    do {
        mid = (low + high) / 2;
        if (sum_probs[mid] >= z && sum_probs[mid - 1] < z) {
            zipf_value = mid;
            break;
        } else if (sum_probs[mid] >= z) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    } while (low <= high);

    // Assert that zipf_value is between 1 and N
    assert((zipf_value >= 1) && (zipf_value <= n));

    return zipf_value;
}

std::vector<uint64_t> gen_zipfian_queries(const std::vector<uint64_t>& data, size_t nq, double alpha) {
    std::vector<uint64_t> queries;
    queries.reserve(nq);

    for (size_t i = 0; i < nq; ++i) {
        int index = zipf(alpha, data.size()) - 1;
        queries.push_back(data[index]);
    }

    return queries;
}

void append_results_to_csv(const std::string& filename, size_t sample_num, size_t search_time, size_t total_time, size_t err_total, size_t err_max, size_t nq) {
    std::ofstream file;
    bool exists = file_exists(filename);
    file.open(filename, std::ios::out | std::ios::app);
    if (file.is_open()) {
        if (!exists) {
            file << "sample_num,RMI search time,RMI total time,RMI avg error,RMI max error,RMI size\n";
        }
        file << sample_num << "," 
             << search_time / nq << "," 
             << total_time / nq << "," 
             << err_total / nq << "," 
             << err_max << "," 
             << wiki_ts_200M_uint64_9::RMI_SIZE << "\n";
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}


int main(int argc, const char * argv[]) {
    const std::string fname = "wiki_ts_200M_uint64"; 
    
    const size_t nq = 10000;
    const size_t repeat = 10;
    const double alpha = 1.3;

    std::cout << "Load data from " << fname << std::endl;
    auto data = benchmark::load_data<uint64_t>(fname);
    std::sort(data.begin(), data.end());
    
    auto data_stats = benchmark::get_data_stats(data);
    std::cout << "mean: " << data_stats.mean 
              << " variance: " << data_stats.var
              << " hardness ratio: " << data_stats.var/(data_stats.mean*data_stats.mean) << std::endl;
    
   
    wiki_ts_200M_uint64_9::load("RMI_output_wiki"); 

    std::string zipf_filename = "result/wiki_ts_200M_uint64_zipfan_results.csv";
    std::string rand_filename = "result/wiki_ts_200M_uint64_random_results.csv";

    for (size_t query_type = 0; query_type < 2; ++query_type) { 
        std::string filename;
        if (query_type == 0) {
            filename = zipf_filename;
            std::cout << "Running Zipfian queries..." << std::endl;
        } else {
            filename = rand_filename;
            std::cout << "Running random queries..." << std::endl;
        }
        
        for (size_t i = 0; i < repeat; ++i) {
            std::vector<uint64_t> queries;
            if (query_type == 0) {
                queries = gen_zipfian_queries(data, nq, alpha); 
            } else {
                queries = benchmark::gen_random_queries(data, nq); 
            }

            size_t search_time = 0;
            size_t total_time = 0;
            size_t err_total = 0;
            size_t err_max = 0;

            for (auto q : queries) {
                size_t err = 0;
                auto start = std::chrono::high_resolution_clock::now();
                auto res = wiki_ts_200M_uint64_9::lookup(q, &err);
                auto end = std::chrono::high_resolution_clock::now();
                search_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                err_total += err;
                err_max = err > err_max ? err : err_max;
                start = std::chrono::high_resolution_clock::now();
                size_t lower_bound_index = (res > err) ? res - err : 0;
                size_t upper_bound_index = (res + err < data.size()) ? res + err : data.size() - 1;
                res = *std::lower_bound(data.begin() + lower_bound_index, data.begin() + upper_bound_index, q);
                end = std::chrono::high_resolution_clock::now();
                total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            }

            total_time += search_time;

            std::cout << " Sample " << i << ": "
                      << " RMI search time: " << search_time / nq 
                      << " RMI total time: " << total_time / nq
                      << " RMI avg error: " << err_total / nq
                      << " RMI max error: " << err_max
                      << " RMI size: " << wiki_ts_200M_uint64_9::RMI_SIZE
                      << std::endl;
            
            append_results_to_csv(filename, i, search_time, total_time, err_total, err_max, nq);
        }
    }

    wiki_ts_200M_uint64_9::cleanup();
    // exit(0);
    return 0;
}

