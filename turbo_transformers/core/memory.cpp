

#include "turbo_transformers/core/memory.h"

#include <cstring>
#include <vector>
#ifdef TT_WITH_CUDA
#include "turbo_transformers/core/cuda_enforce.cuh"
#endif

namespace turbo_transformers {
namespace core {
void *align_alloc(size_t sz, size_t align) {
  void *aligned_mem;
  TT_ENFORCE_EQ(posix_memalign(&aligned_mem, align, sz), 0,
                "Cannot allocate align memory with %d bytes, "
                "align %d",
                sz, align);
  return aligned_mem;
}
using MemcpyFuncTypes = std::function<void *(void *, const void *, size_t)>;

static std::vector<MemcpyFuncTypes> InitMemcpyFuncs() {
  std::vector<MemcpyFuncTypes> results(
      static_cast<size_t>(MemcpyFlag::kNUM_MEMCPY_FLAGS));
  results[static_cast<size_t>(MemcpyFlag::kCPU2CPU)] = std::memcpy;

#ifdef TT_WITH_CUDA
  for (auto &pair : std::vector<std::pair<MemcpyFlag, cudaMemcpyKind>>{
           {MemcpyFlag::kCPU2GPU, cudaMemcpyHostToDevice},
           {MemcpyFlag::kGPU2CPU, cudaMemcpyDeviceToHost},
           {MemcpyFlag::kGPU2GPU, cudaMemcpyDeviceToDevice}}) {
    auto flag = pair.second;
    results[static_cast<size_t>(pair.first)] =
        [flag](void *dst, const void *src, size_t n) -> void * {
      TT_ENFORCE_CUDA_SUCCESS(cudaMemcpy(dst, src, n, flag));
      return dst;
    };
  }

#endif
  return results;
}

void Memcpy(void *dst_data, const void *src_data, size_t data_size,
            MemcpyFlag flag) {
  if (data_size <= 0) return;
  static auto memcpyFuncs = InitMemcpyFuncs();
  auto f = static_cast<size_t>(flag);
  if (f >= memcpyFuncs.size()) {
    TT_THROW("The MemcpyFlag %d is not support now.", f);
  }
  auto &func = memcpyFuncs[f];
  if (!func) {
    TT_THROW(
        "The MemcpyFlag %d is not support since turbo transformers is not "
        "compiled with this device support",
        f);
  }
  func(dst_data, src_data, data_size);
}
MemcpyFlag ToMemcpyFlag(DLDeviceType dst, DLDeviceType src) {
  if (dst == DLDeviceType::kDLCPU) {
    return src == DLDeviceType::kDLCPU ? MemcpyFlag::kCPU2CPU
                                       : MemcpyFlag::kGPU2CPU;
  } else {
    return src == DLDeviceType::kDLCPU ? MemcpyFlag::kCPU2GPU
                                       : MemcpyFlag::kGPU2GPU;
  }
}

}  // namespace core
}  // namespace turbo_transformers
