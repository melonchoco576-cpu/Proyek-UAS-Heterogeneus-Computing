#include "sequential.h"
#include <chrono>
#include <cmath>

// Konstanta simulasi evakuasi
static const double KECEPATAN_EVAKUASI = 1.2;      // m/s
static const double FAKTOR_ANTRIAN     = 0.35;      // faktor penambah waktu antrian
static const int    ITERASI_KOMPUTASI  = 120;       // iterasi untuk beban komputasi tambahan

// Menghitung satu entri ruangan secara numerik
// Menggunakan perhitungan tambahan agar manfaat paralel terlihat jelas
static inline EvacuationResult hitung_evakuasi(const RoomData& room) {
    EvacuationResult res;

    // Waktu dasar: jarak / kecepatan
    res.waktu_evakuasi = room.jarak_meter / KECEPATAN_EVAKUASI;

    // Indeks kepadatan: penghuni relatif terhadap jarak
    res.indeks_kepadatan = static_cast<double>(room.penghuni) / room.jarak_meter;

    // Estimasi antrian: penghuni dikali faktor antrian, ditambah ke waktu evakuasi
    res.estimasi_antrian = room.penghuni * FAKTOR_ANTRIAN;
    res.waktu_evakuasi  += res.estimasi_antrian;

    // Simulasi beban komputasi: iterasi numerik yang relevan
    // Mensimulasikan pergerakan evakuasi bertahap dengan atenuasi
    double beban = 0.0;
    double posisi = room.jarak_meter;
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

// Menjalankan perhitungan Sequential
BenchmarkResult jalankan_sequential(
    const std::vector<RoomData>& data,
    std::vector<EvacuationResult>& hasil
) {
    hasil.resize(data.size());

    auto t_mulai = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < data.size(); ++i) {
        hasil[i] = hitung_evakuasi(data[i]);
    }

    auto t_selesai = std::chrono::high_resolution_clock::now();
    double durasi_ms = std::chrono::duration<double, std::milli>(t_selesai - t_mulai).count();

    BenchmarkResult bench;
    bench.nama_metode       = "Sequential";
    bench.waktu_eksekusi_ms = durasi_ms;
    bench.paralel_unit      = 1;
    return bench;
}
