# Simulasi Waktu Evakuasi Gedung
# Perbandingan Kinerja Sequential, OpenMP, dan OpenCL

---

# Nama Penyusun
1. Reyfani Nazuwa Putri ( 25032014076 )
2. Tshabita Roihana Yusriah Iman ( 25032014050 )
3. Azzahra Regita Cahyani ( 25032014071 )

---

# Deskripsi Proyek

Program ini membandingkan kinerja tiga metode komputasi — Sequential, OpenMP,
dan OpenCL — dalam memproses simulasi perhitungan waktu evakuasi gedung berdasarkan
data 500.000 ruangan virtual. Setiap ruangan memiliki jumlah penghuni dan jarak ke
pintu darurat. Program mengukur waktu eksekusi, speedup, dan efficiency masing-masing
metode, lalu menampilkan ranking performa beserta kesimpulan.

# Tujuan Proyek

- Membuktikan bahwa paralelisme (OpenMP, OpenCL) dapat mempercepat komputasi
  pada dataset besar dibandingkan pendekatan Sequential.
- Menunjukkan konsep Heterogeneous Computing: pemanfaatan CPU dan GPU secara
  bersamaan untuk komputasi intensif.
- Mengukur dan menganalisis speedup, efficiency, serta overhead masing-masing metode.

# Fitur Utama

1. Membaca dan memvalidasi dataset CSV (500.000 baris).
2. Menghitung estimasi waktu evakuasi dengan rumus dasar dan komputasi tambahan
   (indeks kepadatan, estimasi antrian, simulasi beban iteratif).
3. Implementasi Sequential sebagai baseline.
4. Implementasi OpenMP dengan `#pragma omp parallel for`.
5. Implementasi OpenCL dengan kernel GPU (fallback ke CPU OpenCL jika GPU tidak tersedia).
6. Pengukuran waktu eksekusi menggunakan `chrono` (presisi milidetik).
7. Validasi konsistensi hasil ketiga metode dengan toleransi epsilon.
8. Perhitungan speedup dan efficiency.
9. Tampilan ranking performa dan kesimpulan otomatis.

# Struktur Folder

project/
├── src/
│   ├── main.cpp                  # Entry point
│   ├── csv_reader.h/.cpp         # Pembaca dan validator dataset CSV
│   ├── sequential.h/.cpp         # Implementasi Sequential
│   ├── openmp_runner.h/.cpp      # Implementasi OpenMP
│   ├── opencl_runner.h/.cpp      # Implementasi OpenCL
│   ├── benchmark.h/.cpp          # Speedup, efficiency, tampilan hasil
│   ├── data_model.h              # Struct RoomData, EvacuationResult, BenchmarkResult
│   └── evacuation_kernel.cl      # OpenCL kernel
│
├── dataset/
│   └── dataset_waktu_evakuasi_gedung_500k.csv
│
├── docs/
│   ├── laporan_singkat.md
│   ├── contoh_output.txt
│   └── analisis_benchmark.md
│
├── test/
│   ├── hasil_sequential.txt
│   ├── hasil_openmp.txt
│   └── hasil_opencl.txt
│
├── CMakeLists.txt
└── README.md

# Dependensi

| Dependensi | Versi Minimum | Keterangan                                    |
|------------|---------------|-----------------------------------------------|
| C++        | 17            | Standard modern                               |
| CMake      | 3.14          | Build system                                  |
| OpenMP     | 4.0           | `libomp-dev` (Linux) / bawaan GCC/Clang       |
| OpenCL     | 1.2           | `ocl-icd-opencl-dev` + driver GPU (Linux)     |
| GCC/Clang  | 9+            | Kompiler C++                                  |

# Instalasi dependensi (Ubuntu/Debian)

bash
sudo apt update
sudo apt install build-essential cmake libgomp1 ocl-icd-opencl-dev opencl-headers

# Untuk GPU NVIDIA:
sudo apt install nvidia-opencl-dev

# Untuk GPU AMD:
sudo apt install mesa-opencl-icd

# Untuk CPU OpenCL (fallback tanpa GPU):
sudo apt install pocl-opencl-icd

# Instalasi dependensi (macOS)

OpenMP dan OpenCL sudah tersedia melalui Xcode Command Line Tools. Tidak perlu
instalasi tambahan. Kompilasi menggunakan `clang++`.

---

# Cara Build

```bash
# Clone atau masuk ke direktori proyek
cd project

# Buat direktori build
mkdir build && cd build

# Konfigurasi
cmake ..

# Kompilasi
make -j$(nproc)

Setelah kompilasi berhasil, binary `simulasi_evakuasi` tersedia di dalam direktori `build/`.

## Cara Menjalankan Program

bash
# Dari dalam direktori build/
./simulasi_evakuasi

# Atau dengan path dataset eksplisit
./simulasi_evakuasi ../dataset/dataset_waktu_evakuasi_gedung_500k.csv

# Cara Menjalankan OpenMP

OpenMP diaktifkan secara otomatis oleh CMakeLists.txt melalui flag `-fopenmp`.
Untuk mengontrol jumlah thread:

bash
# Gunakan 8 thread
OMP_NUM_THREADS=8 ./simulasi_evakuasi

# Gunakan semua core tersedia (default)
./simulasi_evakuasi
```

Program akan menampilkan jumlah thread aktif yang digunakan OpenMP.

# Cara Menjalankan OpenCL

OpenCL diaktifkan secara otomatis. Program akan:
1. Mendeteksi GPU secara otomatis (prioritas utama).
2. Jika GPU tidak ditemukan, menggunakan CPU OpenCL sebagai fallback.
3. Menampilkan nama device yang digunakan.

Untuk memeriksa device OpenCL yang tersedia di sistem:
```bash
clinfo
```

## Contoh Output

[INFO] Membaca dataset: dataset/dataset_waktu_evakuasi_gedung_500k.csv
[INFO] 500000 data berhasil dimuat.

[INFO] Menjalankan Sequential...
[INFO] Sequential selesai.

[INFO] Menjalankan OpenMP...
[INFO] OpenMP selesai.

[INFO] Menjalankan OpenCL...
[OpenCL] Device : NVIDIA GeForce RTX 3060 (GPU)
[INFO] OpenCL selesai.

[INFO] Memvalidasi konsistensi hasil...
[VALIDASI] OpenMP  : hasil identik dengan Sequential.
[VALIDASI] OpenCL  : hasil identik dengan Sequential.

-------------------------------------------------------
  SIMULASI WAKTU EVAKUASI GEDUNG
-------------------------------------------------------
  Dataset               : dataset/dataset_waktu_evakuasi_gedung_500k.csv
  Jumlah Data           : 500000
  Kecepatan Evakuasi    : 1.2 m/s

  Sequential
-------------------------------------------------------
  Waktu Eksekusi        : 4.2318 detik

  OpenMP
-------------------------------------------------------
  Jumlah Thread         : 12
  Waktu Eksekusi        : 0.4102 detik

-------------------------------------------------------
  OpenCL
-------------------------------------------------------
  Jumlah Work-Item      : 500000
  Waktu Eksekusi        : 0.1847 detik

  HASIL SPEEDUP
-------------------------------------------------------
  OpenMP                : 10.32x
  OpenCL                : 22.91x

  HASIL EFFICIENCY
-------------------------------------------------------
  OpenMP                : 86.00 %
  OpenCL                : 0.00 %

  RANKING PERFORMA
-------------------------------------------------------
  1. OpenCL  (0.1847 detik)
  2. OpenMP  (0.4102 detik)
  3. Sequential  (4.2318 detik)


-------------------------------------------------------
  Metode tercepat adalah OpenCL.

  OpenCL memanfaatkan ribuan core GPU secara masif paralel.
  Setiap work-item memproses satu data ruangan secara independen,
  sehingga 500.000 komputasi berjalan hampir bersamaan.
  Overhead transfer data CPU-GPU terkompensasi oleh kecepatan
  eksekusi kernel pada dataset besar.
-------------------------------------------------------

# Analisis Benchmark

Lihat `docs/analisis_benchmark.md` untuk penjelasan lengkap speedup, efficiency,
overhead, dan perbandingan ketiga metode.

# Penjelasan Sequential

Sequential adalah metode baseline: satu thread CPU mengeksekusi loop dari indeks 0
sampai 499.999 secara berurutan. Tidak ada paralelisme. Digunakan sebagai referensi
untuk menghitung speedup metode lain.

# Penjelasan OpenMP

OpenMP menggunakan directive `#pragma omp parallel for` untuk membagi iterasi loop
ke beberapa thread CPU. Setiap thread mengerjakan subset data secara independen.
Tidak ada transfer data ke device lain, sehingga overhead rendah.

# Penjelasan OpenCL

OpenCL mengirim data ke device (GPU/CPU OpenCL), kemudian menjalankan kernel.
Setiap work-item memproses tepat satu data ruangan. GPU dapat menjalankan ribuan
work-item secara bersamaan. Ada overhead transfer data host-to-device dan
device-to-host, tetapi terkompensasi pada dataset besar.
