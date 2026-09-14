#pragma once

#include "formfactor/catalog.hpp"

#include <optional>
#include <string>
#include <vector>

namespace formfactor {

enum class NumericRelation { AtLeast, AtMost, Equal };
enum class CatalogFilterOutcome { Match, NoMatch, Unknown, Invalid };

struct CatalogQuantityFilter {
  std::string property_id;
  QuantityValueQualifier qualifier{QuantityValueQualifier::Nominal};
  NumericRelation relation{NumericRelation::Equal};
  Quantity threshold;
  // A conditioned property is eligible only when this text exactly matches.
  // Absence means that only an unconditioned property may be evaluated.
  std::optional<std::string> conditions;
};

struct CatalogFilterResult {
  CatalogFilterOutcome outcome{CatalogFilterOutcome::Invalid};
  std::vector<std::string> errors;
  std::string canonical_record;
  [[nodiscard]] bool matched() const {
    return errors.empty() && outcome == CatalogFilterOutcome::Match;
  }
};

// Evaluates one exact numeric catalogue constraint. Missing values and
// condition mismatches remain Unknown; invalid records fail closed.
[[nodiscard]] CatalogFilterResult evaluate_catalog_quantity_filter(
    const CatalogEntry& entry, const CatalogQuantityFilter& filter);

}  // namespace formfactor
