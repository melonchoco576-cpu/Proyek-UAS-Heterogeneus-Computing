#pragma once
#include "data_model.h"
#include <vector>

// Menjalankan perhitungan dengan OpenCL (GPU atau CPU fallback)
BenchmarkResult jalankan_opencl(
    const std::vector<RoomData>& data,
    std::vector<EvacuationResult>& hasil
);
