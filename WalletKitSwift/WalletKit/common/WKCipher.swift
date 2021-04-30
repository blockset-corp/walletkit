//
//  WKCipher.swift
//  WalletKit
//
//  Created by Ed Gamble on 7/18/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import Foundation
import WalletKitCore

public protocol Cipher {
    func encrypt (data: Data) -> Data?
    func decrypt (data: Data) -> Data?
}

public final class CoreCipher: Cipher {
    public static func aes_ecb(key: Data) -> CoreCipher {
        return key.withUnsafeBytes { (keyBytes: UnsafeRawBufferPointer) -> CoreCipher in
            let keyAddr = keyBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            let core = wkCipherCreateForAESECB(keyAddr, keyBytes.count)!
            return CoreCipher (core: core)
        }
    }

    public static func chacha20_poly1305(key: Key, nonce12: Data, ad: Data) -> CoreCipher {
        return nonce12.withUnsafeBytes { (nonce12Bytes: UnsafeRawBufferPointer) -> CoreCipher in
            let nonce12Addr  = nonce12Bytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            return ad.withUnsafeBytes { (adBytes: UnsafeRawBufferPointer) -> CoreCipher in
                let adAddr  = adBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
                let core = wkCipherCreateForChacha20Poly1305(key.core,
                                                                 nonce12Addr, nonce12Bytes.count,
                                                                 adAddr, adBytes.count)!
                return CoreCipher (core: core)
            }
        }
    }

    public static func pigeon(privKey: Key, pubKey: Key, nonce12: Data) -> CoreCipher {
        return nonce12.withUnsafeBytes { (nonce12Bytes: UnsafeRawBufferPointer) -> CoreCipher in
            let nonce12Addr  = nonce12Bytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            let core = wkCipherCreateForPigeon(privKey.core,
                                                   pubKey.core,
                                                   nonce12Addr, nonce12Bytes.count)!
            return CoreCipher (core: core)
        }
    }

    // The Core representation
    internal let core: WKCipher

    deinit { wkCipherGive (core) }

    internal init (core: WKCipher) {
        self.core = core
    }

    public func encrypt (data source: Data) -> Data? {
        return source.withUnsafeBytes { (sourceBytes: UnsafeRawBufferPointer) -> Data? in
            let sourceAddr  = sourceBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            let sourceCount = sourceBytes.count

            let targetCount = wkCipherEncryptLength(self.core, sourceAddr, sourceCount)
            guard targetCount != 0 else { return nil }

            var result = WK_FALSE
            var target = Data (count: targetCount)
            target.withUnsafeMutableBytes { (targetBytes: UnsafeMutableRawBufferPointer) -> Void in
                let targetAddr  = targetBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)

                result = wkCipherEncrypt (self.core, targetAddr, targetCount, sourceAddr, sourceCount)
            }

            return result == WK_TRUE ? target : nil
        }
    }

    public func decrypt (data source: Data) -> Data? {
        return source.withUnsafeBytes { (sourceBytes: UnsafeRawBufferPointer) -> Data? in
            let sourceAddr  = sourceBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            let sourceCount = sourceBytes.count

            let targetCount = wkCipherDecryptLength(self.core, sourceAddr, sourceCount)
            guard targetCount != 0 else { return nil }

            var result = WK_FALSE
            var target = Data (count: targetCount)
            target.withUnsafeMutableBytes { (targetBytes: UnsafeMutableRawBufferPointer) -> Void in
                let targetAddr  = targetBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)

                result = wkCipherDecrypt (self.core, targetAddr, targetCount, sourceAddr, sourceCount)
            }

            return result == WK_TRUE ? target : nil
        }
    }
}
