#include "csv_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

// Membaca dataset CSV dan mengisinya ke dalam vector RoomData
bool baca_dataset(const std::string& path, std::vector<RoomData>& data) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ERROR] File tidak ditemukan: " << path << "\n";
        return false;
    }

    std::string baris;
    int nomor_baris = 0;
    int baris_valid = 0;
    int baris_dilewati = 0;

    // Lewati header
    if (!std::getline(file, baris)) {
        std::cerr << "[ERROR] File CSV kosong.\n";
        return false;
    }
    nomor_baris++;

    data.clear();
    data.reserve(500000);

    while (std::getline(file, baris)) {
        nomor_baris++;
        if (baris.empty()) continue;

        std::stringstream ss(baris);
        std::string token;
        RoomData room;

        try {
            // Kolom 1: ruangan
            if (!std::getline(ss, token, ',')) throw std::runtime_error("kolom ruangan");
            room.nama_ruangan = token;

            // Kolom 2: penghuni
            if (!std::getline(ss, token, ',')) throw std::runtime_error("kolom penghuni");
            room.penghuni = std::stoi(token);

            // Kolom 3: jarak_meter
            if (!std::getline(ss, token, ',')) throw std::runtime_error("kolom jarak_meter");
            room.jarak_meter = std::stod(token);

            // Validasi nilai
            if (room.penghuni < 0) {
                std::cerr << "[WARN] Baris " << nomor_baris << ": penghuni negatif, dilewati.\n";
                baris_dilewati++;
                continue;
            }
            if (room.jarak_meter <= 0.0) {
                std::cerr << "[WARN] Baris " << nomor_baris << ": jarak_meter <= 0, dilewati.\n";
                baris_dilewati++;
                continue;
            }

            data.push_back(room);
            baris_valid++;

        } catch (const std::exception& e) {
            std::cerr << "[WARN] Baris " << nomor_baris << " tidak valid (" << e.what() << "), dilewati.\n";
            baris_dilewati++;
        }
    }

    if (baris_dilewati > 0) {
        std::cout << "[INFO] " << baris_dilewati << " baris dilewati karena tidak valid.\n";
    }

    if (data.empty()) {
        std::cerr << "[ERROR] Tidak ada data valid di dataset.\n";
        return false;
    }

    return true;
}
