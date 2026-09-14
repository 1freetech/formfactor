# Source publisher-origin authorization

CR-014 adds a deterministic, fail-closed policy evaluation primitive for source provenance. It answers one narrow question: does a structurally valid source claim carry the exact publisher ID named by a supplied policy, and does its URL use one of that policy's exact canonical HTTPS origins?

## Acceptance contract

1. Give the claim a visible, single-line publisher ID.
2. Give the policy a visible, single-line publisher ID.
3. List one or more exact origins accepted by FormFactor's conservative HTTPS profile, such as `https://publisher.invalid`.
4. Authorize only when the publisher IDs and extracted origin match exactly.
5. Treat a valid mismatch as `NotAuthorized`.
6. Treat malformed claim or policy input as `Invalid` and emit no canonical record.
7. Sort authorized origins before writing the versioned, length-prefixed replay record.

Paths and queries may appear in a source URL, but never in a policy origin. Subdomains and explicit ports are distinct origins; no suffix, prefix, redirect, DNS, or certificate inference is performed. Tests use the reserved `.invalid` namespace and contain no manufacturer claims.

## Explicit limits

The caller supplies the authorization policy. The core does not yet authenticate that policy, fetch a URL, validate TLS or DNS, follow redirects, check an artifact signature, establish publisher ownership, or connect this decision to catalogue export. `Authorized` therefore means only “matches this validated input policy.” It does not make a source authoritative and cannot establish electrical correctness, safety, compliance, or manufacturability.


## Policy artifact provenance

CR-015 requires the supplied policy to name a visible, revisioned HTTPS source and carry a lowercase 64-hex SHA-256 digest for the exact policy artifact. The canonical record advances to `formfactor-source-authorization-v2` and preserves the source title, URL, revision, and digest. The digest syntax represents the 256-bit SHA-256 output defined by [NIST FIPS 180-4](https://csrc.nist.gov/pubs/fips/180-4/upd1/final).

This is an integrity binding supplied by the caller. FormFactor does not yet retrieve the policy artifact, calculate the digest, verify a signature, authenticate the policy publisher, or prove that the policy content authorizes any origin.


## Exact policy-byte digest verification

CR-016 computes SHA-256 over the exact caller-supplied policy bytes using the NIST FIPS 180-4 algorithm and compares the lowercase digest before authorization. Missing bytes remain unknown and fail closed. An explicitly supplied empty artifact remains distinct and is verified against the standard empty-message digest. Binary bytes, including zero bytes, are hashed without text conversion. Valid records advance to `formfactor-source-authorization-v3` and preserve the verified byte count without embedding the policy content.

The implementation is checked against NIST's empty-message, `abc`, and multi-block test vectors. Digest equality establishes byte integrity only; it does not verify a digital signature, publisher identity, policy authority, or network transport.


## Ed25519 signing-key fingerprint validation

CR-017 validates the structural identity of a caller-supplied policy signing key before any future signature gate can use it. The algorithm is exactly Ed25519, the public key is exactly 32 octets as specified by [RFC 8032](https://www.rfc-editor.org/rfc/rfc8032.html), and its lowercase SHA-256 fingerprint must match those exact bytes. Missing key bytes, 31-byte and 33-byte boundaries, malformed fingerprints, unsupported algorithms, and altered bytes fail closed without a record. Valid inputs produce deterministic `formfactor-policy-signing-key-v1` records that preserve the key ID, algorithm, byte count, and verified fingerprint without embedding the key bytes.

Fingerprint equality establishes only that the supplied identifier and supplied key bytes are internally consistent. FormFactor does not yet verify an Ed25519 signature, establish key ownership or publisher authority, distribute trust anchors, check revocation or rotation, or connect this primitive to publisher authorization or catalogue export.


## Detached Ed25519 signature evidence

CR-018 validates the complete byte envelope required by a later detached-signature verifier. A CR-017-valid Ed25519 key, explicitly present signed bytes, and exactly 64 signature octets are required; RFC 8032 defines the Ed25519 signature size. Missing message or signature bytes and 63-byte or 65-byte signatures fail closed without a record. An explicitly present empty message remains distinct from missing data. Valid inputs produce deterministic `formfactor-detached-signature-evidence-v1` records with SHA-256 bindings for the exact signed bytes and signature, while raw content remains outside the record.

The record states `cryptographic-verification=not-performed`. A different but correctly sized signature is therefore structurally complete and produces different replay evidence; it is not accepted as authentic. FormFactor does not yet evaluate the Ed25519 verification equation, establish key ownership or authority, handle revocation or rotation, or connect this evidence to publisher authorization or catalogue export.
