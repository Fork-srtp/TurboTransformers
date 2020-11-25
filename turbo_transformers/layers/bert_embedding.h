

#pragma once
#include <memory>
#include <utility>

#include "turbo_transformers/core/tensor.h"

namespace turbo_transformers {
namespace layers {

class BERTEmbedding {
 public:
  BERTEmbedding(core::Tensor word_embeddings, core::Tensor position_embeddings,
                core::Tensor token_type_embeddings,
                core::Tensor layer_norm_weights, core::Tensor layer_norm_bias)
      : word_embedings_(
            std::move(word_embeddings)),  // [vocab_size, hidden_size]
        position_embeddings_(std::move(
            position_embeddings)),  // [max_position_embeddings, hidden_size]
        token_type_embeddings_(std::move(
            token_type_embeddings)),  // [token_type_vocab_size, hidden_size]
        layer_norm_weights_(std::move(layer_norm_weights)),  // [hidden_size]
        layer_norm_bias_(std::move(layer_norm_bias)) {       // [hidden_size]
    EnforceShapeAndType();
  }

  void EnforceShapeAndType() const;

  void operator()(const core::Tensor &input_ids,
                  const core::Tensor &position_ids,
                  const core::Tensor &token_type_ids,
                  core::Tensor *output) const;

 private:
  core::Tensor word_embedings_;
  core::Tensor position_embeddings_;
  core::Tensor token_type_embeddings_;
  core::Tensor layer_norm_weights_;
  core::Tensor layer_norm_bias_;
};

}  // namespace layers
}  // namespace turbo_transformers
