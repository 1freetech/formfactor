#include "formfactor/policy_signature.hpp"

#include "formfactor/hash.hpp"

#include <openssl/crypto.h>
#include <openssl/evp.h>

#include <memory>
#include <string_view>

namespace formfactor {
namespace {

void append_field(std::string& record, std::string_view name,
                  std::string_view value) {
  record.append(name);
  record.push_back('=');
  record.append(std::to_string(value.size()));
  record.push_back(':');
  record.append(value);
  record.push_back('\n');
}

}  // namespace

bool DetachedSignatureVerificationResult::verified() const {
  return decision == DetachedSignatureVerificationDecision::Verified &&
         errors.empty() && !canonical_record.empty();
}

DetachedSignatureVerificationResult verify_detached_ed25519_signature(
    const DetachedPolicySignatureEvidence& evidence) {
  DetachedSignatureVerificationResult result;
  const auto envelope =
      validate_detached_policy_signature_evidence(evidence);
  if (!envelope.evidence_complete()) {
    result.errors = envelope.errors;
    return result;
  }

  result.backend_name = "OpenSSL EVP Ed25519";
  result.backend_version = OpenSSL_version(OPENSSL_VERSION);

  using PublicKey =
      std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
  using VerifyContext =
      std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

  const auto* public_key = reinterpret_cast<const unsigned char*>(
      evidence.signing_key.public_key_bytes->data());
  PublicKey key(EVP_PKEY_new_raw_public_key(
                    EVP_PKEY_ED25519, nullptr, public_key,
                    evidence.signing_key.public_key_bytes->size()),
                EVP_PKEY_free);
  if (!key) {
    result.decision = DetachedSignatureVerificationDecision::BackendError;
    result.errors.emplace_back(
        "OpenSSL could not construct the Ed25519 public key");
    return result;
  }

  VerifyContext context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
  if (!context ||
      EVP_DigestVerifyInit(context.get(), nullptr, nullptr, nullptr,
                           key.get()) != 1) {
    result.decision = DetachedSignatureVerificationDecision::BackendError;
    result.errors.emplace_back(
        "OpenSSL could not initialize one-shot Ed25519 verification");
    return result;
  }

  const auto* signature = reinterpret_cast<const unsigned char*>(
      evidence.signature_bytes->data());
  const auto* message = reinterpret_cast<const unsigned char*>(
      evidence.signed_bytes->data());
  const int status =
      EVP_DigestVerify(context.get(), signature,
                       evidence.signature_bytes->size(), message,
                       evidence.signed_bytes->size());

  if (status == 1) {
    result.decision = DetachedSignatureVerificationDecision::Verified;
  } else if (status == 0) {
    result.decision = DetachedSignatureVerificationDecision::Rejected;
  } else {
    result.decision = DetachedSignatureVerificationDecision::BackendError;
    result.errors.emplace_back(
        "OpenSSL reported an Ed25519 verification backend error");
    return result;
  }

  const auto evidence_digest = sha256_hex(envelope.canonical_record);
  if (!evidence_digest) {
    result.decision = DetachedSignatureVerificationDecision::BackendError;
    result.errors.emplace_back(
        "signature evidence record exceeds the supported SHA-256 length");
    return result;
  }

  result.canonical_record =
      "formfactor-detached-signature-verification-v1\n";
  append_field(result.canonical_record, "backend", result.backend_name);
  append_field(result.canonical_record, "backend-version",
               result.backend_version);
  append_field(result.canonical_record, "evidence-sha256",
               *evidence_digest);
  result.canonical_record.append(
      result.decision == DetachedSignatureVerificationDecision::Verified
          ? "decision=verified\n"
          : "decision=rejected\n");
  return result;
}

}  // namespace formfactor
