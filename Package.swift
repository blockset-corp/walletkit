// swift-tools-version:5.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "WalletKitCore",
    products: [
        .library(
            name: "WalletKitCore",
            targets: ["WalletKitCore"]
        ),
        
        .executable(
            name: "WalletKitExplore",
            targets: ["WalletKitExplore"]
        ),
        
        
        .executable(
            name: "WalletKitPerf",
            targets: ["WalletKitPerf"]
        ),
    ],
    dependencies: [],
    targets: [
        .target(
            name: "WalletKitCore",
            dependencies: [
                "WalletKitSQLite",
                "WalletKitEd25519",
                "WalletKitHederaProto",
            ],
            path: "WalletKitCore",
            exclude: [
                "vendor",           // See target: WalletKitSQLite
                "hedera/proto"      // See target: WalletKitHederaProto
            ],
            publicHeadersPath: "include",
            cSettings: [
                .headerSearchPath("include"),
                .headerSearchPath("."),
                .headerSearchPath("./support"),
                .headerSearchPath("vendor/secp256k1"),
            ],
            linkerSettings: [
                .linkedLibrary("resolv"),
                .linkedLibrary("pthread"),
                .linkedLibrary("bsd", .when(platforms: [.linux])),
            ]
        ),

         // Custom compilation flags for SQLite - to silence warnings
        .target(
            name: "WalletKitSQLite",
            dependencies: [],
            path: "WalletKitCore/vendor/sqlite3",
            exclude: [
                "shell.c",
            ],
            publicHeadersPath: nil,
            cSettings: [
                .unsafeFlags([
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
            path: "WalletKitCore/vendor/ed25519",
            exclude: [],
            publicHeadersPath: nil,
            cSettings: [
                .unsafeFlags([])
            ]
        ),


        // Custom compilation flags for hedera/proto - to silence warnings
        .target(
            name: "WalletKitHederaProto",
            dependencies: [],
            path: "WalletKitCore/hedera/proto",
            publicHeadersPath: nil,
            cSettings: [
                .unsafeFlags([
                    "-Wno-shorten-64-to-32",
                ])
            ]
        ),

        .target (
            name: "WalletKitExplore",
            dependencies: ["WalletKitCore"],
            path: "WalletKitExplore",
            cSettings: [
                .headerSearchPath("../WalletKitCore"),
                .headerSearchPath("../WalletKitCore/support"),
                .headerSearchPath("../WalletKitCore/bitcoin"),
            ]
        ),

        .target (
            name: "WalletKitPerf",
            dependencies: ["WalletKitCore", "WalletKitSupportTests"],
            path: "WalletKitPerf",
            cSettings: [
                .headerSearchPath("../WalletKitCore"),
                .headerSearchPath("../WalletKitCore/support"),
                .headerSearchPath("../WalletKitCore/bitcoin"),
                .headerSearchPath("../WalletKitCoreTests/test"),
            ]
        ),
 
        .target(
            name: "WalletKitSupportTests",
            dependencies: ["WalletKitCore"],
            path: "WalletKitCoreTests/test",
            publicHeadersPath: "include",
            cSettings: [
                .define("BITCOIN_TEST_NO_MAIN"),
                .headerSearchPath("../../WalletKitCore"),
                .headerSearchPath("../../WalletKitCore/support"),
                .headerSearchPath("../../WalletKitCore/bitcoin")
            ]
        ),

        .testTarget(
            name: "WalletKitCoreTests",
            dependencies: [
                "WalletKitSupportTests"
            ],
            path: "WalletKitCoreTests",
            exclude: [
                "test"
            ],
            cSettings: [
                .headerSearchPath("../WalletKitCore"),
                .headerSearchPath("../WalletKitCore/support"),
                .headerSearchPath("../WalletKitCore/bitcoin")
            ],
            linkerSettings: [
                .linkedLibrary("pthread"),
                .linkedLibrary("bsd", .when(platforms: [.linux])),
            ]
        ),

    ]
)
