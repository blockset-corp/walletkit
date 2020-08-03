# WalletKitKotlin
An experimental [Kotlin Multiplatform](https://kotl.in/multiplatform) [WalletKit](https://blockset.com/) implementation.

## Libraries

For native targets, WalletKitCore is used directly and JVM targets use the corenative JNA modules.

This project makes use of the following libraries across all targets:

- [kotlinx.serialization](https://github.com/Kotlin/kotlinx.serialization) for multiplatform JSON serialization.
- [ktor](https://github.com/ktorio/ktor/): a multiplatform HTTP client wrapper supporting curl(macos), OkHttp(jvm), URLSession(ios), etc.

## Structure

[src/commonMain/kotlin](src/commonMain/kotlin): Contains common Kotlin that does not use any platform specific APIs.
Code prefixed with `expect` is replaced by code prefixed with `actual` from platform specific source directories (`src/<platform>Main/kotlin`).

[src/commonTest/kotlin](src/commonTest/kotlin): Contains common Kotlin test sources that run on every supported target.

[src/jvmMain/kotlin](src/jvmMain/kotlin): Contains JVM `actual`s and other specific APIs using the corenative module.
This code can be used directly in Kotlin/JVM code or Java.

[src/darwinMain/kotlin](src/darwinMain/kotlin): Contains Apple common `actual`s and other platform specific APIs using the WalletKitCore C API.
This code can be used directly in Kotlin/Native code or Obj-C/Swift.

## Build and Test

Building any target will automatically compile WalletKitCore for the associated platform.

* Build and test project: `./gradlew build`
* Run tests
  * `./gradlew allTest`
  * `./gradlew macosTest`
  * `./gradlew jvmTest`
  * `./gradlew iosSimTest`
  * `./gradlew testDebugUnitTest` Android Unit Tests
* Build all outputs: `./gradlew assemble`
* Build iOS Frameworks
  * `./gradlew linkDebugFrameworkIosSim linkReleaseFrameworkIosSim`
  * `./gradlew linkDebugFrameworkIosArm64 linkReleaseFrameworkIosArm64`
* Generate Docs: `./gradlew dokka`
* Clean build dirs: `./gradlew clean`

Running any gradle command will download all necessary tooling, the first run takes some time.

## Demos

The [demo](demo) module performs a full sync and reports the balance of a wallet, it currently supports JVM and Macos.

* Run jvm: `./gradlew demo:runJar`
* Run macos: `./gradlew demo:runDebugExecutableMacos`


[SwiftDemo](SwiftDemo) is a minimal copy of the existing Swift demo using the WalletKitKotlin as a Framework.
Open the SwiftDemo project in Xcode and click run, WalletKitKotlin will be built automatically.

## Publishing

TODO: Setup and document publishing steps for various repositories.

## Notes

- While possible, the current iteration of the common Kotlin API is not binary compatible with the existing JVM or Swift libraries.
