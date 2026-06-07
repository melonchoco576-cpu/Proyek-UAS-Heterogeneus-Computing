#pragma once
#include "data_model.h"
#include <vector>

// Menjalankan perhitungan dengan OpenMP (multi-thread CPU)
BenchmarkResult jalankan_openmp(
    const std::vector<RoomData>& data,
    std::vector<EvacuationResult>& hasil
);
