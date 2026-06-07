#include "openmp_runner.h"
#include <chrono>
#include <cmath>

#ifdef _OPENMP
#include <omp.h>
#endif

static const double KECEPATAN_EVAKUASI = 1.2;
static const double FAKTOR_ANTRIAN     = 0.35;
static const int    ITERASI_KOMPUTASI  = 120;

// Hitung evakuasi satu ruangan (identik dengan sequential untuk validasi)
static inline EvacuationResult hitung_evakuasi_omp(const RoomData& room) {
    EvacuationResult res;

    res.waktu_evakuasi   = room.jarak_meter / KECEPATAN_EVAKUASI;
    res.indeks_kepadatan = static_cast<double>(room.penghuni) / room.jarak_meter;
    res.estimasi_antrian = room.penghuni * FAKTOR_ANTRIAN;
    res.waktu_evakuasi  += res.estimasi_antrian;

    double beban    = 0.0;
    double posisi   = room.jarak_meter;
    double kecepatan = KECEPATAN_EVAKUASI;
    for (int i = 0; i < ITERASI_KOMPUTASI; ++i) {
        double faktor_padat = 1.0 + (res.indeks_kepadatan / (i + 1.0));
        kecepatan = KECEPATAN_EVAKUASI / faktor_padat;
        posisi   -= kecepatan;
        beban    += std::sqrt(std::fabs(posisi)) * std::log1p(faktor_padat);
        if (posisi <= 0.0) posisi = room.jarak_meter * 0.01;
    }
    res.beban_evakuasi = beban;

    return res;
}

// Menjalankan perhitungan OpenMP dengan pragma parallel for
BenchmarkResult jalankan_openmp(
    const std::vector<RoomData>& data,
    std::vector<EvacuationResult>& hasil
) {
    hasil.resize(data.size());
    int jumlah_thread = 1;

#ifdef _OPENMP
    jumlah_thread = omp_get_max_threads();
#endif

    auto t_mulai = std::chrono::high_resolution_clock::now();

    // Distribusi iterasi ke semua thread secara otomatis
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        hasil[i] = hitung_evakuasi_omp(data[i]);
    }

    auto t_selesai = std::chrono::high_resolution_clock::now();
    double durasi_ms = std::chrono::duration<double, std::milli>(t_selesai - t_mulai).count();

    BenchmarkResult bench;
    bench.nama_metode       = "OpenMP";
    bench.waktu_eksekusi_ms = durasi_ms;
    bench.paralel_unit      = jumlah_thread;
    return bench;
}
