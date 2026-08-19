// Include the binary logger.
#include <log/catalog/encoder.hpp>

#include <stdx/span.hpp>

#include <cstddef>
#include <cstdint>

namespace custom {
// Write the packed MIPI words to the UART as little-endian bytes.
struct writer {
  template <std::size_t N>
  auto operator()(stdx::span<std::uint32_t const, N> packet) const {
    for (auto const word : packet) {
#if defined(ARDUINO)
      Serial.write(static_cast<std::uint8_t>(word));
      Serial.write(static_cast<std::uint8_t>(word >> 8));
      Serial.write(static_cast<std::uint8_t>(word >> 16));
      Serial.write(static_cast<std::uint8_t>(word >> 24));
#else
      (void)word;
#endif
    }
  }
};
} // namespace custom

// Specialize the logging config variable template to use the binary logger with
// a UART writer. Every translation unit that logs must see the same
// specialization.
template <>
inline auto logging::config<> = logging::binary::config{custom::writer{}};
