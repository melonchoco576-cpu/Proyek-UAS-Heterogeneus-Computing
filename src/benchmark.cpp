#include "benchmark.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>

// Menghitung speedup dan efficiency dari tiga metode
RingkasanBenchmark hitung_benchmark(
    const BenchmarkResult& seq,
    const BenchmarkResult& omp,
    const BenchmarkResult& ocl
) {
    RingkasanBenchmark r;
    r.sequential = seq;
    r.openmp     = omp;
    r.opencl     = ocl;

    // Speedup = waktu_sequential / waktu_paralel
    r.speedup_openmp = (omp.waktu_eksekusi_ms > 0.0)
                       ? seq.waktu_eksekusi_ms / omp.waktu_eksekusi_ms
                       : 0.0;
    r.speedup_opencl = (ocl.waktu_eksekusi_ms > 0.0)
                       ? seq.waktu_eksekusi_ms / ocl.waktu_eksekusi_ms
                       : 0.0;

    // Efficiency = speedup / jumlah_unit_paralel * 100%
    r.efficiency_openmp = (omp.paralel_unit > 0)
                          ? (r.speedup_openmp / omp.paralel_unit) * 100.0
                          : 0.0;
    r.efficiency_opencl = (ocl.paralel_unit > 0)
                          ? (r.speedup_opencl / ocl.paralel_unit) * 100.0
                          : 0.0;

    return r;
}

// Memvalidasi bahwa dua set hasil identik dalam toleransi epsilon
bool validasi_hasil(
    const std::vector<EvacuationResult>& ref,
    const std::vector<EvacuationResult>& kandidat,
    const std::string& nama_kandidat,
    double epsilon
) {
    if (ref.size() != kandidat.size()) {
        std::cerr << "[VALIDASI] " << nama_kandidat
                  << ": ukuran hasil berbeda (" << ref.size()
                  << " vs " << kandidat.size() << ")\n";
        return false;
    }

    int mismatch = 0;
    for (size_t i = 0; i < ref.size(); ++i) {
        if (std::fabs(ref[i].waktu_evakuasi - kandidat[i].waktu_evakuasi) > epsilon) {
            mismatch++;
            if (mismatch <= 5) {
                std::cerr << "[VALIDASI] " << nama_kandidat << " baris " << i
                          << ": seq=" << ref[i].waktu_evakuasi
                          << " vs " << kandidat[i].waktu_evakuasi << "\n";
            }
        }
    }

    if (mismatch > 0) {
        std::cerr << "[PERINGATAN] " << nama_kandidat
                  << ": " << mismatch << " dari " << ref.size()
                  << " data berbeda melebihi toleransi epsilon=" << epsilon << "\n";
        return false;
    }
    return true;
}

// Menampilkan output benchmark yang bersih dan profesional
void tampilkan_hasil(
    const RingkasanBenchmark& r,
    const std::string& path_dataset,
    int jumlah_data
) {
    const std::string GARIS = std::string(55, '-');

    std::cout << "\n" << GARIS << "\n";
    std::cout << "  SIMULASI WAKTU EVAKUASI GEDUNG\n";
    std::cout << GARIS << "\n";
    std::cout << std::left << std::setw(22) << "  Dataset"
              << ": " << path_dataset << "\n";
    std::cout << std::left << std::setw(22) << "  Jumlah Data"
              << ": " << jumlah_data << "\n";
    std::cout << std::left << std::setw(22) << "  Kecepatan Evakuasi"
              << ": 1.2 m/s\n";

    // --- Sequential ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  Sequential\n";
    std::cout << GARIS << "\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << std::left << std::setw(22) << "  Waktu Eksekusi"
              << ": " << r.sequential.waktu_eksekusi_ms / 1000.0 << " detik\n";

    // --- OpenMP ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  OpenMP\n";
    std::cout << GARIS << "\n";
    std::cout << std::left << std::setw(22) << "  Jumlah Thread"
              << ": " << r.openmp.paralel_unit << "\n";
    std::cout << std::left << std::setw(22) << "  Waktu Eksekusi"
              << ": " << r.openmp.waktu_eksekusi_ms / 1000.0 << " detik\n";

    // --- OpenCL ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  OpenCL\n";
    std::cout << GARIS << "\n";
    if (r.opencl.waktu_eksekusi_ms < 0) {
        std::cout << "  Status          : GAGAL (device tidak tersedia)\n";
    } else {
        std::cout << std::left << std::setw(22) << "  Jumlah Work-Item"
                  << ": " << r.opencl.paralel_unit << "\n";
        std::cout << std::left << std::setw(22) << "  Waktu Eksekusi"
                  << ": " << r.opencl.waktu_eksekusi_ms / 1000.0 << " detik\n";
    }

    // --- Speedup ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  HASIL SPEEDUP\n";
    std::cout << GARIS << "\n";
    std::cout << std::setprecision(2);
    std::cout << std::left << std::setw(22) << "  OpenMP"  << ": " << r.speedup_openmp << "x\n";
    if (r.opencl.waktu_eksekusi_ms > 0)
        std::cout << std::left << std::setw(22) << "  OpenCL" << ": " << r.speedup_opencl << "x\n";
    else
        std::cout << std::left << std::setw(22) << "  OpenCL" << ": N/A\n";

    // --- Efficiency ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  HASIL EFFICIENCY\n";
    std::cout << GARIS << "\n";
    std::cout << std::setprecision(2);
    std::cout << std::left << std::setw(22) << "  OpenMP"
              << ": " << r.efficiency_openmp << " %\n";
    if (r.opencl.waktu_eksekusi_ms > 0)
        std::cout << std::left << std::setw(22) << "  OpenCL"
                  << ": " << r.efficiency_opencl << " %\n";
    else
        std::cout << std::left << std::setw(22) << "  OpenCL" << ": N/A\n";

    // --- Ranking ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  RANKING PERFORMA\n";
    std::cout << GARIS << "\n";

    struct Peserta { std::string nama; double waktu_ms; bool valid; };
    std::vector<Peserta> peserta = {
        { "Sequential", r.sequential.waktu_eksekusi_ms, true },
        { "OpenMP",     r.openmp.waktu_eksekusi_ms,     r.openmp.waktu_eksekusi_ms > 0 },
        { "OpenCL",     r.opencl.waktu_eksekusi_ms,     r.opencl.waktu_eksekusi_ms > 0 }
    };

    // Urutkan dari tercepat ke terlambat, invalid di akhir
    std::sort(peserta.begin(), peserta.end(), [](const Peserta& a, const Peserta& b) {
        if (!a.valid) return false;
        if (!b.valid) return true;
        return a.waktu_ms < b.waktu_ms;
    });

    for (int i = 0; i < 3; ++i) {
        std::cout << "  " << (i + 1) << ". " << peserta[i].nama;
        if (peserta[i].valid)
            std::cout << "  (" << std::fixed << std::setprecision(4)
                      << peserta[i].waktu_ms / 1000.0 << " detik)";
        else
            std::cout << "  (tidak tersedia)";
        std::cout << "\n";
    }

    // --- Kesimpulan ---
    std::cout << "\n" << GARIS << "\n";
    std::cout << "  KESIMPULAN\n";
    std::cout << GARIS << "\n";

    std::string tercepat = peserta[0].valid ? peserta[0].nama : "tidak ada";
    std::cout << "  Metode tercepat adalah " << tercepat << ".\n\n";

    if (tercepat == "OpenCL") {
        std::cout << "  OpenCL memanfaatkan ribuan core GPU secara masif paralel.\n"
                  << "  Setiap work-item memproses satu data ruangan secara independen,\n"
                  << "  sehingga 500.000 komputasi berjalan hampir bersamaan.\n"
                  << "  Overhead transfer data CPU-GPU terkompensasi oleh kecepatan\n"
                  << "  eksekusi kernel pada dataset besar.\n";
    } else if (tercepat == "OpenMP") {
        std::cout << "  OpenMP membagi 500.000 iterasi ke seluruh thread CPU tersedia.\n"
                  << "  Tidak ada overhead transfer memori CPU-GPU, sehingga untuk\n"
                  << "  dataset berukuran ini OpenMP lebih efisien dari OpenCL.\n"
                  << "  Speedup mendekati jumlah thread yang tersedia.\n";
    } else {
        std::cout << "  Sequential menjadi yang tercepat dalam kondisi ini.\n"
                  << "  Ini bisa terjadi jika OpenMP/OpenCL mengalami overhead\n"
                  << "  yang lebih besar dari manfaat paralelisme pada mesin ini.\n";
    }

    std::cout << GARIS << "\n\n";
}
