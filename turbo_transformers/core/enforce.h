

#pragma once
#include <array>
#include <stdexcept>
#include <string>
#ifdef TT_WITH_CUDA
#include <cublas_v2.h>
#include <cuda_runtime.h>
#endif

#include "absl/debugging/stacktrace.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
namespace turbo_transformers {
namespace core {
namespace details {
/**
 * Implement enforce macros.
 *
 * Will throw `EnforceNotMet` when enforce check failed. Unlike GLOG and
 * std::assert, it:
 *   1. let the program catch the exception and recover from bad state.
 *   2. enforce carry a rich information about call stack. It is useful for
 * debugging.
 */

static constexpr size_t kStackLimit = 20UL;
static constexpr size_t kStackSkipCount = 1UL;
class EnforceNotMet : public std::exception {
 public:
  explicit EnforceNotMet(std::string msg) : msg_(std::move(msg)) {
    n_ = absl::GetStackTrace(stacks_.data(), stacks_.size(), kStackSkipCount);
  }

  const char *what() const noexcept override;

 private:
  mutable std::string msg_;
  std::array<void *, kStackLimit> stacks_{};
  size_t n_;
  mutable bool stack_added_{false};
};
}  // namespace details

#if !defined(_WIN32)
#define TT_UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)
#else
#define TT_UNLIKELY(condition) (condition)
#endif

#define TT_THROW(...)                                         \
  do {                                                        \
    throw ::turbo_transformers::core::details::EnforceNotMet( \
        absl::StrFormat(__VA_ARGS__));                        \
  } while (false)

#define TT_ENFORCE(cond, ...)                                            \
  do {                                                                   \
    if (TT_UNLIKELY(!(cond))) {                                          \
      std::string err_msg("enforce error");                              \
      err_msg += #cond;                                                  \
      err_msg += absl::StrFormat(" at %s:%d\n", __FILE__, __LINE__);     \
      err_msg += absl::StrFormat(__VA_ARGS__);                           \
      throw ::turbo_transformers::core::details::EnforceNotMet(err_msg); \
    }                                                                    \
  } while (false)

#define TT_ENFORCE_EQ(a, b, ...) TT_ENFORCE((a) == (b), __VA_ARGS__)
#define TT_ENFORCE_NE(a, b, ...) TT_ENFORCE((a) != (b), __VA_ARGS__)
#define TT_ENFORCE_LT(a, b, ...) TT_ENFORCE((a) < (b), __VA_ARGS__)
#define TT_ENFORCE_LE(a, b, ...) TT_ENFORCE((a) <= (b), __VA_ARGS__)
#define TT_ENFORCE_GT(a, b, ...) TT_ENFORCE((a) > (b), __VA_ARGS__)
#define TT_ENFORCE_GE(a, b, ...) TT_ENFORCE((a) >= (b), __VA_ARGS__)

}  // namespace core
}  // namespace turbo_transformers
