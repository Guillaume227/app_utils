#pragma once

namespace app_utils {

template <typename A0, typename... Args>
constexpr bool all_equal(A0 const& a0, Args const&... args) {
  // check input values are all equal
  return ((args == a0) and ...);
}

template <typename Arg, typename... Args>
consteval bool strictly_increasing(Arg const& arg0, Args const&... args) {

  if constexpr (sizeof...(args) == 0) {
    (void) arg0; // avoid unreferenced formal parameter compiler warning
    return true;
  } else {
    Arg const* arg = &arg0;
    return ((*arg < args ? (arg = &args, true) : false) && ...);
  }
}

template <size_t arg0, size_t... args>
consteval bool strictly_increasing() {

  if constexpr (sizeof...(args) == 0) {
    (void) arg0; // avoid unreferenced formal parameter compiler warning
    return true;
  } else {
    size_t arg = arg0;
    return ((arg < args ? (arg = args, true) : false) && ...);
  }
}

}  // namespace app_utils
