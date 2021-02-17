//
//  BRCryptoBaseP.h
//  BRCore
//
//  Created by Ed Gamble on 12/10/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoBaseP_h
#define BRCryptoBaseP_h

#include "BRCryptoBase.h"

#if !defined (MAX)
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#if !defined (MIN)
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

/// Private-ish
///
/// This is an implementation detail
///
#define free_const(pointer)    free((void*) (pointer))

#define assign_const(type, lval, rval) do { \
  type *__ptr = (type *) &(lval);           \
  *__ptr = (rval);                          \
} while (0)


#endif /* BRCryptoBaseP_h */
