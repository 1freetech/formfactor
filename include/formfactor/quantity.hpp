#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace formfactor {

// Exact decimal SI quantity support for truth-layer records.
// Authoritative unit references:
// - BIPM, The International System of Units (SI Brochure), 9th edition.
// - NIST Special Publication 811, Guide for the Use of the International
//   System of Units (SI).
//
// Offset units such as degrees Celsius are intentionally excluded from this
// initial slice because they require affine conversion rather than SI-prefix
// scaling. Missing or unsupported units must remain unknown at higher layers.
enum class Dimension {
  Dimensionless,
  Voltage,
  Current,
  Resistance,
  Capacitance,
  Inductance,
  Frequency,
  Length,
  Time,
  Power
};

enum class Unit {
  One,
  Volt,
  Millivolt,
  Microvolt,
  Ampere,
  Milliampere,
  Microampere,
  Ohm,
  Kiloohm,
  Megaohm,
  Farad,
  Millifarad,
  Microfarad,
  Nanofarad,
  Picofarad,
  Henry,
  Millihenry,
  Microhenry,
  Nanohenry,
  Hertz,
  Kilohertz,
  Megahertz,
  Gigahertz,
  Meter,
  Millimeter,
  Micrometer,
  Nanometer,
  Second,
  Millisecond,
  Microsecond,
  Nanosecond,
  Watt,
  Milliwatt
};

class Quantity {
 public:
  [[nodiscard]] std::int64_t coefficient() const { return coefficient_; }
  [[nodiscard]] int exponent10() const { return exponent10_; }
  [[nodiscard]] Dimension dimension() const { return dimension_; }

 private:
  constexpr Quantity(std::int64_t coefficient, int exponent10,
                     Dimension dimension)
      : coefficient_(coefficient),
        exponent10_(exponent10),
        dimension_(dimension) {}

  std::int64_t coefficient_{};
  int exponent10_{};
  Dimension dimension_{Dimension::Dimensionless};

  friend std::optional<Quantity> make_quantity(std::int64_t, int, Unit);
  friend std::optional<Quantity> make_si_quantity(std::int64_t, int,
                                                  Dimension);
};

// Constructs coefficient * 10^decimal_exponent * unit and stores it in exact
// canonical SI decimal form. No binary floating-point value is introduced.
// Unit values outside the explicitly supported enumeration fail closed.
[[nodiscard]] std::optional<Quantity> make_quantity(
    std::int64_t coefficient, int decimal_exponent, Unit unit);

// Constructs an already-SI decimal quantity at a validated import boundary.
// The supported exponent range [-30, +30] matches the current SI prefix range
// from quecto through quetta. Values outside it are rejected, not rounded.
// Dimension values outside the explicitly supported enumeration are rejected.
[[nodiscard]] std::optional<Quantity> make_si_quantity(
    std::int64_t coefficient, int exponent10, Dimension dimension);

// Returns -1, 0, or 1 for quantities of the same physical dimension.
// Different dimensions are not comparable and return std::nullopt.
[[nodiscard]] std::optional<int> compare(const Quantity& lhs,
                                         const Quantity& rhs);
[[nodiscard]] bool equivalent(const Quantity& lhs, const Quantity& rhs);

// Stable, locale-independent representation for reproducible records and
// tests. This is intentionally not a user-facing engineering formatter.
[[nodiscard]] std::string canonical_quantity_record(const Quantity& quantity);

}  // namespace formfactor
