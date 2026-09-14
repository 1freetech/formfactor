#include "formfactor/key_authority.hpp"

#include "formfactor/hash.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <string_view>

namespace formfactor {
namespace {

bool valid_record_text(std::string_view text) {
  bool visible = false;
  for (const unsigned char character : text) {
    if (character <= 0x20U || character == 0x7fU) {
      if (character != ' ') return false;
    } else {
      visible = true;
    }
  }
  return visible;
}

bool digits(std::string_view text, std::size_t begin, std::size_t count) {
  for (std::size_t index = begin; index < begin + count; ++index) {
    if (text[index] < '0' || text[index] > '9') return false;
  }
  return true;
}

unsigned number(std::string_view text, std::size_t begin, std::size_t count) {
  unsigned value = 0;
  for (std::size_t index = begin; index < begin + count; ++index) {
    value = value * 10U + static_cast<unsigned>(text[index] - '0');
  }
  return value;
}

bool leap_year(unsigned year) {
  return year % 4U == 0U && (year % 100U != 0U || year % 400U == 0U);
}

bool valid_utc_timestamp(std::string_view timestamp) {
  if (timestamp.size() != 20U || timestamp[4] != '-' ||
      timestamp[7] != '-' || timestamp[10] != 'T' ||
      timestamp[13] != ':' || timestamp[16] != ':' ||
      timestamp[19] != 'Z' || !digits(timestamp, 0, 4) ||
      !digits(timestamp, 5, 2) || !digits(timestamp, 8, 2) ||
      !digits(timestamp, 11, 2) || !digits(timestamp, 14, 2) ||
      !digits(timestamp, 17, 2)) {
    return false;
  }
  const unsigned year = number(timestamp, 0, 4);
  const unsigned month = number(timestamp, 5, 2);
  const unsigned day = number(timestamp, 8, 2);
  const unsigned hour = number(timestamp, 11, 2);
  const unsigned minute = number(timestamp, 14, 2);
  const unsigned second = number(timestamp, 17, 2);
  if (month < 1U || month > 12U || hour > 23U || minute > 59U ||
      second > 59U) {
    return false;
  }
  constexpr std::array<unsigned, 12> days{
      31U, 28U, 31U, 30U, 31U, 30U,
      31U, 31U, 30U, 31U, 30U, 31U};
  unsigned maximum = days[month - 1U];
  if (month == 2U && leap_year(year)) maximum = 29U;
  return day >= 1U && day <= maximum;
}

void append_field(std::string& record, std::string_view name,
                  std::string_view value) {
  record.append(name);
  record.push_back('=');
  record.append(std::to_string(value.size()));
  record.push_back(':');
  record.append(value);
  record.push_back('\n');
}

struct CanonicalBinding {
  std::string publisher_id;
  std::string key_id;
  std::string key_record_sha256;
};

}  // namespace

bool PolicyKeyTrustResult::trusted() const {
  return decision == PolicyKeyTrustDecision::Trusted && errors.empty() &&
         !canonical_record.empty();
}

PolicyKeyTrustResult evaluate_policy_key_trust(
    const PolicyKeyTrustRoot& root, const std::string& publisher_id,
    const PolicySigningKey& candidate, const std::string& evaluation_utc) {
  PolicyKeyTrustResult result;
  if (!valid_record_text(root.root_id)) {
    result.errors.emplace_back("trust-root ID must be visible single-line text");
  }
  if (root.version == 0U) {
    result.errors.emplace_back("trust-root version must be greater than zero");
  }
  if (!valid_utc_timestamp(root.expires_utc)) {
    result.errors.emplace_back(
        "trust-root expiry must use YYYY-MM-DDTHH:MM:SSZ UTC");
  }
  if (!valid_utc_timestamp(evaluation_utc)) {
    result.errors.emplace_back(
        "trust evaluation time must use YYYY-MM-DDTHH:MM:SSZ UTC");
  }
  if (root.trusted_keys.empty()) {
    result.errors.emplace_back("trust root requires at least one key binding");
  }
  if (!valid_record_text(publisher_id)) {
    result.errors.emplace_back(
        "candidate publisher ID must be visible single-line text");
  }

  const auto candidate_result = validate_policy_signing_key(candidate);
  for (const auto& error : candidate_result.errors) {
    result.errors.emplace_back("candidate signing key: " + error);
  }

  std::vector<CanonicalBinding> bindings;
  bindings.reserve(root.trusted_keys.size());
  for (const auto& binding : root.trusted_keys) {
    if (!valid_record_text(binding.publisher_id)) {
      result.errors.emplace_back(
          "trusted publisher ID must be visible single-line text");
      continue;
    }
    const auto key_result = validate_policy_signing_key(binding.signing_key);
    if (!key_result.fingerprint_verified()) {
      for (const auto& error : key_result.errors) {
        result.errors.emplace_back("trusted signing key: " + error);
      }
      continue;
    }
    const auto digest = sha256_hex(key_result.canonical_record);
    if (!digest) {
      result.errors.emplace_back(
          "trusted signing-key record exceeds the supported SHA-256 length");
      continue;
    }
    bindings.push_back(
        {binding.publisher_id, binding.signing_key.key_id, *digest});
  }

  std::sort(bindings.begin(), bindings.end(),
            [](const CanonicalBinding& left,
               const CanonicalBinding& right) {
              if (left.publisher_id != right.publisher_id) {
                return left.publisher_id < right.publisher_id;
              }
              return left.key_id < right.key_id;
            });
  for (std::size_t index = 1; index < bindings.size(); ++index) {
    if (bindings[index - 1].publisher_id == bindings[index].publisher_id &&
        bindings[index - 1].key_id == bindings[index].key_id) {
      result.errors.emplace_back(
          "trust root contains an ambiguous duplicate publisher and key ID");
    }
  }
  if (!result.errors.empty()) return result;

  if (evaluation_utc >= root.expires_utc) {
    result.decision = PolicyKeyTrustDecision::Expired;
  } else {
    const bool trusted = std::any_of(
        root.trusted_keys.begin(), root.trusted_keys.end(),
        [&](const TrustedPolicyKeyBinding& binding) {
          return binding.publisher_id == publisher_id &&
                 binding.signing_key.algorithm == candidate.algorithm &&
                 binding.signing_key.key_id == candidate.key_id &&
                 binding.signing_key.public_key_sha256 ==
                     candidate.public_key_sha256 &&
                 binding.signing_key.public_key_bytes ==
                     candidate.public_key_bytes;
        });
    result.decision = trusted ? PolicyKeyTrustDecision::Trusted
                              : PolicyKeyTrustDecision::NotTrusted;
  }

  const auto candidate_digest = sha256_hex(candidate_result.canonical_record);
  if (!candidate_digest) {
    result.decision = PolicyKeyTrustDecision::Invalid;
    result.errors.emplace_back(
        "candidate signing-key record exceeds the supported SHA-256 length");
    return result;
  }

  result.canonical_record = "formfactor-policy-key-trust-v1\n";
  append_field(result.canonical_record, "root-id", root.root_id);
  result.canonical_record.append("root-version=");
  result.canonical_record.append(std::to_string(root.version));
  result.canonical_record.push_back('\n');
  append_field(result.canonical_record, "root-expires-utc", root.expires_utc);
  append_field(result.canonical_record, "evaluated-at-utc", evaluation_utc);
  append_field(result.canonical_record, "candidate-publisher", publisher_id);
  append_field(result.canonical_record, "candidate-key-record-sha256",
               *candidate_digest);
  result.canonical_record.append("trusted-key-count=");
  result.canonical_record.append(std::to_string(bindings.size()));
  result.canonical_record.push_back('\n');
  for (const auto& binding : bindings) {
    append_field(result.canonical_record, "trusted-publisher",
                 binding.publisher_id);
    append_field(result.canonical_record, "trusted-key-id", binding.key_id);
    append_field(result.canonical_record, "trusted-key-record-sha256",
                 binding.key_record_sha256);
  }
  switch (result.decision) {
    case PolicyKeyTrustDecision::Trusted:
      result.canonical_record.append("decision=trusted\n");
      break;
    case PolicyKeyTrustDecision::NotTrusted:
      result.canonical_record.append("decision=not-trusted\n");
      break;
    case PolicyKeyTrustDecision::Expired:
      result.canonical_record.append("decision=expired\n");
      break;
    case PolicyKeyTrustDecision::Invalid:
      break;
  }
  return result;
}

bool PolicyKeyRootTransitionResult::sequence_valid() const {
  return decision == PolicyKeyRootTransitionDecision::Sequential &&
         errors.empty() && !canonical_record.empty();
}

PolicyKeyRootTransitionResult validate_policy_key_root_transition(
    const PolicyKeyTrustRoot& current_root,
    const PolicyKeyTrustRoot& candidate_root,
    const std::string& evaluation_utc) {
  PolicyKeyRootTransitionResult result;

  const auto validate_root =
      [&](const PolicyKeyTrustRoot& root) -> PolicyKeyTrustResult {
    if (root.trusted_keys.empty()) {
      return evaluate_policy_key_trust(
          root, std::string{}, PolicySigningKey{}, evaluation_utc);
    }
    const auto binding = std::min_element(
        root.trusted_keys.begin(), root.trusted_keys.end(),
        [](const TrustedPolicyKeyBinding& left,
           const TrustedPolicyKeyBinding& right) {
          if (left.publisher_id != right.publisher_id) {
            return left.publisher_id < right.publisher_id;
          }
          return left.signing_key.key_id < right.signing_key.key_id;
        });
    return evaluate_policy_key_trust(
        root, binding->publisher_id, binding->signing_key, evaluation_utc);
  };

  const auto current = validate_root(current_root);
  const auto candidate = validate_root(candidate_root);
  for (const auto& error : current.errors) {
    result.errors.emplace_back("current trust root: " + error);
  }
  for (const auto& error : candidate.errors) {
    result.errors.emplace_back("candidate trust root: " + error);
  }
  if (!result.errors.empty()) return result;

  if (current_root.version == std::numeric_limits<std::uint64_t>::max()) {
    result.errors.emplace_back(
        "current trust-root version cannot be incremented");
    return result;
  }

  if (current_root.root_id != candidate_root.root_id) {
    result.decision = PolicyKeyRootTransitionDecision::DifferentRoot;
  } else if (candidate_root.version <= current_root.version) {
    result.decision = PolicyKeyRootTransitionDecision::Rollback;
  } else if (candidate_root.version != current_root.version + 1U) {
    result.decision = PolicyKeyRootTransitionDecision::VersionGap;
  } else if (candidate.decision == PolicyKeyTrustDecision::Expired) {
    result.decision = PolicyKeyRootTransitionDecision::Expired;
  } else {
    result.decision = PolicyKeyRootTransitionDecision::Sequential;
  }

  const auto current_digest = sha256_hex(current.canonical_record);
  const auto candidate_digest = sha256_hex(candidate.canonical_record);
  if (!current_digest || !candidate_digest) {
    result.decision = PolicyKeyRootTransitionDecision::Invalid;
    result.errors.emplace_back(
        "trust-root transition evidence exceeds the supported SHA-256 length");
    return result;
  }

  result.canonical_record = "formfactor-policy-key-root-transition-v1\n";
  append_field(result.canonical_record, "root-id", current_root.root_id);
  result.canonical_record.append("current-version=");
  result.canonical_record.append(std::to_string(current_root.version));
  result.canonical_record.push_back('\n');
  result.canonical_record.append("candidate-version=");
  result.canonical_record.append(std::to_string(candidate_root.version));
  result.canonical_record.push_back('\n');
  append_field(result.canonical_record, "evaluated-at-utc", evaluation_utc);
  append_field(result.canonical_record, "current-root-record-sha256",
               *current_digest);
  append_field(result.canonical_record, "candidate-root-record-sha256",
               *candidate_digest);
  switch (result.decision) {
    case PolicyKeyRootTransitionDecision::Sequential:
      result.canonical_record.append("decision=sequential\n");
      break;
    case PolicyKeyRootTransitionDecision::Rollback:
      result.canonical_record.append("decision=rollback\n");
      break;
    case PolicyKeyRootTransitionDecision::VersionGap:
      result.canonical_record.append("decision=version-gap\n");
      break;
    case PolicyKeyRootTransitionDecision::DifferentRoot:
      result.canonical_record.append("decision=different-root\n");
      break;
    case PolicyKeyRootTransitionDecision::Expired:
      result.canonical_record.append("decision=expired\n");
      break;
    case PolicyKeyRootTransitionDecision::Invalid:
      break;
  }
  result.canonical_record.append("signature-verification=not-performed\n");
  result.canonical_record.append("persistence=not-performed\n");
  return result;
}

}  // namespace formfactor
