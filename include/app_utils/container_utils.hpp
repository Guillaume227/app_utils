#pragma once

#include <algorithm>

namespace app_utils {

template <typename T, typename UnaryPredicate>
bool all_of(T const& t, UnaryPredicate u) {
  return std::all_of(std::begin(t), std::end(t), u);
}

}  // namespace app_utils