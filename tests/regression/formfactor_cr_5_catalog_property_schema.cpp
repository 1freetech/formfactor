#include "formfactor/catalog.hpp"

#include <cassert>

int main() {
  using formfactor::Dimension;
  using formfactor::QuantityValueQualifier;

  const auto voltage =
      formfactor::catalog_quantity_property_schema("voltage.maximum");
  assert(voltage.has_value());
  assert(voltage->dimension == Dimension::Voltage);
  assert(voltage->qualifier == QuantityValueQualifier::Maximum);

  // A plausible-looking name is still unknown until its semantic contract is
  // implemented. No spelling-based dimension or qualifier inference is safe.
  assert(!formfactor::catalog_quantity_property_schema("temperature.maximum")
              .has_value());
  assert(!formfactor::catalog_quantity_property_schema("temperature.typical")
              .has_value());
}
