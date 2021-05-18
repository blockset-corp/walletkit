//
//  WKCoder.swift
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

public protocol Coder {
    func encode (data: Data) -> String?
    func decode (string: String) -> Data?
}

public final class CoreCoder: Coder {

    public static var hex: CoreCoder {
        return CoreCoder (core: wkCoderCreate (WK_CODER_HEX)!)
    }

    public static var base58: CoreCoder {
        return CoreCoder (core: wkCoderCreate (WK_CODER_BASE58)!)
    }

    public static var base58check: CoreCoder {
        return CoreCoder (core: wkCoderCreate (WK_CODER_BASE58CHECK)!)
    }

    public static var base58ripple: CoreCoder {
        return CoreCoder (core: wkCoderCreate (WK_CODER_BASE58RIPPLE)!)
    }

    // The Core representation
    internal let core: WKCoder

    deinit { wkCoderGive (core) }

    internal init (core: WKCoder) {
        self.core = core
    }

    public func encode (data source: Data) -> String? {
        return source.withUnsafeBytes { (sourceBytes: UnsafeRawBufferPointer) -> String? in
            let sourceAddr  = sourceBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            let sourceCount = sourceBytes.count

            let targetCount = wkCoderEncodeLength(self.core, sourceAddr, sourceCount)
            guard targetCount != 0 else { return nil }

            var target = Data (count: targetCount)
            return target.withUnsafeMutableBytes { (targetBytes: UnsafeMutableRawBufferPointer) -> String? in
                let targetAddr  = targetBytes.baseAddress?.assumingMemoryBound(to: Int8.self)

                let result = wkCoderEncode (self.core, targetAddr, targetCount, sourceAddr, sourceCount)
                
                return result == WK_TRUE ? String (cString: targetAddr!) : nil
            }
        }
    }

    public func decode (string source: String) -> Data? {
        return source.withCString { (sourceAddr: UnsafePointer<Int8>) -> Data? in
            let targetCount = wkCoderDecodeLength(self.core, sourceAddr)
            guard targetCount != 0 else { return nil }

            var result = WK_FALSE
            var target = Data (count: targetCount)
            target.withUnsafeMutableBytes { (targetBytes: UnsafeMutableRawBufferPointer) -> Void in
                let targetAddr  = targetBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)

                result = wkCoderDecode (self.core, targetAddr, targetCount, sourceAddr)
            }

            return result == WK_TRUE ? target : nil
        }
    }
}

