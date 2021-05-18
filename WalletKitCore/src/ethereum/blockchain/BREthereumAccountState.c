//
//  BREthereumAccountState.c
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include <assert.h>
#include "BREthereumAccountState.h"

extern BREthereumAccountState
ethAccountStateCreateEmpty (void) {
    return ETHEREUM_EMPTY_ACCOUNT_STATE_INIT;
}

extern uint64_t
ethAccountStateGetNonce (BREthereumAccountState state) {
    return state.nonce;
}

extern BREthereumEther
ethAccountStateGetBalance (BREthereumAccountState state) {
    return state.balance;
}

extern BREthereumHash
ethAccountStateGetStorageRoot (BREthereumAccountState state) {
    return state.storageRoot;
}

extern BREthereumHash
ethAccountStateGetCodeHash (BREthereumAccountState state) {
    return state.codeHash;
}

extern BREthereumBoolean
ethAccountStateEqual (BREthereumAccountState s1,
                   BREthereumAccountState s2) {
    return AS_ETHEREUM_BOOLEAN(s1.nonce == s2.nonce &&
                               ETHEREUM_BOOLEAN_IS_TRUE(ethEtherIsEQ(s1.balance, s2.balance)));
}

extern BREthereumAccountState
ethAccountStateCreate (uint64_t nonce,
                    BREthereumEther balance,
                    BREthereumHash storageRoot,
                    BREthereumHash codeHash) {

    BREthereumAccountState state;

    state.nonce = nonce;
    state.balance = balance;
    state.storageRoot = storageRoot;
    state.codeHash = codeHash;

    return state;
}

extern BRRlpItem
ethAccountStateRlpEncode(BREthereumAccountState state, BRRlpCoder coder) {
    BRRlpItem items[4];

    items[0] = rlpEncodeUInt64(coder, state.nonce, 0);
    items[1] = ethEtherRlpEncode(state.balance, coder);
    items[2] = ethHashRlpEncode(state.storageRoot, coder);
    items[3] = ethHashRlpEncode(state.codeHash, coder);

    return rlpEncodeListItems(coder, items, 4);
}

extern BREthereumAccountState
ethAccountStateRlpDecode (BRRlpItem item, BRRlpCoder coder) {
    BREthereumAccountState state;

    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);
    assert (4 == itemsCount);

    state.nonce = rlpDecodeUInt64(coder, items[0], 0);
    state.balance = ethEtherRlpDecode(items[1], coder);
    state.storageRoot = ethHashRlpDecode(items[2], coder);
    state.codeHash = ethHashRlpDecode(items[3], coder);

    return state;
}

