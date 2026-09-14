#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace formfactor {

// Computes the lowercase hexadecimal SHA-256 digest defined by NIST FIPS
// 180-4. Inputs longer than the standard's 64-bit bit-length field can encode
// fail closed with no result.
[[nodiscard]] std::optional<std::string> sha256_hex(std::string_view bytes);
[[nodiscard]] bool valid_sha256_hex(std::string_view digest);

}  // namespace formfactor
