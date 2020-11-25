

#pragma once
#include <chrono>
#include <string>

#include "loguru.hpp"
#include "turbo_transformers/layers/kernels/common.h"
#ifdef TT_WITH_CUDA
#include "turbo_transformers/core/cuda_device_context.h"
#endif

namespace turbo_transformers {
namespace benchmark {
class Timer {
 public:
  virtual double ElapseSecond() = 0;
};

#ifdef TT_WITH_CUDA
class GPUTimer : public Timer {
 public:
  explicit GPUTimer(cudaStream_t stream) : stream_(stream) {
    cudaEventCreate(&start_event_);
    cudaEventCreate(&stop_event_);
    cudaEventRecord(start_event_, stream_);
  }

  double ElapseSecond() override {
    cudaEventRecord(stop_event_, stream_);
    cudaEventSynchronize(stop_event_);
    float elapse;
    cudaEventElapsedTime(&elapse, start_event_, stop_event_);
    return elapse / 1000;
  }

 private:
  cudaEvent_t start_event_, stop_event_;
  cudaStream_t stream_;
};
#endif

class CPUTimer : public Timer {
 public:
  CPUTimer() : start_(std::chrono::system_clock::now()) {}

  void Reset() { start_ = std::chrono::system_clock::now(); }

  double ElapseSecond() override {
    auto end = std::chrono::system_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    return double(duration.count()) * std::chrono::microseconds::period::num /
           std::chrono::microseconds::period::den;
  }

 private:
  std::chrono::time_point<std::chrono::system_clock> start_;
};

template <typename Func>
float TestFuncSpeed(Func&& func, int step, const std::string& infor,
                    double g_bytes, DLDeviceType dev) {
  func();
  std::unique_ptr<Timer> timer;
  if (dev == kDLCPU) {
    timer = std::make_unique<CPUTimer>();
  } else if (dev == kDLGPU) {
#ifdef TT_WITH_CUDA
    static auto stream = core::CUDADeviceContext::GetInstance().stream();
    timer = std::make_unique<GPUTimer>(stream);
#else
    TT_THROW("Timer GPU is not supported");
#endif
  } else {
    TT_THROW("Timer device id %d is not supported", dev);
  }

  for (int i = 0; i < step; ++i) {
    func();
  }
  auto elapse = timer->ElapseSecond() / step;

  auto ret = g_bytes / elapse;
  return ret;
}

}  // namespace benchmark
}  // namespace turbo_transformers
