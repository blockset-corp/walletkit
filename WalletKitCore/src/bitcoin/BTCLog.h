//
//  WKBTCLog.h
//
//  Created by Bryan Goring on 11/10/21.
//  Copyright (c) 2021 breadwallet LLC.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef BTCLog_h
#define BTCLog_h

// Declaration of the space of this module's logs
#include "support/BRLog.h"
LOG_DECLARE_MODULE(BTC);
LOG_DECLARE_SUBMODULE(BTC,PEER);
LOG_DECLARE_SUBMODULE(BTC,BWM);
LOG_DECLARE_SUBMODULE(BTC,BPM);

// Specialize general purpose LOG() for reporting on peers; antecedent: peer_log
#define _va_first(first, ...) first
#define _va_rest(first, ...) __VA_ARGS__
#define LOG_PEER(lvl, peer, ...) LOG (lvl, BTC_PEER, "%s:%"PRIu16" " _va_first(__VA_ARGS__, NULL) "\n", btcPeerHost(peer),\
        (peer)->port, _va_rest(__VA_ARGS__, NULL))

#endif /* BTCLog_h */
