#ifndef __BENCHMARK__
#define __BENCHMARK__

#include "getarg.h"

struct benchmark {
  #ifdef DEVICE_CPU_ENABLED
  struct benchmark_cpu* cpu_bench;
  #endif

  #ifdef DEVICE_GPU_ENABLED
  struct benchmark_gpu* gpu_bench;
  #endif

  device_type device;
  double gflops;
};

struct hardware {
  #ifdef DEVICE_CPU_ENABLED
  struct cpu* cpu;
  #endif

  #ifdef DEVICE_GPU_ENABLED
  struct gpu* gpu;
  #endif
};

struct config {
  #ifdef DEVICE_CPU_ENABLED
  int n_threads;
  #endif
};

struct benchmark* init_benchmark_device(device_type device);
bench_type get_benchmark_type(struct benchmark* bench);
struct hardware* get_hardware_info();
struct config* get_benchmark_config(struct benchmark* bench, int n_threads);
bool init_benchmark(struct benchmark* bench, struct hardware* hw, struct config* cfg, bench_type type);
void print_hw_info(struct hardware* hw);
void print_bench_cfg(struct config* cfg);
void compute(struct benchmark* bench);
double get_gflops(struct benchmark* bench);
const char* get_benchmark_name(struct benchmark* bench);

#endif
