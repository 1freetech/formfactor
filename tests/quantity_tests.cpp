#include "formfactor/quantity.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>

int main() {
  using formfactor::Dimension;
  using formfactor::Unit;

  // Equivalent SI prefixes compare exactly without binary floating point.
  const auto one_volt = formfactor::make_quantity(1, 0, Unit::Volt);
  const auto thousand_mv = formfactor::make_quantity(1000, 0, Unit::Millivolt);
  assert(one_volt && thousand_mv);
  assert(formfactor::equivalent(*one_volt, *thousand_mv));

  const auto one_uf = formfactor::make_quantity(1, 0, Unit::Microfarad);
  const auto thousand_nf = formfactor::make_quantity(1000, 0, Unit::Nanofarad);
  const auto million_pf = formfactor::make_quantity(1000000, 0, Unit::Picofarad);
  assert(one_uf && thousand_nf && million_pf);
  assert(formfactor::equivalent(*one_uf, *thousand_nf));
  assert(formfactor::equivalent(*one_uf, *million_pf));

  // Decimal values remain exact: 2.2 kohm == 2200 ohm.
  const auto two_point_two_kohm =
      formfactor::make_quantity(22, -1, Unit::Kiloohm);
  const auto twenty_two_hundred_ohm =
      formfactor::make_quantity(2200, 0, Unit::Ohm);
  assert(two_point_two_kohm && twenty_two_hundred_ohm);
  assert(formfactor::equivalent(*two_point_two_kohm,
                             *twenty_two_hundred_ohm));
  assert(formfactor::canonical_quantity_record(*two_point_two_kohm) ==
         "resistance:22e2");

  // Physical dimensions are never silently compared.
  const auto volts = formfactor::make_quantity(5, 0, Unit::Volt);
  const auto amps = formfactor::make_quantity(5, 0, Unit::Ampere);
  assert(volts && amps);
  assert(!formfactor::compare(*volts, *amps).has_value());
  assert(!formfactor::equivalent(*volts, *amps));

  // Ordering works across prefixes and for signed exact quantities.
  const auto low = formfactor::make_quantity(999, 0, Unit::Millivolt);
  assert(low && formfactor::compare(*low, *one_volt) == -1);

  const auto negative =
      formfactor::make_si_quantity(-15, -9, Dimension::Time);
  const auto less_negative =
      formfactor::make_si_quantity(-14, -9, Dimension::Time);
  assert(negative && less_negative);
  assert(formfactor::compare(*negative, *less_negative) == -1);

  // Zero has one deterministic canonical representation regardless of input.
  const auto zero_a =
      formfactor::make_si_quantity(0, -30, Dimension::Current);
  const auto zero_b =
      formfactor::make_si_quantity(0, 30, Dimension::Current);
  assert(zero_a && zero_b && formfactor::equivalent(*zero_a, *zero_b));
  assert(formfactor::canonical_quantity_record(*zero_a) == "current:0e0");

  // Boundary coefficients, including INT64_MIN, remain orderable exactly.
  const auto huge = formfactor::make_si_quantity(
      std::numeric_limits<std::int64_t>::max(), 9, Dimension::Frequency);
  const auto smaller = formfactor::make_si_quantity(
      std::numeric_limits<std::int64_t>::max() - 1, 9,
      Dimension::Frequency);
  assert(huge && smaller && formfactor::compare(*huge, *smaller) == 1);

  const auto most_negative = formfactor::make_si_quantity(
      std::numeric_limits<std::int64_t>::min(), 0, Dimension::Power);
  const auto next_negative = formfactor::make_si_quantity(
      std::numeric_limits<std::int64_t>::min() + 1, 0,
      Dimension::Power);
  assert(most_negative && next_negative);
  assert(formfactor::compare(*most_negative, *next_negative) == -1);

  // Unsupported exponent ranges are rejected rather than rounded/clamped.
  assert(!formfactor::make_si_quantity(1, -31, Dimension::Length));
  assert(!formfactor::make_si_quantity(1, 31, Dimension::Length));
  assert(!formfactor::make_quantity(1, 30, Unit::Gigahertz));

  // Every declared unit maps to its physical dimension. These checks guard
  // the fail-closed switch from accidentally rejecting or remapping a unit.
  const std::array<std::pair<Unit, Dimension>, 33> supported_units{{
      {Unit::One, Dimension::Dimensionless},
      {Unit::Volt, Dimension::Voltage},
      {Unit::Millivolt, Dimension::Voltage},
      {Unit::Microvolt, Dimension::Voltage},
      {Unit::Ampere, Dimension::Current},
      {Unit::Milliampere, Dimension::Current},
      {Unit::Microampere, Dimension::Current},
      {Unit::Ohm, Dimension::Resistance},
      {Unit::Kiloohm, Dimension::Resistance},
      {Unit::Megaohm, Dimension::Resistance},
      {Unit::Farad, Dimension::Capacitance},
      {Unit::Millifarad, Dimension::Capacitance},
      {Unit::Microfarad, Dimension::Capacitance},
      {Unit::Nanofarad, Dimension::Capacitance},
      {Unit::Picofarad, Dimension::Capacitance},
      {Unit::Henry, Dimension::Inductance},
      {Unit::Millihenry, Dimension::Inductance},
      {Unit::Microhenry, Dimension::Inductance},
      {Unit::Nanohenry, Dimension::Inductance},
      {Unit::Hertz, Dimension::Frequency},
      {Unit::Kilohertz, Dimension::Frequency},
      {Unit::Megahertz, Dimension::Frequency},
      {Unit::Gigahertz, Dimension::Frequency},
      {Unit::Meter, Dimension::Length},
      {Unit::Millimeter, Dimension::Length},
      {Unit::Micrometer, Dimension::Length},
      {Unit::Nanometer, Dimension::Length},
      {Unit::Second, Dimension::Time},
      {Unit::Millisecond, Dimension::Time},
      {Unit::Microsecond, Dimension::Time},
      {Unit::Nanosecond, Dimension::Time},
      {Unit::Watt, Dimension::Power},
      {Unit::Milliwatt, Dimension::Power},
  }};
  for (const auto& [unit, expected_dimension] : supported_units) {
    const auto quantity = formfactor::make_quantity(1, 0, unit);
    const auto replay = formfactor::make_quantity(1, 0, unit);
    assert(quantity && replay);
    assert(quantity->dimension() == expected_dimension);
    assert(formfactor::canonical_quantity_record(*quantity) ==
           formfactor::canonical_quantity_record(*replay));
  }

  const std::array supported_dimensions{
      Dimension::Dimensionless, Dimension::Voltage, Dimension::Current,
      Dimension::Resistance, Dimension::Capacitance, Dimension::Inductance,
      Dimension::Frequency, Dimension::Length, Dimension::Time,
      Dimension::Power};
  for (const auto dimension : supported_dimensions) {
    const auto quantity = formfactor::make_si_quantity(1, 0, dimension);
    assert(quantity && quantity->dimension() == dimension);
  }

  // Unsupported identifiers must fail closed instead of becoming a real
  // dimension or silently falling back to a dimensionless quantity. Test the
  // adjacent boundaries and distant values with signed and zero coefficients.
  const std::array invalid_dimensions{
      -1, 10, 1000, std::numeric_limits<int>::max()};
  const std::array invalid_units{
      -1, 33, 1000, std::numeric_limits<int>::max()};
  const std::array coefficients{
      std::numeric_limits<std::int64_t>::min(), std::int64_t{0},
      std::numeric_limits<std::int64_t>::max()};
  const std::array exponents{-30, 0, 30};
  for (const auto raw : invalid_dimensions) {
    const auto dimension = static_cast<Dimension>(raw);
    for (const auto coefficient : coefficients) {
      for (const auto exponent : exponents) {
        assert(!formfactor::make_si_quantity(coefficient, exponent, dimension));
        assert(!formfactor::make_si_quantity(coefficient, exponent, dimension));
      }
    }
  }
  for (const auto raw : invalid_units) {
    const auto unit = static_cast<Unit>(raw);
    for (const auto coefficient : coefficients) {
      for (const auto exponent : exponents) {
        assert(!formfactor::make_quantity(coefficient, exponent, unit));
        assert(!formfactor::make_quantity(coefficient, exponent, unit));
      }
    }
  }
}
