// swift-tools-version:5.1
//
import PackageDescription

let package = Package(
    name: "WalletKit",
    platforms: [
        .macOS(.v10_15),
        .iOS(.v11)
    ],
    products: [
        .library(
            name: "WalletKit",
            targets: ["WalletKit"]
        ),
    ],

    dependencies: [
        .package(path: "../WalletKitCore")
    ],

    targets: [
        .target(
            name: "WalletKit",
            dependencies: [
                "WalletKitCore"
            ],
            path: "WalletKit"
        ),

        .testTarget(
            name: "WalletKitTests",
            dependencies: [
                "WalletKit"
            ],
            path: "WalletKitTests"
        ),
    ]
)
