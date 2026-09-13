#include "pcbtech/quantity.hpp"

#include <cassert>
#include <cstdint>
#include <limits>

int main() {
  using pcbtech::Dimension;
  using pcbtech::Unit;

  // Equivalent SI prefixes compare exactly without binary floating point.
  const auto one_volt = pcbtech::make_quantity(1, 0, Unit::Volt);
  const auto thousand_mv = pcbtech::make_quantity(1000, 0, Unit::Millivolt);
  assert(one_volt && thousand_mv);
  assert(pcbtech::equivalent(*one_volt, *thousand_mv));

  const auto one_uf = pcbtech::make_quantity(1, 0, Unit::Microfarad);
  const auto thousand_nf = pcbtech::make_quantity(1000, 0, Unit::Nanofarad);
  const auto million_pf = pcbtech::make_quantity(1000000, 0, Unit::Picofarad);
  assert(one_uf && thousand_nf && million_pf);
  assert(pcbtech::equivalent(*one_uf, *thousand_nf));
  assert(pcbtech::equivalent(*one_uf, *million_pf));

  // Decimal values remain exact: 2.2 kohm == 2200 ohm.
  const auto two_point_two_kohm =
      pcbtech::make_quantity(22, -1, Unit::Kiloohm);
  const auto twenty_two_hundred_ohm =
      pcbtech::make_quantity(2200, 0, Unit::Ohm);
  assert(two_point_two_kohm && twenty_two_hundred_ohm);
  assert(pcbtech::equivalent(*two_point_two_kohm,
                             *twenty_two_hundred_ohm));
  assert(pcbtech::canonical_quantity_record(*two_point_two_kohm) ==
         "resistance:22e2");

  // Physical dimensions are never silently compared.
  const auto volts = pcbtech::make_quantity(5, 0, Unit::Volt);
  const auto amps = pcbtech::make_quantity(5, 0, Unit::Ampere);
  assert(volts && amps);
  assert(!pcbtech::compare(*volts, *amps).has_value());
  assert(!pcbtech::equivalent(*volts, *amps));

  // Ordering works across prefixes and for signed exact quantities.
  const auto low = pcbtech::make_quantity(999, 0, Unit::Millivolt);
  assert(low && pcbtech::compare(*low, *one_volt) == -1);

  const auto negative =
      pcbtech::make_si_quantity(-15, -9, Dimension::Time);
  const auto less_negative =
      pcbtech::make_si_quantity(-14, -9, Dimension::Time);
  assert(negative && less_negative);
  assert(pcbtech::compare(*negative, *less_negative) == -1);

  // Zero has one deterministic canonical representation regardless of input.
  const auto zero_a =
      pcbtech::make_si_quantity(0, -30, Dimension::Current);
  const auto zero_b =
      pcbtech::make_si_quantity(0, 30, Dimension::Current);
  assert(zero_a && zero_b && pcbtech::equivalent(*zero_a, *zero_b));
  assert(pcbtech::canonical_quantity_record(*zero_a) == "current:0e0");

  // Boundary coefficients, including INT64_MIN, remain orderable exactly.
  const auto huge = pcbtech::make_si_quantity(
      std::numeric_limits<std::int64_t>::max(), 9, Dimension::Frequency);
  const auto smaller = pcbtech::make_si_quantity(
      std::numeric_limits<std::int64_t>::max() - 1, 9,
      Dimension::Frequency);
  assert(huge && smaller && pcbtech::compare(*huge, *smaller) == 1);

  const auto most_negative = pcbtech::make_si_quantity(
      std::numeric_limits<std::int64_t>::min(), 0, Dimension::Power);
  const auto next_negative = pcbtech::make_si_quantity(
      std::numeric_limits<std::int64_t>::min() + 1, 0,
      Dimension::Power);
  assert(most_negative && next_negative);
  assert(pcbtech::compare(*most_negative, *next_negative) == -1);

  // Unsupported exponent ranges are rejected rather than rounded/clamped.
  assert(!pcbtech::make_si_quantity(1, -31, Dimension::Length));
  assert(!pcbtech::make_si_quantity(1, 31, Dimension::Length));
  assert(!pcbtech::make_quantity(1, 30, Unit::Gigahertz));
}
