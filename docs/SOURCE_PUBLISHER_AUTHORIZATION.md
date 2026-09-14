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
