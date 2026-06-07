#pragma once
#include <string>
#include <vector>

// Struktur data satu entri ruangan dari dataset
struct RoomData {
    std::string nama_ruangan;
    int         penghuni;
    double      jarak_meter;
};

// Struktur hasil perhitungan waktu evakuasi
struct EvacuationResult {
    double waktu_evakuasi;        // detik, hasil akhir
    double indeks_kepadatan;      // penghuni / jarak
    double estimasi_antrian;      // penghuni * faktor antrian
    double beban_evakuasi;        // komputasi numerik tambahan
};

// Struktur hasil benchmark satu metode
struct BenchmarkResult {
    std::string nama_metode;
    double      waktu_eksekusi_ms;  // milidetik
    int         paralel_unit;       // jumlah thread / work-item
};
