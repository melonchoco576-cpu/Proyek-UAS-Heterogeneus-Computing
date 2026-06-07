#pragma once
#include "data_model.h"
#include <vector>
#include <string>

// Struktur ringkasan perbandingan semua metode
struct RingkasanBenchmark {
    BenchmarkResult sequential;
    BenchmarkResult openmp;
    BenchmarkResult opencl;
    double speedup_openmp;
    double speedup_opencl;
    double efficiency_openmp;   // persen
    double efficiency_opencl;   // persen
};

// Menghitung speedup dan efficiency
RingkasanBenchmark hitung_benchmark(
    const BenchmarkResult& seq,
    const BenchmarkResult& omp,
    const BenchmarkResult& ocl
);

// Memvalidasi bahwa hasil dua metode identik dalam toleransi epsilon
bool validasi_hasil(
    const std::vector<EvacuationResult>& ref,
    const std::vector<EvacuationResult>& kandidat,
    const std::string& nama_kandidat,
    double epsilon = 1e-2
);

// Menampilkan output benchmark ke terminal
void tampilkan_hasil(
    const RingkasanBenchmark& ringkasan,
    const std::string& path_dataset,
    int jumlah_data
);
