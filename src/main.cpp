#include <iostream>
#include <vector>
#include <string>

#include "data_model.h"
#include "csv_reader.h"
#include "sequential.h"
#include "openmp_runner.h"
#include "opencl_runner.h"
#include "benchmark.h"

int main(int argc, char* argv[]) {

    // Path dataset: argumen pertama atau default
    std::string path_dataset = "dataset/dataset_waktu_evakuasi_gedung_500k.csv";
    if (argc > 1) path_dataset = argv[1];

    // --- Membaca dataset CSV ---
    std::vector<RoomData> data;
    std::cout << "[INFO] Membaca dataset: " << path_dataset << "\n";
    if (!baca_dataset(path_dataset, data)) {
        std::cerr << "[FATAL] Gagal membaca dataset. Program dihentikan.\n";
        return 1;
    }
    std::cout << "[INFO] " << data.size() << " data berhasil dimuat.\n\n";

    // --- Menjalankan perhitungan Sequential ---
    std::cout << "[INFO] Menjalankan Sequential...\n";
    std::vector<EvacuationResult> hasil_seq;
    BenchmarkResult bench_seq = jalankan_sequential(data, hasil_seq);
    std::cout << "[INFO] Sequential selesai.\n\n";

    // --- Menjalankan perhitungan OpenMP ---
    std::cout << "[INFO] Menjalankan OpenMP...\n";
    std::vector<EvacuationResult> hasil_omp;
    BenchmarkResult bench_omp = jalankan_openmp(data, hasil_omp);
    std::cout << "[INFO] OpenMP selesai.\n\n";

    // --- Menjalankan perhitungan OpenCL ---
    std::cout << "[INFO] Menjalankan OpenCL...\n";
    std::vector<EvacuationResult> hasil_ocl;
    BenchmarkResult bench_ocl = jalankan_opencl(data, hasil_ocl);
    std::cout << "[INFO] OpenCL selesai.\n\n";

    // --- Validasi hasil ---
    std::cout << "[INFO] Memvalidasi konsistensi hasil...\n";
    bool valid_omp = validasi_hasil(hasil_seq, hasil_omp, "OpenMP");
    bool valid_ocl = (bench_ocl.waktu_eksekusi_ms > 0)
                     ? validasi_hasil(hasil_seq, hasil_ocl, "OpenCL", 1.0)
                     : false;

    if (valid_omp)
        std::cout << "[VALIDASI] OpenMP  : hasil identik dengan Sequential.\n";
    if (bench_ocl.waktu_eksekusi_ms > 0) {
        if (valid_ocl)
            std::cout << "[VALIDASI] OpenCL  : hasil identik dengan Sequential.\n";
        else
            std::cout << "[VALIDASI] OpenCL  : terdapat perbedaan (kemungkinan presisi float).\n";
    }
    std::cout << "\n";

    // --- Menghitung speedup dan efficiency ---
    RingkasanBenchmark ringkasan = hitung_benchmark(bench_seq, bench_omp, bench_ocl);

    // --- Menampilkan hasil benchmark ---
    tampilkan_hasil(ringkasan, path_dataset, static_cast<int>(data.size()));

    return 0;
}
