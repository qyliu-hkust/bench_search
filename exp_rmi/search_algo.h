//
//  search_algo.h
//  bench_search
//
//  Created by Liu Qiyu on 2024/8/18.
//

#ifndef search_algo_h
#define search_algo_h

//#define IS_PREFETCH

#include <functional>

namespace search {
template<typename RandomIt, typename K>
inline RandomIt lower_bound_linear(RandomIt start, RandomIt end, const K& key) {
    auto it = start;
    while (it != end) {
        if (*it >= key)
            return it;
        ++it;
    }
    return end;
}


template<typename RandomIt, typename K>
inline RandomIt lower_bound_branchless(RandomIt start, RandomIt end, const K& key) {
    auto base = start;
    auto n = std::distance(start, end);
    
    while (n > 1) {
        auto half = n / 2;
        n -= half;
        
#ifdef IS_PREFETCH
        __builtin_prefetch(&base[n / 2 - 1]);
        __builtin_prefetch(&base[half + n / 2 - 1]);
#endif
        
        base = (base[half - 1] < key) ? base + half : base; // w.r.t. cmov instruction
    }
    
    return base;
}

template<typename RandomIt, typename K, size_t search_bound=64>
inline RandomIt lower_bound_interpolation(RandomIt start, RandomIt end, const K& key) {
    auto lo = start, hi = std::prev(end);
    auto max = *hi;
    
    while (lo < hi && key >= *lo && key <= *hi) {
        auto n = std::distance(lo, hi);
        if (n < search_bound)
            break;
            
        auto lo_key = *lo, hi_key = *hi;
        auto est_pos_rel = (double)(key - lo_key)/(float)(hi_key - lo_key);
        auto offset = (size_t)(est_pos_rel * n);
        auto mid = lo + offset;
        
        if (mid == lo)
            break;
        
        if (key < *mid) {
            hi = mid;
        } else if (key > *mid) {
            lo = mid;
        } else {
            lo = mid;
            hi = mid + 1;
            while (lo != start && *(lo - 1) == key)
                lo--;
            break;
        }
    }
    
    auto it = std::lower_bound(lo, hi, key);
    return (key > max) ? std::next(it) : it;
}


template<typename RandomIt, typename K>
inline RandomIt upper_bound_branchless(RandomIt start, RandomIt end, const K& key) {
    auto base = start;
    auto n = std::distance(start, end);
    
    while (n > 1) {
        auto half = n / 2;
        n -= half;
        base = (base[half-1] <= key) ? base + half : base; // w.r.t. cmov instruction
    }

    return base;
}
}

template<typename K>
void test_lower_bound(std::vector<K> data, std::vector<K> queries) {
    std::sort(data.begin(), data.end());
    bool flag = 1;
    for (auto i=0; i<queries.size(); ++i) {
        auto res_lb_new = search::lower_bound_branchless(data.begin(), data.end(), queries[i]);
        auto res_lb_std = std::lower_bound(data.begin(), data.end(), queries[i]);
        
        if (*res_lb_new != *res_lb_std) {
            std::cout << "query " << i << " res_lb_new " << res_lb_new - data.begin() << " res_lb_std " << res_lb_std - data.begin() << std::endl;
            flag = 0;
        }
    }
    if (flag) {
        std::cout << "pass test." << std::endl;
    }
}

template<typename K>
void test_upper_bound(std::vector<K> data, std::vector<K> queries) {
    std::sort(data.begin(), data.end());
    bool flag = 1;
    
    for (auto i=0; i<queries.size(); ++i) {
        auto res_ub_new = search::upper_bound_branchless(data.begin(), data.end(), queries[i]);
        auto res_ub_std = std::upper_bound(data.begin(), data.end(), queries[i]);
        if (res_ub_new != res_ub_std) {
            std::cout << "query " << queries[i] << " res_ub_new " << *res_ub_new << " res_ub_std " << *res_ub_std << std::endl;
            flag = 0;
        }
    }
    if (flag) {
        std::cout << "pass test." << std::endl;
    }
}



#endif /* search_algo_h */
