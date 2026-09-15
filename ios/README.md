# FormFactor iOS Prototype

This branch is separate from `main` so iPhone work cannot break the normal Windows and Linux playable releases.

## Touch controls

1. Tap a component in the left panel to choose it.
2. Tap empty board space to place it.
3. Tap one placed component, then tap another, to connect them.
4. Tap the green check button at the top to test the circuit.
5. Tap the red X button at the top to clear the board.
6. A first passing circuit needs a Power part connected to a valid load such as a resistor, LED, chip, or connector.

## What GitHub builds automatically

The `ios-prototype` workflow uses a GitHub macOS runner to compile two versions:

- an iPhone Simulator app used to prove the app launches as an iOS bundle;
- an unsigned arm64 iPhone app used to prove the code also compiles for real iPhone hardware.

## Why the unsigned build cannot be installed by tapping a ZIP

Apple requires native iPhone apps to be signed with an Apple Developer identity and a provisioning profile. An unsigned `.app` or `.ipa` can be built and downloaded, but a normal iPhone will reject it when installation is attempted.

For an actual phone download, the next clean step is **TestFlight**. That requires an Apple Developer account, a distribution certificate/profile, and App Store Connect access. Once those signing pieces are connected to the build pipeline, this branch can produce a signed IPA and upload each tested iOS prototype to TestFlight while `main` continues to publish Windows and Linux separately.
