# WalletKitKotlin
An experimental [Kotlin Multiplatform](https://kotl.in/multiplatform) [WalletKit](https://blockset.com/) implementation.

## Libraries

For native targets, WalletKitCore is used directly and JVM targets use the corenative JNA modules.

This project makes use of the following libraries across all targets:

- [kotlinx.serialization](https://github.com/Kotlin/kotlinx.serialization) for multiplatform serialization.
- [ktor](https://github.com/ktorio/ktor/) for a multiplatform Http client.

## Structure

[src/commonMain/kotlin](src/commonMain/kotlin): Contains common Kotlin that does not use any platform specific APIs.
Code with the `expect` keyword replaces code prefixed with `actual` in a platform specific source directory (`src/<platform>Main/kotlin`).
Mutliplatform Kotlin applications can use this API to target any supported platforms.

[src/commonTest/kotlin](src/commonTest/kotlin): Contains common Kotlin test sources that run on every supported target.

[src/jvmMain/kotlin](src/jvmMain/kotlin): Contains JVM `actual`s and other specific APIs using the corenative module.
This code can be used directly in Kotlin/JVM code or Java.

[src/darwinMain/kotlin](src/darwinMain/kotlin): Contains Apple common `actual`s and other platform specific APIs using the WalletKitCore C API.
This code can be used directly in Kotlin/Native code or Obj-C/Swift.

## Build and Test

Building any target will automatically compile WalletKitCore for the associated platform.

* Run tests
  * `./gradlew allTest`
  * `./gradlew macosTest`
  * `./gradlew jvmTest`
  * `./gradlew iosSimTest`
* Build all outputs: `./gradlew assemble`
* Generate Docs: `./gradlew dokka`
* Clean build dirs: `./gradlew clean`

Running any gradle command will download all necessary tooling, the first run takes some time.

## Demo

The current demo will perform a full sync and report the balance of a wallet.
* Run macos): `./gradlew demo:runDebugExecutableMacos`
* Run demo (jvm): `./gradlew demo:runJar`

## Publishing

TODO: Setup and document publishing steps for various repositories.

## Notes

- While possible, the current iteration of the common Kotlin API is not binary compatible with the existing JVM or Swift libraries.
- Android can be added atop the JVM implementation but requires some build configuration to support development environments without the Android SDK installed.
