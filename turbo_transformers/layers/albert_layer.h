

#pragma once
#include <memory>
#include <utility>

#include "turbo_transformers/core/tensor.h"

namespace turbo_transformers {
namespace layers {

class AlbertLayer {
 public:
  AlbertLayer(core::Tensor dense_weight, core::Tensor dense_bias,
              core::Tensor dense_output_weight, core::Tensor dense_output_bias,
              core::Tensor layer_norm_weight, core::Tensor layer_norm_bias)
      : dense_weight_(std::move(dense_weight)),
        dense_bias_(std::move(dense_bias)),
        dense_output_weight_(std::move(dense_output_weight)),
        dense_output_bias_(std::move(dense_output_bias)),
        layer_norm_weight_(std::move(layer_norm_weight)),
        layer_norm_bias_(std::move(layer_norm_bias)) {
    EnforceShapeAndType();
  }

  void EnforceShapeAndType() const;
  void operator()(const core::Tensor& input_tensor, core::Tensor* hidden_output,
                  core::Tensor* output) const;

 private:
  core::Tensor dense_weight_;
  core::Tensor dense_bias_;
  core::Tensor dense_output_weight_;
  core::Tensor dense_output_bias_;
  core::Tensor layer_norm_weight_;
  core::Tensor layer_norm_bias_;
};

}  // namespace layers
}  // namespace turbo_transformers
