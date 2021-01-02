#include <stdio.h>
#include <omp.h>
#include <string.h>

#define TYPE __m256

#include "Arch.h"

#include "sandy_bridge.h"  
#include "ivy_bridge.h"
#include "haswell.h" 
#include "skylake_256.h"
#include "skylake_512.h"
#include "broadwell.h"
#include "kaby_lake.h"
#include "coffe_lake.h"
#include "cannon_lake_256.h"  
#include "cannon_lake_512.h"
#include "ice_lake_256.h"
#include "ice_lake_512.h"
#include "knl.h"
#include "zen.h"
#include "zen_plus.h"

struct benchmark {
  int n_threads;
  double gflops;
  char* name;
  bench_type benchmark_type;
  void(*compute_function)(TYPE *farr_ptr, TYPE, int);
};

enum {
  BENCH_256_3_NOFMA,  
  BENCH_256_5,
  BENCH_256_8,
  BENCH_256_10,
  BENCH_512_8,
  BENCH_512_12,
};

static char *bench_name[] = {
  [BENCH_TYPE_SANDY_BRIDGE]    = "Sandy Bridge (AVX)",
  [BENCH_TYPE_IVY_BRIDGE]      = "Ivy Bridge (AVX)",
  [BENCH_TYPE_HASWELL]         = "Haswell (AVX2)",
  [BENCH_TYPE_BROADWELL]       = "Broadwell (AVX2)",
  [BENCH_TYPE_SKYLAKE_256]     = "Skylake (AVX2)",
  [BENCH_TYPE_SKYLAKE_512]     = "Skylake (AVX512)",
  [BENCH_TYPE_KABY_LAKE]       = "Kaby Lake (AVX2)",
  [BENCH_TYPE_COFFE_LAKE]      = "Coffe Lake (AVX2)",
  [BENCH_TYPE_ICE_LAKE]        = "Ice Lake (AVX2)",
  [BENCH_TYPE_KNIGHTS_LANDING] = "Knights Landing (AVX512)",
  [BENCH_TYPE_ZEN]             = "Zen (AVX2)",
  [BENCH_TYPE_ZEN_PLUS]        = "Zen+ (AVX2)",
};

static char *bench_types_str[] = {
  [BENCH_TYPE_SANDY_BRIDGE]    = "sandy_bridge",
  [BENCH_TYPE_IVY_BRIDGE]      = "ivy_bridge",
  [BENCH_TYPE_HASWELL]         = "haswell",
  [BENCH_TYPE_BROADWELL]       = "broadwell",
  [BENCH_TYPE_SKYLAKE_256]     = "skylake_256",
  [BENCH_TYPE_SKYLAKE_512]     = "skylake_512",
  [BENCH_TYPE_KABY_LAKE]       = "kaby_lake",
  [BENCH_TYPE_COFFE_LAKE]      = "coffe_lake",
  [BENCH_TYPE_ICE_LAKE]        = "ice_lake",
  [BENCH_TYPE_KNIGHTS_LANDING] = "knights_landing",
  [BENCH_TYPE_ZEN]             = "zen",
  [BENCH_TYPE_ZEN_PLUS]        = "zen_plus",
};

bench_type parse_benchmark(char* str) {
  int len = sizeof(bench_types_str) / sizeof(bench_types_str[0]);
  for(bench_type t = 0; t < len; t++) {
    if(strcmp(str, bench_types_str[t]) == 0) {
      return t;    
    }
  }
  return BENCH_TYPE_INVALID;    
}

void print_bench_types() {
  int len = sizeof(bench_types_str) / sizeof(bench_types_str[0]);
  long unsigned int longest = 0;
  for(bench_type t = 0; t < len; t++) {
    if(strlen(bench_name[t]) > longest) longest = strlen(bench_name[t]);
  }
  
  printf("Available benchmark types:\n");  
  for(bench_type t = 0; t < len; t++) {
    printf(" - %s %*s(Keyword: %s)\n", bench_name[t], (int) (strlen(bench_name[t]) - longest), "", bench_types_str[t]);
  }
}

double compute_gflops(int n_threads, char bench) {
  int fma_available;
  int op_per_it;
  int bytes_in_vect;
  
  switch(bench) {
    case BENCH_256_3_NOFMA:
      fma_available = B_256_3_NOFMA_FMA_AV;
      op_per_it = B_256_3_NOFMA_OP_IT;
      bytes_in_vect = B_256_3_NOFMA_BYTES;
      break;
    case BENCH_256_5:
      fma_available = B_256_5_FMA_AV;
      op_per_it = B_256_5_OP_IT;
      bytes_in_vect = B_256_5_BYTES;
      break;  
    case BENCH_256_8:
      fma_available = B_256_8_FMA_AV;
      op_per_it = B_256_8_OP_IT;
      bytes_in_vect = B_256_8_BYTES;
      break;
    case BENCH_256_10:
      fma_available = B_256_10_FMA_AV;
      op_per_it = B_256_10_OP_IT;
      bytes_in_vect = B_256_10_BYTES;
      break;
    case BENCH_512_8:
      fma_available = B_512_8_FMA_AV;
      op_per_it = B_512_8_OP_IT;
      bytes_in_vect = B_512_8_BYTES;
      break;
    case BENCH_512_12:
      fma_available = B_512_12_FMA_AV;
      op_per_it = B_512_12_OP_IT;
      bytes_in_vect = B_512_12_BYTES;
      break;  
    default:
      printf("ERROR: Invalid benchmark type!\n");
      return -1.0;
  }
  
  return (double)((long)n_threads*MAXFLOPS_ITERS*op_per_it*(bytes_in_vect/4)*fma_available)/1000000000;        
}

bool select_benchmark(struct benchmark* bench) {
  switch(bench->benchmark_type) {
    case BENCH_TYPE_SANDY_BRIDGE:
      bench->compute_function = compute_sandy_bridge;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_3_NOFMA);
      break;  
    case BENCH_TYPE_IVY_BRIDGE:
      bench->compute_function = compute_ivy_bridge;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_3_NOFMA);
      break;
    case BENCH_TYPE_HASWELL:
      bench->compute_function = compute_haswell;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_10);
      break;  
    case BENCH_TYPE_SKYLAKE_512:
      bench->compute_function = compute_skylake_512;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_512_8);
      break;
    case BENCH_TYPE_SKYLAKE_256:    
      bench->compute_function = compute_skylake_256;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_8);          
      break;
    case BENCH_TYPE_BROADWELL:
      bench->compute_function = compute_broadwell;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_8);
      break;  
    case BENCH_TYPE_KABY_LAKE:
      bench->compute_function = compute_kaby_lake;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_8);
      break;  
    case BENCH_TYPE_COFFE_LAKE:
      bench->compute_function = compute_coffe_lake;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_8);
      break;  
    case BENCH_TYPE_ICE_LAKE:
      bench->compute_function = compute_ice_lake_256;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_8);        
      break;
    case BENCH_TYPE_KNIGHTS_LANDING:
      bench->compute_function = compute_knl;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_512_12);
      break;
    case BENCH_TYPE_ZEN:
      bench->compute_function = compute_zen;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_5);
      break;
    case BENCH_TYPE_ZEN_PLUS:
      bench->compute_function = compute_zen_plus;
      bench->gflops = compute_gflops(bench->n_threads, BENCH_256_5);
      break;    
    default:
      printf("ERROR: No valid benchmark! (bench: %d)\n", bench->benchmark_type);
      return false;
  }
  
  bench->name = bench_name[bench->benchmark_type];
  return true;
}

struct benchmark* init_benchmark(struct cpu* cpu, int n_threads, bench_type benchmark_type) {    
  struct benchmark* bench = malloc(sizeof(struct benchmark));
  
  if(n_threads > MAX_NUMBER_THREADS) {
    printf("ERROR: Max number of threads is %d\n", MAX_NUMBER_THREADS);
    return NULL;
  }
  
  bench->n_threads = n_threads;
  
  // Manual benchmark select
  if(benchmark_type != BENCH_TYPE_INVALID) {
    bench->benchmark_type = benchmark_type;
  }
  else {  // Automatic benchmark select
    struct uarch* uarch_struct = get_uarch_struct(cpu);
    MICROARCH u = uarch_struct->uarch;  
    bool avx = cpu_has_avx(cpu);
    bool avx512 = cpu_has_avx512(cpu);
    
    switch(u) {
      case UARCH_SANDY_BRIDGE:
        bench->benchmark_type = BENCH_TYPE_SANDY_BRIDGE;
        break;  
      case UARCH_IVY_BRIDGE:
        bench->benchmark_type = BENCH_TYPE_IVY_BRIDGE;
        break;  
      case UARCH_HASWELL:
        bench->benchmark_type = BENCH_TYPE_HASWELL;
        break;  
      case UARCH_SKYLAKE:
        if(avx512)
          bench->benchmark_type = BENCH_TYPE_SKYLAKE_512;
        else
          bench->benchmark_type = BENCH_TYPE_SKYLAKE_256;
        break;
      case UARCH_CASCADE_LAKE:
        bench->benchmark_type = BENCH_TYPE_SKYLAKE_512;
        break;  
      case UARCH_BROADWELL:
        bench->benchmark_type = BENCH_TYPE_BROADWELL;
        break;  
      case UARCH_KABY_LAKE:
        bench->benchmark_type = BENCH_TYPE_KABY_LAKE;
        break;  
      case UARCH_COFFE_LAKE:
        bench->benchmark_type = BENCH_TYPE_COFFE_LAKE;
        break;  
      case UARCH_ICE_LAKE:
        bench->benchmark_type = BENCH_TYPE_ICE_LAKE;
        break;
      case UARCH_KNIGHTS_LANDING:
        bench->benchmark_type = BENCH_TYPE_KNIGHTS_LANDING;
        break;  
      case UARCH_ZEN:
        bench->benchmark_type = BENCH_TYPE_ZEN;
        break;  
      case UARCH_ZEN_PLUS:
        bench->benchmark_type = BENCH_TYPE_ZEN_PLUS;
        break;  
      default:
        printf("ERROR: No valid uarch found! (uarch: %d)\n", u);
        return NULL;
    }
  }
  
  if(select_benchmark(bench))
    return bench;
  return NULL;
}

void compute(struct benchmark* bench) {      
  TYPE mult = {0};
  TYPE *farr_ptr = NULL;
  
  #pragma omp parallel for
  for(int t=0; t < bench->n_threads; t++)
    bench->compute_function(farr_ptr, mult, t);
}

double get_gflops(struct benchmark* bench) {
  return bench->gflops;    
}

char* get_benchmark_name(struct benchmark* bench) {
  return bench->name;    
}
