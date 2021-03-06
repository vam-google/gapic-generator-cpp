// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GAPIC_GENERATOR_CPP_GAX_RETRY_POLICY_H_
#define GAPIC_GENERATOR_CPP_GAX_RETRY_POLICY_H_

#include "gax/status.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <ratio>

namespace google {
namespace gax {

/**
 * Define the interface for controlling how clients retry RPC operations.
 *
 * Idempotent (and certain non-idempotent) operations can be retried
 * transparently to the user. However, we need to give the users enough
 * flexiblity to control when to stop retrying.
 *
 * The application provides an instance of this class when the client is
 * created.
 */
class RetryPolicy {
 public:
  virtual ~RetryPolicy() = default;

  /**
   * Return a new copy of this object with the same retry criteria and fresh
   * state.
   */
  virtual std::unique_ptr<RetryPolicy> clone() const = 0;

  /**
   * Handle an RPC failure
   *
   * @return true if the RPC operation should be retried.
   */
  virtual bool OnFailure(Status const& status) = 0;

  /**
   * Calculate the deadline for the next RPC operation.
   *
   * Any internal state modification, if neccessary, should occur in OnFailure.
   *
   * Note: this is different from the deadline in LimitedDurationRetryPolicy,
   *       which is the deadline after which retry attempts should be abandoned.
   *
   * @return the _deadline_ for the next RPC, NOT its maximum _duration_.
   */
  virtual std::chrono::system_clock::time_point OperationDeadline() const = 0;
};

class DefaultClock {
 public:
  std::chrono::system_clock::time_point now() const {
    return std::chrono::system_clock::now();
  }
};

/**
 * Implement a simple "count errors and then stop" retry policy.
 */
template <typename Clock = DefaultClock>
class LimitedErrorCountRetryPolicy : public RetryPolicy {
 public:
  template <typename Rep, typename Period>
  LimitedErrorCountRetryPolicy(int max_failures,
                               std::chrono::duration<Rep, Period> rpc_duration,
                               Clock c = Clock{})
      : c_(std::move(c)),
        rpc_duration_(std::chrono::duration_cast<std::chrono::milliseconds>(
            rpc_duration)),
        failure_count_(0),
        max_failures_(max_failures) {}

  LimitedErrorCountRetryPolicy(LimitedErrorCountRetryPolicy const& rhs) noexcept
      : LimitedErrorCountRetryPolicy(rhs.max_failures_, rhs.rpc_duration_,
                                     rhs.c_) {}

  LimitedErrorCountRetryPolicy(LimitedErrorCountRetryPolicy&& rhs) noexcept
      : LimitedErrorCountRetryPolicy(rhs.max_failures_, rhs.rpc_duration_,
                                     std::move(rhs.c_)) {}

  std::unique_ptr<RetryPolicy> clone() const override {
    return std::unique_ptr<RetryPolicy>(
        new LimitedErrorCountRetryPolicy(*this));
  }

  bool OnFailure(Status const& status) override {
    return !status.IsPermanentFailure() && failure_count_++ < max_failures_;
  }

  std::chrono::system_clock::time_point OperationDeadline() const override {
    return c_.now() + rpc_duration_;
  }

 private:
  Clock c_;
  std::chrono::milliseconds const rpc_duration_;
  int failure_count_;
  int const max_failures_;
};

/**
 * Implement a simple "keep trying for this time" retry policy.
 */
template <typename Clock = DefaultClock>
class LimitedDurationRetryPolicy : public RetryPolicy {
 public:
  template <typename Rep1, typename Period1, typename Rep2, typename Period2>
  LimitedDurationRetryPolicy(std::chrono::duration<Rep1, Period1> max_duration,
                             std::chrono::duration<Rep2, Period2> rpc_duration,
                             Clock c = Clock{})
      : c_(std::move(c)),
        rpc_duration_(std::chrono::duration_cast<std::chrono::milliseconds>(
            rpc_duration)),
        max_duration_(std::chrono::duration_cast<std::chrono::milliseconds>(
            max_duration)),
        deadline_(max_duration_ + c_.now()) {}

  LimitedDurationRetryPolicy(LimitedDurationRetryPolicy const& rhs) noexcept
      : LimitedDurationRetryPolicy(rhs.max_duration_, rhs.rpc_duration_,
                                   rhs.c_) {}

  LimitedDurationRetryPolicy(LimitedDurationRetryPolicy&& rhs) noexcept
      : LimitedDurationRetryPolicy(rhs.max_duration_, rhs.rpc_duration_,
                                   std::move(rhs.c_)) {}

  std::unique_ptr<RetryPolicy> clone() const override {
    return std::unique_ptr<RetryPolicy>(
        new LimitedDurationRetryPolicy<Clock>(*this));
  }

  bool OnFailure(Status const& status) override {
    return !status.IsPermanentFailure() && c_.now() < deadline_;
  }

  std::chrono::system_clock::time_point OperationDeadline() const override {
    return std::min(deadline_, c_.now() + rpc_duration_);
  }

 private:
  Clock c_;
  std::chrono::milliseconds const rpc_duration_;
  std::chrono::milliseconds const max_duration_;
  std::chrono::system_clock::time_point const deadline_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_RETRY_POLICY_H_
