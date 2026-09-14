#include "formfactor/catalog_filter.hpp"

#include <locale>
#include <sstream>

namespace formfactor {
namespace {

const char* qualifier_name(QuantityValueQualifier qualifier) {
  switch (qualifier) {
    case QuantityValueQualifier::Nominal: return "nominal";
    case QuantityValueQualifier::Minimum: return "minimum";
    case QuantityValueQualifier::Typical: return "typical";
    case QuantityValueQualifier::Maximum: return "maximum";
  }
  return nullptr;
}

bool valid_property_id(const std::string& identifier) {
  if (identifier.empty() || identifier.front() < 'a' ||
      identifier.front() > 'z') return false;
  for (const char character : identifier) {
    const bool lowercase = character >= 'a' && character <= 'z';
    const bool digit = character >= '0' && character <= '9';
    if (!lowercase && !digit && character != '.' && character != '_' &&
        character != '-') return false;
  }
  const char last = identifier.back();
  return (last >= 'a' && last <= 'z') || (last >= '0' && last <= '9');
}

const char* relation_name(NumericRelation relation) {
  switch (relation) {
    case NumericRelation::AtLeast: return "at-least";
    case NumericRelation::AtMost: return "at-most";
    case NumericRelation::Equal: return "equal";
  }
  return nullptr;
}

const char* outcome_name(CatalogFilterOutcome outcome) {
  switch (outcome) {
    case CatalogFilterOutcome::Match: return "match";
    case CatalogFilterOutcome::NoMatch: return "no-match";
    case CatalogFilterOutcome::Unknown: return "unknown";
    case CatalogFilterOutcome::Invalid: return "invalid";
  }
  return "invalid";
}

void append_text(std::ostringstream& output, const char* name,
                 const std::string& value) {
  output << name << '=' << value.size() << ':' << value << '\n';
}

}  // namespace

CatalogFilterResult evaluate_catalog_quantity_filter(
    const CatalogEntry& entry, const CatalogQuantityFilter& filter) {
  CatalogFilterResult result;
  const auto validation = validate_catalog_entry(entry);
  if (!validation.linked_model_ready()) {
    result.errors.emplace_back("catalogue entry must pass validation before filtering");
    return result;
  }
  if (!valid_property_id(filter.property_id)) {
    result.errors.emplace_back("filter property id is invalid");
    return result;
  }
  const char* qualifier = qualifier_name(filter.qualifier);
  if (qualifier == nullptr) {
    result.errors.emplace_back("filter qualifier is unsupported");
    return result;
  }
  const char* relation = relation_name(filter.relation);
  if (relation == nullptr) {
    result.errors.emplace_back("filter relation is unsupported");
    return result;
  }

  const CatalogQuantityProperty* selected = nullptr;
  for (const auto& property : entry.quantity_properties) {
    if (property.property_id == filter.property_id &&
        property.qualifier == filter.qualifier) {
      selected = &property;
      break;
    }
  }

  if (selected == nullptr || selected->conditions != filter.conditions) {
    result.outcome = CatalogFilterOutcome::Unknown;
  } else {
    const auto comparison = compare(selected->value, filter.threshold);
    if (!comparison.has_value()) {
      result.errors.emplace_back(
          "filter threshold dimension must match the property dimension");
      return result;
    }
    bool match = false;
    switch (filter.relation) {
      case NumericRelation::AtLeast: match = *comparison >= 0; break;
      case NumericRelation::AtMost: match = *comparison <= 0; break;
      case NumericRelation::Equal: match = *comparison == 0; break;
    }
    result.outcome = match ? CatalogFilterOutcome::Match
                           : CatalogFilterOutcome::NoMatch;
  }

  std::ostringstream record;
  record.imbue(std::locale::classic());
  record << "formfactor-catalog-filter-v1\n";
  append_text(record, "catalog-id", entry.catalog_id);
  append_text(record, "property-id", filter.property_id);
  record << "qualifier=" << qualifier << '\n';
  record << "relation=" << relation << '\n';
  record << "threshold=" << canonical_quantity_record(filter.threshold) << '\n';
  record << "conditions-present=" << (filter.conditions.has_value() ? 1 : 0)
         << '\n';
  if (filter.conditions.has_value()) {
    append_text(record, "conditions", *filter.conditions);
  }
  record << "outcome=" << outcome_name(result.outcome) << '\n';
  result.canonical_record = record.str();
  return result;
}

}  // namespace formfactor
