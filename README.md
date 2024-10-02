#  Perhaps This is a Readme

Compile:

``

Run:

`./main data_file_path result_output_path`

Dataset:


# Why Are Learned Indexes So Effective but Sometimes Ineffective?
## Ⅰ. Benchmark Datasets (4 real datasets and 3 synthetic datasets)
We adopt 4 real datasets from SOSD [1], which can be directly downloaded via the following links.

Specifically:

* **[fb](https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/JGVF9A/EATHF7):** A set of user IDs randomly sampled from Facebook.
* **[wiki](https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/JGVF9A/SVN8PI):** A set of edit timestamp IDs committed to Wikipedia.
* **[books](https://www.dropbox.com/s/y2u3nbanbnbmg7n/books_800M_uint64.zst?dl=1):** A dataset of book popularity from Amazon.
* **[osm_cellids](https://www.dropbox.com/s/j1d4ufn4fyb4po2/osm_cellids_800M_uint64.zst?dl=1):** A set of cell IDs from OpenStreetMap.

We also generate 3 synthetic datasets by sampling from uniform, normal, and log-normal distributions, following a process similar to [1, 2]. All keys are stored as 64-bit unsigned integers (`uint64_t` in C++). 

```C++
cd ./data
bash gen_data.sh
```

**Statistics of benchmark datasets.**

| Dataset | Category | Keys | Raw Size | $h_D$ | $\overline{Cov}$ |
|---|---|---|---|---|---|
| fb | Real | 200 M | 1.6 GB | 3.88 | 94 |
| wiki | Real | 200 M | 1.6 GB | 1.77 | 877 |
| books | Real | 800 M | 6.4 GB | 5.39 | 101 |
| osm | Real | 800 M | 6.4 GB | 1.91 | 129 |
| uniform | Synthetic | 400 M | 3.2 GB | Varied | N.A. |
| normal | Synthetic | 400 M | 3.2 GB | Varied | N.A. |
| lognormal | Synthetic | 400 M | 3.2 GB | Varied | N.A. |


**References:**  
[1] Marcus, et al. SOSD: A Benchmark Suite for Similarity Search over Sorted Data. PVLDB, 2020.  
[2] Zhang, et al. Making Learned Indexes Practical: A Comprehensive Study on Data Distribution and Model Selection. PVLDB, 2024.  


## II. RUN RMI BENCHMARK

### 1. Generate RMI Models: 
RMI code: https://github.com/learnedsystems/RMI/tree/master  

The following scripts are invoked to generate a set of RMI model configs with various index sizes. Notably, `wiki_200M_uint64` can be replaced to other dataset name. 
```C++
cargo run --release -- --optimize optimizer_out_wiki.json wiki_200M_uint64
cargo run wiki_200M_uint64 --param-grid optimizer_out_wiki.json -d YOUR_RMI_SAVE_FOLDER --threads 8 --zero-build-time
```
The above process will generate **9** RMI models for each dataset in **YOUR_RMI_SAVE_FOLDER**. 
For example, for dataset `fb_200M_uint64`, it includes 9 RMI model parameter files (`fb_200M_uint64_i_L1_PARAMETERS`, `i=0,...,9`), 27 RMI source code files (3 code files for each RMI model, named `fb_200M_uint64_i_data.h`, `fb_200M_uint64_i.cpp`, and `fb_200M_uint64_i.h`, for `i=0,...,9`). 

To make reproduction easier, we have generated all the required RMI parameter files and source files, available at **[link](https://www.dropbox.com/home/rmi_data)**.

### 2. Run Benchmark
To run the benchmarks:
```C++
cd exp_rmi
make -f Makefile_all run_all
```

## III. RUN PGM BENCHMARK
The original PGM-Index implementation is from: https://github.com/gvinciguerra/PGM-index

Our PGM++ implementation is based on the original PGM-Index. The codes are all put in folder `./exp_pgm`. 

To run the benchmarks:
```C++
cd exp_pgm
g++ main.cpp -std=c++17 -I. -o main -fopenmp
./main data_file_path result_output_path
```


