#include "opencl_runner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <cmath>

#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

// Membaca source code kernel dari file .cl
static std::string baca_kernel_source(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Kernel OpenCL tidak ditemukan: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Memeriksa error OpenCL dan melempar exception jika gagal
static void periksa_cl(cl_int err, const std::string& konteks) {
    if (err != CL_SUCCESS) {
        throw std::runtime_error("[OpenCL ERROR] " + konteks + " (kode: " + std::to_string(err) + ")");
    }
}

// Menjalankan perhitungan OpenCL
BenchmarkResult jalankan_opencl(
    const std::vector<RoomData>& data,
    std::vector<EvacuationResult>& hasil
) {
    const int n = static_cast<int>(data.size());
    hasil.resize(n);

    BenchmarkResult bench;
    bench.nama_metode  = "OpenCL";
    bench.paralel_unit = n;

    // --- Siapkan array flat untuk transfer ke device ---
    std::vector<float> jarak_arr(n), waktu_out(n), kepadatan_out(n), antrian_out(n), beban_out(n);
    std::vector<int>   penghuni_arr(n);

    for (int i = 0; i < n; ++i) {
        jarak_arr[i]    = static_cast<float>(data[i].jarak_meter);
        penghuni_arr[i] = data[i].penghuni;
    }

    try {
        cl_int err;

        // --- Enumerasi platform ---
        cl_uint n_platform = 0;
        clGetPlatformIDs(0, nullptr, &n_platform);
        if (n_platform == 0) throw std::runtime_error("Tidak ada platform OpenCL tersedia.");

        std::vector<cl_platform_id> platforms(n_platform);
        clGetPlatformIDs(n_platform, platforms.data(), nullptr);

        // --- Pilih device: GPU diutamakan, fallback CPU ---
        cl_device_id device = nullptr;
        cl_device_type tipe_device = CL_DEVICE_TYPE_GPU;
        bool pakai_gpu = false;

        for (auto& plat : platforms) {
            cl_uint n_dev = 0;
            if (clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 0, nullptr, &n_dev) == CL_SUCCESS && n_dev > 0) {
                std::vector<cl_device_id> devs(n_dev);
                clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, n_dev, devs.data(), nullptr);
                device    = devs[0];
                pakai_gpu = true;
                break;
            }
        }

        if (!pakai_gpu) {
            std::cout << "[OpenCL] GPU tidak ditemukan. Menggunakan CPU OpenCL sebagai fallback.\n";
            for (auto& plat : platforms) {
                cl_uint n_dev = 0;
                if (clGetDeviceIDs(plat, CL_DEVICE_TYPE_CPU, 0, nullptr, &n_dev) == CL_SUCCESS && n_dev > 0) {
                    std::vector<cl_device_id> devs(n_dev);
                    clGetDeviceIDs(plat, CL_DEVICE_TYPE_CPU, n_dev, devs.data(), nullptr);
                    device     = devs[0];
                    tipe_device = CL_DEVICE_TYPE_CPU;
                    break;
                }
            }
        }

        if (!device) throw std::runtime_error("Tidak ada device OpenCL (GPU maupun CPU) tersedia.");

        // Tampilkan info device
        char nama_device[256] = {0};
        clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(nama_device), nama_device, nullptr);
        std::cout << "[OpenCL] Device : " << nama_device
                  << (pakai_gpu ? " (GPU)" : " (CPU fallback)") << "\n";

        // --- Buat context dan command queue ---
        cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        periksa_cl(err, "clCreateContext");

        cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
        periksa_cl(err, "clCreateCommandQueue");

        // --- Kompilasi kernel ---
        std::string kernel_src = baca_kernel_source("src/evacuation_kernel.cl");
        const char* src_ptr    = kernel_src.c_str();
        size_t      src_len    = kernel_src.size();

        cl_program program = clCreateProgramWithSource(context, 1, &src_ptr, &src_len, &err);
        periksa_cl(err, "clCreateProgramWithSource");

        err = clBuildProgram(program, 1, &device, "-cl-fast-relaxed-math", nullptr, nullptr);
        if (err != CL_SUCCESS) {
            size_t log_len = 0;
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_len);
            std::string log(log_len, '\0');
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_len, &log[0], nullptr);
            throw std::runtime_error("Build kernel gagal:\n" + log);
        }

        cl_kernel kernel = clCreateKernel(program, "hitung_evakuasi", &err);
        periksa_cl(err, "clCreateKernel");

        // --- Alokasi buffer device ---
        size_t sz_float = n * sizeof(float);
        size_t sz_int   = n * sizeof(int);

        cl_mem buf_jarak    = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sz_float, jarak_arr.data(),    &err); periksa_cl(err, "buf_jarak");
        cl_mem buf_penghuni = clCreateBuffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sz_int,   penghuni_arr.data(), &err); periksa_cl(err, "buf_penghuni");
        cl_mem buf_waktu    = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                         sz_float, nullptr,            &err); periksa_cl(err, "buf_waktu");
        cl_mem buf_kepadat  = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                         sz_float, nullptr,            &err); periksa_cl(err, "buf_kepadat");
        cl_mem buf_antrian  = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                         sz_float, nullptr,            &err); periksa_cl(err, "buf_antrian");
        cl_mem buf_beban    = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                         sz_float, nullptr,            &err); periksa_cl(err, "buf_beban");

        // --- Set argumen kernel ---
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_jarak);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_penghuni);
        clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_waktu);
        clSetKernelArg(kernel, 3, sizeof(cl_mem), &buf_kepadat);
        clSetKernelArg(kernel, 4, sizeof(cl_mem), &buf_antrian);
        clSetKernelArg(kernel, 5, sizeof(cl_mem), &buf_beban);
        clSetKernelArg(kernel, 6, sizeof(int),    &n);

        // --- Eksekusi kernel ---
        size_t global_size = static_cast<size_t>(n);
        size_t local_size  = 256;
        // Bulatkan global_size ke kelipatan local_size
        if (global_size % local_size != 0)
            global_size = ((global_size / local_size) + 1) * local_size;

        auto t_mulai = std::chrono::high_resolution_clock::now();

        cl_event event_kernel;
        err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, &event_kernel);
        periksa_cl(err, "clEnqueueNDRangeKernel");

        clFinish(queue);

        auto t_selesai = std::chrono::high_resolution_clock::now();
        bench.waktu_eksekusi_ms = std::chrono::duration<double, std::milli>(t_selesai - t_mulai).count();

        clReleaseEvent(event_kernel);

        // --- Baca hasil dari device ---
        clEnqueueReadBuffer(queue, buf_waktu,   CL_TRUE, 0, sz_float, waktu_out.data(),     0, nullptr, nullptr);
        clEnqueueReadBuffer(queue, buf_kepadat, CL_TRUE, 0, sz_float, kepadatan_out.data(), 0, nullptr, nullptr);
        clEnqueueReadBuffer(queue, buf_antrian, CL_TRUE, 0, sz_float, antrian_out.data(),   0, nullptr, nullptr);
        clEnqueueReadBuffer(queue, buf_beban,   CL_TRUE, 0, sz_float, beban_out.data(),     0, nullptr, nullptr);

        // --- Salin ke hasil ---
        for (int i = 0; i < n; ++i) {
            hasil[i].waktu_evakuasi   = static_cast<double>(waktu_out[i]);
            hasil[i].indeks_kepadatan = static_cast<double>(kepadatan_out[i]);
            hasil[i].estimasi_antrian = static_cast<double>(antrian_out[i]);
            hasil[i].beban_evakuasi   = static_cast<double>(beban_out[i]);
        }

        // --- Bersihkan resource ---
        clReleaseMemObject(buf_jarak);
        clReleaseMemObject(buf_penghuni);
        clReleaseMemObject(buf_waktu);
        clReleaseMemObject(buf_kepadat);
        clReleaseMemObject(buf_antrian);
        clReleaseMemObject(buf_beban);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        bench.waktu_eksekusi_ms = -1.0;
        bench.paralel_unit      = 0;
    }

    return bench;
}
