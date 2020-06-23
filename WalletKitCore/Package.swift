// swift-tools-version:5.1
//
import PackageDescription

let package = Package(
    name: "WalletKitCore",
    products: [
        .library(
            name: "WalletKitCore",
            targets: ["WalletKitCore"]
        ),
        
        .executable(
            name: "WalletKitCoreExplore",
            targets: ["WalletKitCoreExplore"]
        ),


        .executable(
            name: "WalletKitCorePerf",
            targets: ["WalletKitCorePerf"]
        ),
    ],
    dependencies: [],
    targets: [
        // MARK: - Core Targets

        //
        // We want to compile all Core sources with various warnings enabled.  This is performed
        // in a Package.swift with a `cSettings.unsafeFlags` declaration.  However, a sub-package
        // dependency is forbidden from having a unsafeFlags declaration.  Thus walletkit-swift
        // depending on walletkit-core will fail during package validation.  We work around this
        // by ensuring that this top-level `.target` does not have an unsafeFlags declaration but
        // that subtargets do have our desired unsafeFlags.
        //
        // In order to accomplish the above this `WalletKitCore` target will depend on
        // `WalletKitCoreSafe`, where ALL the sources will be, BUT we need at least one source
        // file to remain in `WalletKitCore`.  We'll create one.
        //
        .target(
            name: "WalletKitCore",
            dependencies: [
                "WalletKitCoreSafe",
                "WalletKitSQLite",
                "WalletKitEd25519",
                "WalletKitHederaProto",
                "WalletKitBlake2"
            ],
            path: ".",
            sources: ["src/version"],               // Holds BRCryptoVersion.c only
            publicHeadersPath: "include",           // Export all public includes
            linkerSettings: [
                .linkedLibrary("resolv"),
                .linkedLibrary("pthread"),
                .linkedLibrary("bsd", .when(platforms: [.linux])),
            ]
        ),

        .target(
            name: "WalletKitCoreSafe",
            dependencies: [
            ],
            path: "src",
            exclude: [
                "hedera/proto",      // See target: WalletKitHederaProto
                "version"
            ],
            publicHeadersPath: "version",   // A directory WITHOUT headers
            cSettings: [
                .headerSearchPath("../include"),           // BRCrypto
                .headerSearchPath("."),
                .headerSearchPath("../vendor"),
                .headerSearchPath("../vendor/secp256k1"),  // To compile vendor/secp256k1/secp256k1.c
                .unsafeFlags([
                    // Enable warning flags
                    "-Wall",
                    "-Wconversion",
                    "-Wsign-conversion",
                    "-Wparentheses",
                    "-Wswitch",
                    // Disable warning flags, if appropriate
                    "-Wno-implicit-int-conversion",
                    // "-Wno-sign-conversion",
                    "-Wno-missing-braces"
                ])
            ]
        ),

        // Custom compilation flags for SQLite - to silence warnings
        .target(
            name: "WalletKitSQLite",
            dependencies: [],
            path: "vendor/sqlite3",
            sources: ["sqlite3.c"],
            publicHeadersPath: "include",
            cSettings: [
                .unsafeFlags([
                    "-Xclang", "-analyzer-disable-all-checks",
                    "-D_HAVE_SQLITE_CONFIG_H=1",
                    "-Wno-ambiguous-macro",
                    "-Wno-shorten-64-to-32",
                    "-Wno-unreachable-code",
                    "-Wno-#warnings"
                ])
            ]
        ),

        // Custom compilation flags for ed15519 - to silence warnings
        .target(
            name: "WalletKitEd25519",
            dependencies: [],
            path: "vendor/ed25519",
            exclude: [],
            publicHeadersPath: nil,
            cSettings: [
                .unsafeFlags([
                    "-Xclang", "-analyzer-disable-all-checks"
                ])
            ]
        ),

        // Custom compilation flags for hedera/proto - to silence warnings
        .target(
            name: "WalletKitHederaProto",
            dependencies: [],
            path: "src/hedera/proto",
            publicHeadersPath: nil,
            cSettings: [
                .unsafeFlags([
                    "-Xclang", "-analyzer-disable-all-checks",
                    "-Wno-shorten-64-to-32",
                ])
            ]
        ),
        
        // Custom compilation flags for blake2 - to silence warnings
        .target(
            name: "WalletKitBlake2",
            dependencies: [],
            path: "vendor/blake2",
            exclude: [],
            publicHeadersPath: nil,
            cSettings: [
                .unsafeFlags([
                    "-Xclang", "-analyzer-disable-all-checks"
                ])
            ]
        ),

        // MARK: - Core Misc Targets

        .target (
            name: "WalletKitCoreExplore",
            dependencies: ["WalletKitCore"],
            path: "WalletKitCoreExplore",
            cSettings: [
                .headerSearchPath("../include"),
                .headerSearchPath("../src"),
            ]
        ),

        .target (
            name: "WalletKitCorePerf",
            dependencies: ["WalletKitCore", "WalletKitCoreSupportTests"],
            path: "WalletKitCorePerf",
            cSettings: [
                .headerSearchPath("../include"),
                .headerSearchPath("../src"),
            ]
        ),

        // MARK: - Core Test Targets

        .target(
            name: "WalletKitCoreSupportTests",
            dependencies: ["WalletKitCore"],
            path: "WalletKitCoreTests/test",
            publicHeadersPath: "include",
            cSettings: [
                .define("BITCOIN_TEST_NO_MAIN"),
                .headerSearchPath("../../include"),
                .headerSearchPath("../../src"),
            ]
        ),

        .testTarget(
            name: "WalletKitCoreTests",
            dependencies: [
                "WalletKitCoreSupportTests"
            ],
            path: "WalletKitCoreTests",
            exclude: [
                "test"
            ],
            cSettings: [
                .headerSearchPath("../src"),
            ],
            linkerSettings: [
                .linkedLibrary("pthread"),
                .linkedLibrary("bsd", .when(platforms: [.linux])),
            ]
        ),
    ]
)
