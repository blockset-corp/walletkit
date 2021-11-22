//
//  BREthereumLog.h
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

#ifndef BREthLog_h
#define BREthLog_h

// Declaration of the space of this module's logs
#include "support/BRLog.h"
LOG_DECLARE_MODULE(ETH);
LOG_DECLARE_SUBMODULE(ETH,INIT);
LOG_DECLARE_SUBMODULE(ETH,BCS);
LOG_DECLARE_SUBMODULE(ETH,MEM);
LOG_DECLARE_SUBMODULE(ETH,SHOW);
LOG_DECLARE_SUBMODULE(ETH,LES);

#endif /* BREthLog_h */
