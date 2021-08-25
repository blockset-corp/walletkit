//
//  BRAvalancheAccount.h
//  
//
//  Created by Amit on 10/06/2021.
//

#ifndef BRAvalancheAccount_h
#define BRAvalancheAccount_h

#include <stdio.h>
#include "support/BRKey.h"
#include "support/BRInt.h"
#include "support/BRCrypto.h"
#include "support/BRBech32.h"
#include "BRAvalancheAddress.h"
#include "support/BRBIP32Sequence.h"
#include "ethereum/base/BREthereumAddress.h"


#ifdef __cplusplus
extern "C" {
#endif

// prefix (3 bytes) + pkh (20 bytes)



typedef struct BRAvalancheAccountRecord *BRAvalancheAccount;

struct BRAvalancheAccountRecord {
    BRAvalancheXAddress xaddress;
    BREthereumAddress caddress;
};

extern BRAvalancheAccount  /* caller must free - using "free" function */
avalancheAccountCreateWithSeed (UInt512 seed);

extern BRKey
deriveAvalanchePrivateKeyFromSeed (UInt512 seed);


#ifdef __cplusplus
}
#endif

#endif /* BRAvalancheAccount+h */

