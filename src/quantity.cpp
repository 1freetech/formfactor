#include "formfactor/quantity.hpp"

#include <cstdint>
#include <string>

namespace formfactor {
namespace {

constexpr int kMinExponent10 = -30;
constexpr int kMaxExponent10 = 30;

struct UnitDefinition {
  Dimension dimension;
  int scale_exponent10;
  bool supported{true};
};

constexpr bool valid_dimension(Dimension dimension) {
  switch (dimension) {
    case Dimension::Dimensionless:
    case Dimension::Voltage:
    case Dimension::Current:
    case Dimension::Resistance:
    case Dimension::Capacitance:
    case Dimension::Inductance:
    case Dimension::Frequency:
    case Dimension::Length:
    case Dimension::Time:
    case Dimension::Power:
      return true;
  }
  return false;
}

constexpr UnitDefinition definition(Unit unit) {
  switch (unit) {
    case Unit::One: return {Dimension::Dimensionless, 0};
    case Unit::Volt: return {Dimension::Voltage, 0};
    case Unit::Millivolt: return {Dimension::Voltage, -3};
    case Unit::Microvolt: return {Dimension::Voltage, -6};
    case Unit::Ampere: return {Dimension::Current, 0};
    case Unit::Milliampere: return {Dimension::Current, -3};
    case Unit::Microampere: return {Dimension::Current, -6};
    case Unit::Ohm: return {Dimension::Resistance, 0};
    case Unit::Kiloohm: return {Dimension::Resistance, 3};
    case Unit::Megaohm: return {Dimension::Resistance, 6};
    case Unit::Farad: return {Dimension::Capacitance, 0};
    case Unit::Millifarad: return {Dimension::Capacitance, -3};
    case Unit::Microfarad: return {Dimension::Capacitance, -6};
    case Unit::Nanofarad: return {Dimension::Capacitance, -9};
    case Unit::Picofarad: return {Dimension::Capacitance, -12};
    case Unit::Henry: return {Dimension::Inductance, 0};
    case Unit::Millihenry: return {Dimension::Inductance, -3};
    case Unit::Microhenry: return {Dimension::Inductance, -6};
    case Unit::Nanohenry: return {Dimension::Inductance, -9};
    case Unit::Hertz: return {Dimension::Frequency, 0};
    case Unit::Kilohertz: return {Dimension::Frequency, 3};
    case Unit::Megahertz: return {Dimension::Frequency, 6};
    case Unit::Gigahertz: return {Dimension::Frequency, 9};
    case Unit::Meter: return {Dimension::Length, 0};
    case Unit::Millimeter: return {Dimension::Length, -3};
    case Unit::Micrometer: return {Dimension::Length, -6};
    case Unit::Nanometer: return {Dimension::Length, -9};
    case Unit::Second: return {Dimension::Time, 0};
    case Unit::Millisecond: return {Dimension::Time, -3};
    case Unit::Microsecond: return {Dimension::Time, -6};
    case Unit::Nanosecond: return {Dimension::Time, -9};
    case Unit::Watt: return {Dimension::Power, 0};
    case Unit::Milliwatt: return {Dimension::Power, -3};
  }
  return {Dimension::Dimensionless, 0, false};
}

struct DecimalValue {
  std::int64_t coefficient;
  int exponent10;
};

DecimalValue normalize(std::int64_t coefficient, int exponent10) {
  if (coefficient == 0) return {0, 0};
  while (coefficient % 10 == 0 && exponent10 < kMaxExponent10) {
    coefficient /= 10;
    ++exponent10;
  }
  return {coefficient, exponent10};
}

unsigned decimal_digits(std::uint64_t value) {
  unsigned digits = 1;
  while (value >= 10) {
    value /= 10;
    ++digits;
  }
  return digits;
}

std::uint64_t magnitude(std::int64_t value) {
  if (value >= 0) return static_cast<std::uint64_t>(value);
  // Avoid signed overflow when value is INT64_MIN.
  return static_cast<std::uint64_t>(-(value + 1)) + 1U;
}

// formfactor currently targets GCC/Clang on Linux and already requires 128-bit
// integer support for exact geometry comparisons. __extension__ prevents the
// known compiler extension from being reported as a pedantic warning here.
__extension__ using Int128 = __int128;

Int128 pow10_128(unsigned exponent) {
  Int128 value = 1;
  for (unsigned i = 0; i < exponent; ++i) value *= 10;
  return value;
}

std::string dimension_name(Dimension dimension) {
  switch (dimension) {
    case Dimension::Dimensionless: return "dimensionless";
    case Dimension::Voltage: return "voltage";
    case Dimension::Current: return "current";
    case Dimension::Resistance: return "resistance";
    case Dimension::Capacitance: return "capacitance";
    case Dimension::Inductance: return "inductance";
    case Dimension::Frequency: return "frequency";
    case Dimension::Length: return "length";
    case Dimension::Time: return "time";
    case Dimension::Power: return "power";
  }
  return "invalid";
}

}  // namespace

std::optional<Quantity> make_si_quantity(std::int64_t coefficient,
                                         int exponent10,
                                         Dimension dimension) {
  if (!valid_dimension(dimension)) return std::nullopt;
  if (exponent10 < kMinExponent10 || exponent10 > kMaxExponent10) {
    return std::nullopt;
  }
  const auto normalized = normalize(coefficient, exponent10);
  return Quantity{normalized.coefficient, normalized.exponent10, dimension};
}

std::optional<Quantity> make_quantity(std::int64_t coefficient,
                                      int decimal_exponent,
                                      Unit unit) {
  if (decimal_exponent < kMinExponent10 || decimal_exponent > kMaxExponent10) {
    return std::nullopt;
  }
  const auto unit_definition = definition(unit);
  if (!unit_definition.supported) return std::nullopt;
  const int combined_exponent =
      decimal_exponent + unit_definition.scale_exponent10;
  if (combined_exponent < kMinExponent10 ||
      combined_exponent > kMaxExponent10) {
    return std::nullopt;
  }
  return make_si_quantity(coefficient, combined_exponent,
                          unit_definition.dimension);
}

std::optional<int> compare(const Quantity& lhs, const Quantity& rhs) {
  if (lhs.dimension() != rhs.dimension()) return std::nullopt;

  if (lhs.coefficient() == 0 && rhs.coefficient() == 0) return 0;
  if (lhs.coefficient() < 0 && rhs.coefficient() >= 0) return -1;
  if (lhs.coefficient() >= 0 && rhs.coefficient() < 0) return 1;

  const bool negative = lhs.coefficient() < 0;
  const auto lhs_magnitude = magnitude(lhs.coefficient());
  const auto rhs_magnitude = magnitude(rhs.coefficient());
  const int lhs_order =
      static_cast<int>(decimal_digits(lhs_magnitude)) + lhs.exponent10();
  const int rhs_order =
      static_cast<int>(decimal_digits(rhs_magnitude)) + rhs.exponent10();

  if (lhs_order != rhs_order) {
    const int magnitude_compare = lhs_order < rhs_order ? -1 : 1;
    return negative ? -magnitude_compare : magnitude_compare;
  }

  const int common_exponent =
      lhs.exponent10() < rhs.exponent10() ? lhs.exponent10()
                                          : rhs.exponent10();
  const auto lhs_scale =
      static_cast<unsigned>(lhs.exponent10() - common_exponent);
  const auto rhs_scale =
      static_cast<unsigned>(rhs.exponent10() - common_exponent);

  // Equal decimal order bounds the alignment gap to the digit-width gap of
  // two int64 coefficients, so the products remain inside signed 128-bit.
  const Int128 lhs_aligned =
      static_cast<Int128>(lhs.coefficient()) * pow10_128(lhs_scale);
  const Int128 rhs_aligned =
      static_cast<Int128>(rhs.coefficient()) * pow10_128(rhs_scale);

  if (lhs_aligned < rhs_aligned) return -1;
  if (lhs_aligned > rhs_aligned) return 1;
  return 0;
}

bool equivalent(const Quantity& lhs, const Quantity& rhs) {
  const auto comparison = compare(lhs, rhs);
  return comparison.has_value() && *comparison == 0;
}

std::string canonical_quantity_record(const Quantity& quantity) {
  return dimension_name(quantity.dimension()) + ":" +
         std::to_string(quantity.coefficient()) + "e" +
         std::to_string(quantity.exponent10());
}

}  // namespace formfactor
