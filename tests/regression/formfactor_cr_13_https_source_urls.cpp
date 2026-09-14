#include "formfactor/component.hpp"

#include <array>
#include <cassert>
#include <string>
#include <vector>

namespace {
formfactor::Component component() {
  return {"Fixture Maker", "FF-013", "SYNTHETIC-2", 2, 1.0, 1.0, 1.0, true,
          formfactor::AccuracyTier::Verified,
          {{"Synthetic source", "https://fixture.invalid/ff-013", "Rev A"}}};
}
}  // namespace

int main() {
  const auto valid = component();
  assert(formfactor::validate(valid).export_allowed());
  assert(formfactor::valid_source_url("https://x"));
  assert(formfactor::valid_source_url(
      "https://fixture.invalid:443/ff-013?revision=a"));

  const std::array<const char*, 12> invalid_urls{
      "", "http://fixture.invalid/ff-013", "ftp://fixture.invalid/ff-013",
      "HTTPS://fixture.invalid/ff-013", "https:///ff-013",
      "https://Fixture.invalid/ff-013", "https://user@fixture.invalid/ff-013",
      "https://fixture.invalid/ff-013#page-2",
      "https://fixture.invalid/ff 013", "https://fixture.invalid:abc/ff-013",
      "https://fixture..invalid/ff-013", "https://-fixture.invalid/ff-013"};
  for (const char* url : invalid_urls) {
    auto invalid = valid;
    invalid.sources.front().url = url;
    const auto first = formfactor::validate(invalid);
    const auto replay = formfactor::validate(invalid);
    assert(!first.export_allowed());
    assert(first.effective_tier == formfactor::AccuracyTier::Partial);
    assert(first.errors ==
           std::vector<std::string>{
               "verified components require a revisioned authoritative HTTPS source"});
    assert(first.errors == replay.errors);
    assert(first.effective_tier == replay.effective_tier);
  }
}
