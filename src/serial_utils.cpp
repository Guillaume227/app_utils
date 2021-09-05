#include <app_utils/serial_utils.hpp>
#include <math.h> // ldexpf, frexpf

namespace app_utils::serial {

size_t from_bytes(std::byte const* buffer, size_t const buffer_size, float& val) {
  uint32_t res;
  from_bytes(buffer, buffer_size, res);
  int e = (res >> 23) & 0xFF;
  uint32_t sig_i = res & 0x7FFFFF;
  bool neg = res & (1u << 31);

  float sig = 0.0;
  if (e != 0 || sig_i != 0) {
    sig = sig_i / (8388608.0f * 2.0f) + 0.5f;
    e -= 126;
  }

  if (neg) {
    sig = -sig;
  }

  val = ldexpf(sig, e);
  return sizeof(float);
}

size_t to_bytes(std::byte* buffer, size_t buffer_size, float const& val) {
  int e = 0;
  float sig = frexpf(val, &e);
  float sig_abs = fabsf(sig);
  uint32_t sig_i = 0;

  if (sig_abs >= 0.5) {
    sig_i = (uint32_t)((sig_abs - 0.5f) * 2.0f * 8388608.0f);
    e += 126;
  }

  uint32_t res = ((e & 0xFF) << 23) | (sig_i & 0x7FFFFF);
  if (sig < 0) {
    res |= 1u << 31;
  }

  return to_bytes(buffer, buffer_size, res);
}

} // namespace app_utils::serial
