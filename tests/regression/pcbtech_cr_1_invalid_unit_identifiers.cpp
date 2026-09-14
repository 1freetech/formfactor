#include "pcbtech/quantity.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string_view>

namespace {

constexpr std::string_view kCrId = "pcbtech-cr-1";

int fail(std::string_view reason) {
  std::cerr << kCrId << ": implemented gate failed: " << reason << '\n';
  return 1;
}

}  // namespace

int main() {
  using pcbtech::Dimension;
  using pcbtech::Unit;

  // CR pcbtech-cr-1 protects the implemented fail-closed import boundary.
  // Invalid identifiers must never create a Quantity that could reach a
  // canonical record or a higher-level engineering export.
  const std::array invalid_units{
      -1, 33, 1000, std::numeric_limits<int>::max()};
  const std::array invalid_dimensions{
      -1, 10, 1000, std::numeric_limits<int>::max()};
  const std::array coefficients{
      std::numeric_limits<std::int64_t>::min(), std::int64_t{0},
      std::numeric_limits<std::int64_t>::max()};
  const std::array exponents{-30, 0, 30};

  for (const auto raw_unit : invalid_units) {
    const auto unit = static_cast<Unit>(raw_unit);
    for (const auto coefficient : coefficients) {
      for (const auto exponent : exponents) {
        const auto first =
            pcbtech::make_quantity(coefficient, exponent, unit);
        const auto replay =
            pcbtech::make_quantity(coefficient, exponent, unit);
        if (first.has_value() || replay.has_value()) {
          return fail("unsupported Unit produced a Quantity");
        }
      }
    }
  }

  for (const auto raw_dimension : invalid_dimensions) {
    const auto dimension = static_cast<Dimension>(raw_dimension);
    for (const auto coefficient : coefficients) {
      for (const auto exponent : exponents) {
        const auto first =
            pcbtech::make_si_quantity(coefficient, exponent, dimension);
        const auto replay =
            pcbtech::make_si_quantity(coefficient, exponent, dimension);
        if (first.has_value() || replay.has_value()) {
          return fail("unsupported Dimension produced a Quantity");
        }
      }
    }
  }

  // Supported identifiers at both ends of the current enums remain valid.
  const auto first_unit = pcbtech::make_quantity(
      std::numeric_limits<std::int64_t>::min(), -30, Unit::One);
  const auto last_unit = pcbtech::make_quantity(
      std::numeric_limits<std::int64_t>::max(), 30, Unit::Milliwatt);
  const auto first_dimension = pcbtech::make_si_quantity(
      std::numeric_limits<std::int64_t>::min(), -30,
      Dimension::Dimensionless);
  const auto last_dimension = pcbtech::make_si_quantity(
      std::numeric_limits<std::int64_t>::max(), 30, Dimension::Power);
  if (!first_unit || !last_unit || !first_dimension || !last_dimension) {
    return fail("supported boundary identifier was rejected");
  }

  const auto last_unit_replay = pcbtech::make_quantity(
      std::numeric_limits<std::int64_t>::max(), 30, Unit::Milliwatt);
  if (!last_unit_replay ||
      pcbtech::canonical_quantity_record(*last_unit) !=
          pcbtech::canonical_quantity_record(*last_unit_replay)) {
    return fail("supported boundary replay was not deterministic");
  }

  std::cout << kCrId
            << ": implemented gate passed: unsupported unit and dimension "
               "identifiers rejected\n";
  return 0;
}
