#include "formfactor/catalog.hpp"

#include <array>
#include <cassert>
#include <string>
#include <utility>

int main() {
  using formfactor::Dimension;
  using formfactor::QuantityValueQualifier;
  const std::array dimensions{
      std::pair{"capacitance", Dimension::Capacitance},
      std::pair{"current", Dimension::Current},
      std::pair{"frequency", Dimension::Frequency},
      std::pair{"inductance", Dimension::Inductance},
      std::pair{"length", Dimension::Length},
      std::pair{"power", Dimension::Power},
      std::pair{"resistance", Dimension::Resistance},
      std::pair{"time", Dimension::Time},
      std::pair{"voltage", Dimension::Voltage}};
  const std::array qualifiers{
      std::pair{"nominal", QuantityValueQualifier::Nominal},
      std::pair{"minimum", QuantityValueQualifier::Minimum},
      std::pair{"typical", QuantityValueQualifier::Typical},
      std::pair{"maximum", QuantityValueQualifier::Maximum}};

  for (const auto& [dimension_name, dimension] : dimensions) {
    for (const auto& [qualifier_name, qualifier] : qualifiers) {
      const std::string id = std::string{dimension_name} + "." + qualifier_name;
      const auto schema = formfactor::catalog_quantity_property_schema(id);
      assert(schema.has_value());
      assert(schema->property_id == id);
      assert(schema->dimension == dimension);
      assert(schema->qualifier == qualifier);
      assert(formfactor::catalog_quantity_property_schema(id)->property_id == id);
    }
  }

  assert(!formfactor::catalog_quantity_property_schema("temperature.maximum")
              .has_value());
  assert(!formfactor::catalog_quantity_property_schema("voltage.recommended")
              .has_value());
}
