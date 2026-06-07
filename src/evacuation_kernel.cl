/*
 * OpenCL Kernel: Simulasi Waktu Evakuasi Gedung
 * Setiap work-item memproses satu data ruangan secara independen.
 */

#define KECEPATAN_EVAKUASI 1.2f
#define FAKTOR_ANTRIAN     0.35f
#define ITERASI_KOMPUTASI  120

__kernel void hitung_evakuasi(
    __global const float* jarak_arr,        // jarak_meter tiap ruangan
    __global const int*   penghuni_arr,     // jumlah penghuni tiap ruangan
    __global float*       waktu_out,        // output: waktu evakuasi
    __global float*       kepadatan_out,    // output: indeks kepadatan
    __global float*       antrian_out,      // output: estimasi antrian
    __global float*       beban_out,        // output: beban komputasi
    const int             n_data
) {
    int gid = get_global_id(0);
    if (gid >= n_data) return;

    float jarak    = jarak_arr[gid];
    int   penghuni = penghuni_arr[gid];

    // Waktu dasar
    float waktu = jarak / KECEPATAN_EVAKUASI;

    // Indeks kepadatan
    float kepadatan = (float)penghuni / jarak;

    // Estimasi antrian
    float antrian = (float)penghuni * FAKTOR_ANTRIAN;
    waktu += antrian;

    // Simulasi beban komputasi iteratif
    float beban    = 0.0f;
    float posisi   = jarak;
    float kecepatan = KECEPATAN_EVAKUASI;

    for (int i = 0; i < ITERASI_KOMPUTASI; ++i) {
        float faktor_padat = 1.0f + (kepadatan / ((float)i + 1.0f));
        kecepatan = KECEPATAN_EVAKUASI / faktor_padat;
        posisi   -= kecepatan;
        float pos_abs = (posisi < 0.0f) ? -posisi : posisi;
        beban += sqrt(pos_abs) * log1p(faktor_padat);
        if (posisi <= 0.0f) posisi = jarak * 0.01f;
    }

    waktu_out[gid]     = waktu;
    kepadatan_out[gid] = kepadatan;
    antrian_out[gid]   = antrian;
    beban_out[gid]     = beban;
}
