# Analisis Benchmark: Simulasi Waktu Evakuasi Gedung

---

## 1. Definisi Metrik

### Speedup
Speedup mengukur seberapa besar peningkatan kecepatan metode paralel dibandingkan sequential.

```
Speedup = Waktu_Sequential / Waktu_Paralel
```

Speedup ideal (linear) = jumlah unit paralel. Dalam praktik selalu lebih rendah karena overhead.

### Efficiency
Efficiency mengukur seberapa efisien setiap unit paralel dimanfaatkan.

```
Efficiency = (Speedup / Jumlah_Unit_Paralel) * 100%
```

Efficiency 100% berarti setiap thread/work-item bekerja penuh tanpa idle.
Efficiency rendah berarti ada overhead, load imbalance, atau bottleneck sinkronisasi.

---

## 2. Komponen Waktu Eksekusi

### Sequential
- Hanya waktu komputasi murni (loop 0..N-1)
- Tidak ada overhead paralelisme

### OpenMP
- Waktu komputasi (dibagi ke thread)
- Overhead thread creation dan join
- Overhead sinkronisasi (implicit barrier di akhir parallel for)
- Fork-join model: overhead fork di awal, join di akhir

### OpenCL
- Waktu transfer data Host -> Device (CPU ke GPU)
- Waktu kompilasi kernel (hanya sekali per eksekusi)
- Waktu eksekusi kernel di device
- Waktu transfer data Device -> Host (GPU ke CPU)
- Overhead enqueue dan finish

---

## 3. Faktor yang Mempengaruhi Performa

### Thread Count (OpenMP)
Semakin banyak thread, semakin besar potensi speedup. Namun di atas jumlah core fisik,
thread tambahan tidak memberikan manfaat dan malah menambah overhead context switch.

### Memory Hierarchy (OpenCL)
GPU memiliki hirarki memori:
- Global Memory: besar, lambat (mirip RAM)
- Local Memory: kecil, sangat cepat (shared per workgroup)
- Private Memory: register tiap work-item

Kernel ini menggunakan global memory karena setiap work-item independen.
Optimasi lebih lanjut dapat memanfaatkan local memory untuk operasi reduce.

### CPU to GPU Transfer (Overhead OpenCL)
Transfer 500.000 * (4 byte float + 4 byte int) = sekitar 4 MB data input.
Transfer hasil output: 500.000 * 4 * 4 byte = sekitar 8 MB.
Pada dataset 500.000 entri, overhead transfer ini biasanya masih jauh lebih kecil
dari manfaat paralelisme masif GPU.

### Load Imbalance
Setiap work-item / thread memproses komputasi dengan jumlah iterasi yang sama (120 iterasi).
Beban per work-item seragam, sehingga load imbalance minimal.

---

## 4. Hukum Amdahl

Hukum Amdahl menyatakan bahwa speedup maksimum dibatasi oleh bagian kode yang tidak
dapat diparalelkan:

```
Speedup_max = 1 / (S + (1-S)/N)
```

Di mana S = fraksi serial (I/O, inisialisasi) dan N = jumlah prosesor.

Dalam program ini, bagian serial meliputi:
- Pembacaan CSV
- Inisialisasi buffer
- Validasi hasil

Bagian paralel (komputasi evakuasi) mencapai lebih dari 95% total waktu,
sehingga potensi speedup tinggi.

---

## 5. Bottleneck

### OpenMP
- Cache coherence: banyak thread mengakses array hasil (write) secara bersamaan.
  Karena setiap thread menulis ke indeks berbeda, false sharing minimal.
- Overhead barrier di akhir `parallel for` menunggu thread paling lambat selesai.

### OpenCL
- Transfer PCIe (CPU-GPU) adalah bottleneck utama untuk dataset kecil.
- Untuk dataset 500.000 entri, throughput transfer (~10 GB/s pada PCIe 4.0)
  membuat overhead transfer kecil dibandingkan manfaat paralelisme GPU.
- Kompilasi JIT kernel menambah latency satu kali di awal.

---

## 6. Scalability

### OpenMP
Scalability dibatasi oleh jumlah core CPU fisik. Pada sistem 12-core, speedup
diharapkan berkisar 8-11x (efisiensi 67-92%) karena overhead fork-join.

### OpenCL
Scalability tinggi pada GPU karena ratusan hingga ribuan compute unit tersedia.
Setiap SM/CU mengeksekusi banyak work-item secara pipelined (latency hiding).

---

## 7. Synchronization

### OpenMP
Sinkronisasi implisit terjadi di akhir `parallel for` (implicit barrier).
Tidak ada sinkronisasi antar thread selama loop berlangsung karena setiap
iterasi independen.

### OpenCL
Sinkronisasi antara host dan device dilakukan melalui `clFinish()`, yang
memblokir CPU hingga semua work-item selesai. Tidak ada sinkronisasi antar
work-item karena komputasi sepenuhnya independen.

---

## 8. Kesimpulan Analisis

| Aspek            | Sequential | OpenMP        | OpenCL               |
|------------------|------------|---------------|----------------------|
| Paralel Unit     | 1          | N thread CPU  | 500.000 work-item    |
| Overhead         | Tidak ada  | Fork-join     | Transfer data + JIT  |
| Scalability      | Tidak ada  | Terbatas core | Sangat tinggi (GPU)  |
| Efisiensi memory | Sangat baik| Baik          | Perlu perhatian PCIe |
| Kemudahan kode   | Tinggi     | Tinggi        | Sedang               |

Untuk dataset besar (>100.000 entri) dengan komputasi per-elemen yang sepenuhnya
independen, OpenCL pada GPU umumnya memberikan speedup tertinggi. OpenMP cocok
jika GPU tidak tersedia atau dataset cukup kecil sehingga overhead transfer OpenCL
mendominasi.
