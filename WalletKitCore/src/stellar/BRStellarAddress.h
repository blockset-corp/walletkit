//
//  File.h
//  
//
//  Created by Carl Cherry on 2021-05-19.
//

#ifndef BRStellarAddress_h
#define BRStellarAddress_h

#include <stdio.h>
#include "BRStellarBase.h"
#include "support/BRKey.h"

typedef struct BRStellarAddressRecord *BRStellarAddress;

extern BRStellarAddress
stellarAddressCreate(BRKey * publicKey);

extern BRStellarAddress stellarAddressClone (BRStellarAddress address);

extern BRStellarAddress
stellarAddressCreateFromString(const char * stellarAddressString, bool strict);

extern char *
stellarAddressAsString(BRStellarAddress address);

extern void
stellarAddressFree(BRStellarAddress address);

extern size_t
stellarAddressHashValue (BRStellarAddress address);

/**
 * Compare 2 stellar addresses
 *
 * @param a1  first address
 * @param a2  second address
 *
 * @return 1 - if addresses are equal
 *         0 - if not equal
 */
extern int
stellarAddressEqual (BRStellarAddress a1, BRStellarAddress a2);

#endif /* BRStellarAddress_h */
