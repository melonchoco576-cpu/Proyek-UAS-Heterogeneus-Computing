cmake_minimum_required(VERSION 3.14)
project(SimulasiEvakuasi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimisasi release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# ==== OpenMP ====
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP ditemukan: ${OpenMP_CXX_VERSION}")
else()
    message(FATAL_ERROR "OpenMP tidak ditemukan. Install libomp-dev (Linux) atau Xcode Command Line Tools (macOS).")
endif()

# ==== OpenCL Manual Path ====

set(OpenCL_INCLUDE_DIR
    "C:/Users/viola/vcpkg/packages/opencl_x64-windows/include")
set(OpenCL_LIBRARY
    "C:/Users/viola/vcpkg/packages/opencl_x64-windows/lib/OpenCL.lib")
include_directories(${OpenCL_INCLUDE_DIR})
message(STATUS "OpenCL manual path digunakan")

# ==== Source files ====
set(SOURCES
    src/main.cpp
    src/csv_reader.cpp
    src/sequential.cpp
    src/openmp_runner.cpp
    src/opencl_runner.cpp
    src/benchmark.cpp
)

add_executable(simulasi_evakuasi ${SOURCES})

target_include_directories(simulasi_evakuasi PRIVATE src)

target_link_libraries(simulasi_evakuasi
    PRIVATE
    OpenMP::OpenMP_CXX
    ${OpenCL_LIBRARY}
)

# Salin kernel ke direktori build agar bisa diakses saat runtime
configure_file(src/evacuation_kernel.cl src/evacuation_kernel.cl COPYONLY)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
