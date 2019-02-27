// Copyright 2019 Google Inc.  All rights reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef GOOGLE_API_CODEGEN_INTERNAL_GAPIC_COMMON_TEMPLATES_H_
#define GOOGLE_API_CODEGEN_INTERNAL_GAPIC_COMMON_TEMPLATES_H_

#include <memory>

namespace google {
namespace api {
namespace codegen {
namespace internal {

constexpr char kIncludeTemplate[] = R"(
#include $include$)";

constexpr char kNamespaceStartTemplate[] = R"(
namespace $namespace$ {)";

constexpr char kNamespaceEndTemplate[] = R"(
} // namespace $namespace$)";

} // namespace internal
} // namespace codegen
} // namespace api
} // namespace google

#endif // GOOGLE_API_CODEGEN_INTERNAL_GAPIC_COMMON_TEMPLATES_H_
