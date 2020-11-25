

#include "benchmark_help.h"
#include "catch2/catch.hpp"
#include "loguru.hpp"
#include "turbo_transformers/layers/kernels/common.h"
#include "turbo_transformers/layers/kernels/layer_norm.h"

#ifdef TT_WITH_CUDA
#include "turbo_transformers/core/cuda_device_context.h"
#endif

namespace turbo_transformers {
namespace layers {
namespace kernels {

static void LayerNormBenmarkHelper(int batch_size, int hidden_size,
                                   int seq_length, bool is_add_bias,
                                   const std::string& info, DLDeviceType dev,
                                   int n_step) {
  auto g_bytes = batch_size * seq_length * hidden_size * sizeof(float) / 1e9;
  auto input = common::CreateTensorAndFillRandom<float>(
      {batch_size, seq_length, hidden_size}, dev, 0);
  auto bias = common::CreateTensorAndFillRandom<float>({hidden_size}, dev, 0);
  auto gamma = common::CreateTensorAndFillRandom<float>({hidden_size}, dev, 0);
  auto beta = common::CreateTensorAndFillRandom<float>({hidden_size}, dev, 0);
  auto out = common::CreateTensorAndFillRandom<float>(
      {batch_size, seq_length, hidden_size}, dev, 0);

  if (is_add_bias) {
    benchmark::TestFuncSpeed(
        [&]() { AddBiasLayerNorm<float>(input, bias, gamma, beta, &out); },
        n_step, info, g_bytes, dev);
  } else {
    benchmark::TestFuncSpeed([&]() { LayerNorm<float>(gamma, beta, &out); },
                             n_step, info, g_bytes, dev);
  }
}

TEST_CASE("layernorm-cpu-benchmark") {
  constexpr int n_step = 150;
  std::vector<int64_t> hidden_size_list{12 * 64, 2000};
  std::vector<int64_t> batch_size_list{1, 20};
  std::vector<int64_t> seq_length_list{10,  20,  40,  60,  80,
                                       100, 200, 300, 400, 500};

  for (auto hidden_size : hidden_size_list) {
    for (auto batch_size : batch_size_list) {
      for (auto seq_length : seq_length_list) {
        std::stringstream ss;
        ss << "CPU LayerNorm " << batch_size << ", " << seq_length << ", "
           << hidden_size;
        LayerNormBenmarkHelper(batch_size, hidden_size, seq_length, true,
                               ss.str(), kDLCPU, n_step);
        ss << " AddBias";
        LayerNormBenmarkHelper(batch_size, hidden_size, seq_length, false,
                               ss.str(), kDLCPU, n_step);
      }  // for
    }
  }
}

#ifdef TT_WITH_CUDA
TEST_CASE("layernorm-gpu-benchmark") {
  constexpr int n_step = 150;

  int64_t hidden_size = 12 * 64;

  std::vector<int64_t> batch_size_list{1, 20};
  std::vector<int64_t> seq_length_list{10,  20,  40,  60,  80,
                                       100, 200, 300, 400, 500};
  for (auto batch_size : batch_size_list)
    for (auto seq_length : seq_length_list) {
      std::stringstream ss;
      ss << "GPU LayerNorm " << batch_size << ", " << seq_length << ", "
         << hidden_size;
      LayerNormBenmarkHelper(batch_size, hidden_size, seq_length, true,
                             ss.str(), kDLGPU, n_step);
      ss << " AddBias";
      LayerNormBenmarkHelper(batch_size, hidden_size, seq_length, false,
                             ss.str(), kDLGPU, n_step);
    }  // for
}
#endif

}  // namespace kernels
}  // namespace layers
}  // namespace turbo_transformers
