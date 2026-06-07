#pragma once
#include "data_model.h"
#include <vector>

// Menjalankan perhitungan Sequential (baseline single-thread)
BenchmarkResult jalankan_sequential(
    const std::vector<RoomData>& data,
    std::vector<EvacuationResult>& hasil
);
