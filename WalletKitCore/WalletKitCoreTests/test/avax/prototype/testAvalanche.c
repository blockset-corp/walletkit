//
//  testAvalanche.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "support/BRArray.h"
#include "support/BRCrypto.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRBIP39WordsEn.h"
#include "support/BRKey.h"
#include "support/BRBase58.h"

#include "avalanche/prototype/BRAvalancheAccount.h"
#include "avalanche/prototype/BRAvalancheAddress.h"
#include "avalanche/prototype/BRAvalancheTransaction.h"
#include "avalanche/prototype/BRAvaxTransaction.h"
#include "avalanche/prototype/BRAvaxEncode.h"

static int debug_log = 0;

static uint8_t char2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    return 0;
}

static void hex2bin(const char* src, uint8_t * target)
{
    while(*src && src[1])
    {
        *(target++) = (char2int(src[0]) << 4) | (char2int(src[1]) & 0x0f);
        src += 2;
    }
}

static void bin2HexString (uint8_t *input, size_t inputSize, char * output) {
    for (size_t i = 0; i < inputSize; i++) {
        sprintf(&output[i*2], "%02x", input[i]);
    }
}

static void printBytes(const char* message, uint8_t * bytes, size_t byteSize)
{
    if (message) printf("%s\n", message);
    for(int i = 0; i < byteSize; i++) {
        if (i >= 0 && i % 8 == 0) printf("\n");
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

static void printByteString(const char* message, uint8_t * bytes, size_t byteSize)
{
    if (message) printf("%s\n", message);
    for(int i = 0; i < byteSize; i++) {
        printf("%02X", bytes[i]);
    }
    printf("\n");
}

typedef struct {
    const char * paperKey;
    const char * pubKey;
    const char * privKey;
    const char * xaddress;
    const char * caddress;
    const char * ripemd160;
} TestAccount;

//test account was made via :
//https://iancoleman.io/bip39/ - bip32 seed phrase and decoded base58 encoded private seed to:
//https://wallet.avax.network/access/privatekey
TestAccount avaxTestAccount = {
    "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone",
    "029dc79308883267bb49f3924e9eb58d60bcecd17ad3f2f53681ecc5c668b2ba5f",
    "de7176242724956611e9a4f6dfb7a3b3b7eeeec0475b8bccdfec4e52a49c1466",
    "avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal",
    "bbc9bf879c06b13274c200c8b246881ef1ca33a0",
    "cc30e2015780a6c72efaef2280e3de4a954e770c"
    
};

// caller must free
static BRAvalancheAccount
makeAccount(TestAccount accountInfo) {
    UInt512 seed = UINT512_ZERO;
    //generate seed from paperKey
    BRBIP39DeriveKey(seed.u8, accountInfo.paperKey, NULL);
    return avalancheAccountCreateWithSeed(seed);
}

static BRKey
makeKey(TestAccount accountInfo){
    UInt512 seed = UINT512_ZERO;
    //generate seed from paperKey
    BRBIP39DeriveKey(seed.u8, accountInfo.paperKey, NULL);
    BRKey key = deriveAvalanchePrivateKeyFromSeed(seed);
    return key;
}


//Test-Case resource
//https://github.com/ava-labs/ledger-app-avalanche/blob/a340702ec427b15e7e4ed31c47cc5e2fb170ebdf/tests/basic-tests.js#L19


// MARK: -

//Get test cases from ava project itself
//https://github.com/ava-labs/avalanche-wallet/blob/master/tests/js/wallets/SingletonWallet.test.ts
//https://github.com/ava-labs/ledger-app-avalanche/blob/develop/tests/basic-tests.js

void testAddressDecode(){
    //test:
    uint8_t recovered[20];
    size_t rec_len;
   
    avax_addr_bech32_decode(recovered, &rec_len, "avax","avax1escwyq2hsznvwth6au3gpc77f225uacvwldgal" );
    printf("expected:");
    
    for(int i=0; i < rec_len; i++){
        printf("%02x", recovered[i]);
    }
    uint8_t ripemd160[20];
    hex2bin(avaxTestAccount.ripemd160, ripemd160);
    assert(0==memcmp(recovered,ripemd160, sizeof(ripemd160)));
}

void testSignatureGeneration(){
    
    BRKey key = makeKey(avaxTestAccount);
    key.compressed=0;
    uint8_t msg[5] = { 104,101,108,108,111};
    uint8_t sig[65];
    avaxDigestHashAndSignBytes(&key, &msg[0], sizeof(msg), &sig[0]);
    uint8_t exp_sig[65];
    hex2bin("f72ca286c8e6f1a0ddf1fb6ee18c93cd649cf058b4ce7e75fab3ab2cabeb29af2fdafd1b57bdefddfaddc3b89d333f5b5dbb02928a416f500792df201ad4424a01", exp_sig);
    assert(0==memcmp(sig, exp_sig, 65));
}

void testBasicSend(){
    
    char * memo = "hello";
    //SETUP THE WALLET TO WALLET TRANSFER
    
    struct TxIdRecord parentTx1;
    memcpy(parentTx1.base58,"XQYUrRZUMuHv6GXDer2oU9Gje6YkZWTVHLUdDWipWdMtpNVQh", 50);
    parentTx1.id = avaxTxidDecodeBase58(parentTx1);
    
    struct BRAssetRecord asset;
    memcpy(asset.base58,
    "U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK", 50);
    asset.id = avaxAssetDecodeAssetId(asset);
    
    BRArrayOf(struct AddressRecord) addresses;
    array_new(addresses, 1);
    struct AddressRecord addy;
    size_t addressLen;
    avax_addr_bech32_decode(&(addy.rmd160[0]), &addressLen, "fuji", "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q");
    array_add(addresses,addy);
    
    //this was a change address so it would have been second in the utxo
    struct BRAvaxUtxoRecord utxo1;
    utxo1.amount=9964000000;
    utxo1.asset = asset;
    utxo1.tx = parentTx1;
    utxo1.addresses = addresses;
    utxo1.output_index= 1;
    
   
    struct TxIdRecord parentTx2;
    memcpy(parentTx2.base58,"Da5BCvPhMEXK2bPEC5H7rssQYXiS1jnhGUntFiejtWvAbWmqP", 50);
    parentTx2.id = avaxTxidDecodeBase58(parentTx2);
    
    struct BRAvaxUtxoRecord utxo2;
    utxo2.amount=2000000000;
    utxo2.asset = asset;
    utxo2.tx = parentTx2;
    utxo2.addresses= addresses;
    utxo2.output_index = 0;
    
    
    BRArrayOf(struct BRAvaxUtxoRecord) utxos;
    array_new (utxos, 2);
    array_add(utxos, utxo1);
    array_add(utxos, utxo2);
    
    //CREATE THE BASE UNSIGNED TX FROM UTXOS
    
    struct BaseTxRecord  tx = avaxTransactionCreate("fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", "fuji1k3lf9kxsmyf9jyx4dlq7hffvyu4eppmv89w2f0","fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q","U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",12300000, utxos, "hello",NETWORK_ID_FUJI, "2JVSBoinj9C2J33VntvzYtVJNZdN2NKiwwKjcumHUWEb5DbBrm");
    
    //SERIALIZE INPUTS AND OUTPUTS
    char result_hex_string[200];
    
    size_t buffer1size;
    avaxPackTransferableOutput(tx.outputs[0],NULL,&buffer1size);
    uint8_t buffer1[buffer1size];
    avaxPackTransferableOutput(tx.outputs[0],&buffer1[0],&buffer1size);
    
    bin2HexString(&buffer1[0], buffer1size, &result_hex_string[0]);
    assert(0==strcmp(&result_hex_string[0], "3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000000bbaee000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c"));
    
    result_hex_string[0]='\0';
    size_t buffer2size;
    avaxPackTransferableOutput(tx.outputs[1],NULL,&buffer2size);
    uint8_t buffer2[buffer2size];
    avaxPackTransferableOutput(tx.outputs[1],&buffer2[0],&buffer2size);
    bin2HexString(&buffer2[0], buffer2size, &result_hex_string[0]);
    assert(0==strcmp(&result_hex_string[0], "3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000002511ba1e000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c"));
    
    
    result_hex_string[0]='\0';
    size_t buffer3size;
    avaxPackTransferableInput(tx.inputs[0],NULL,&buffer3size);
    uint8_t buffer3[buffer3size];
    avaxPackTransferableInput(tx.inputs[0], &buffer3[0],&buffer3size);
    bin2HexString(&buffer3[0], buffer3size, &result_hex_string[0]);
    assert(0==strcmp(&result_hex_string[0], "450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e693000000000100000000"));
    
    size_t final_buffer_size;
    avaxPackBaseTx(tx, NULL, &final_buffer_size);
    assert(307 == final_buffer_size);
    
    uint8_t buffer[final_buffer_size];
    avaxPackBaseTx(tx, &buffer[0], &final_buffer_size);
    
    
    char buffer_hex[final_buffer_size*4];
    bin2HexString(&buffer[0], final_buffer_size, &buffer_hex[0]);
    printf("\r\nfinal buffer: %s\rn", &buffer_hex[0]);
    //CLEANUP
    assert(0==strcmp(&buffer_hex[0], "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000000bbaee000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000002511ba1e000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c00000001450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e6930000000001000000000000000568656c6c6f"));
    releaseTransaction(&tx);
    
    
    //0.Create Transaction struct
    tx = avaxTransactionCreate("fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", "fuji1k3lf9kxsmyf9jyx4dlq7hffvyu4eppmv89w2f0","fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q","U8iRqJoiJm8xZHAacmvYyZVwqQx6uDNtQeP3CQ6fcgQk3JqnK",10300000000, utxos, "hello",NETWORK_ID_FUJI, "2JVSBoinj9C2J33VntvzYtVJNZdN2NKiwwKjcumHUWEb5DbBrm");
    
    //1. serialize Transaction
    avaxPackBaseTx(tx, NULL, &final_buffer_size);
    assert(395 == final_buffer_size);
    uint8_t bufferTx[final_buffer_size];
    avaxPackBaseTx(tx, &bufferTx[0], &final_buffer_size);
    
    //Assert serializatoin
    char buffer_hex2[final_buffer_size*4];
    bin2HexString(&bufferTx[0], final_buffer_size, &buffer_hex2[0]);
    printf("\r\nfinal buffer2: %s\rn", &buffer_hex2[0]);
    assert(0==strcmp(&buffer_hex2[0], "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000631f5dc000000000000000000000000100000001cc30e2015780a6c72efaef2280e3de4a954e770c3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000265ed870000000000000000000000000100000001b47e92d8d0d9125910d56fc1eba52c272b90876c000000021c8acd205ff6161efce0952071fc63ca8e99717bb829e74ce33997c2b369334b000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000500000000773594000000000100000000450a5390bcf287869b9dcef42ca6b4305fde20e5f29d40e719a87fe7dd043600000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000251e6930000000001000000000000000568656c6c6f"));
    
    //2. Generate TxHash to sign
    uint8_t txHash[32];
    avaxTxHash(&txHash[0], bufferTx, final_buffer_size);
    
    //assert hash
    char txHashHex[65];
    bin2HexString(&txHash[0], 32, &txHashHex[0]);
    assert(0==strcmp(txHashHex,"31d3ab6136b423dc0b4ed69769dcfd3207067d94534357e6410a61e5a6859b48"));
    
    
    BRKey key = makeKey(avaxTestAccount);
    key.compressed=0;
    
    //assert single signature
    struct BRAvaxCompactSignature sig;
    avaxHashAndSignBytes(&key, &bufferTx[0], final_buffer_size, &sig.bytes[0]);
    uint8_t exp_sig[65];
    hex2bin("c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d8900", exp_sig);
    assert(0==memcmp(sig.bytes, exp_sig, 65));
    
    //3.Generate Signatures for Tx
    BRArrayOf(struct BRAvaxCompactSignature) signatures;
    array_new(signatures, 2);
    for(int i=0; i < array_count(tx.inputs); i++){
        //Note: we only support Singleton Wallet
        assert(tx.inputs[i].input.secp256k1.address_indices_len == 1);
        struct BRAvaxCompactSignature sigBuffer;
        avaxHashAndSignBytes(&key, &bufferTx[0], final_buffer_size, &sigBuffer.bytes[0]);
        array_add(signatures, sigBuffer);
    }
    
    //4.Pack Signatures
    size_t signatureBufferSize;
    avaxPackSignatures(NULL, &signatureBufferSize, signatures);
    uint8_t signatureBuffer[signatureBufferSize];
    avaxPackSignatures(&signatureBuffer[0], &signatureBufferSize, signatures);
    
    //assert packed signatures
    uint8_t exp_sigs[signatureBufferSize];
    hexDecode(&exp_sigs[0], signatureBufferSize, "000000020000000900000001c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d89000000000900000001c7617e7e105b14481af67595376ab64d24e809f3986ee1133a5b77fafbda0db32cd46b17f884de9dccd1dcc13e9a9bfccb1999e49e6483f7e101a908bf4f6d8900", 300);
    assert(0==memcmp(&signatureBuffer[0], exp_sigs, signatureBufferSize));
    array_clear(signatures);
    array_free(signatures);
    
    //5. Append txBuffer with signature buffer
}

extern void runAvalancheTest (void) {
    printf("Running avalanche unit tests...\n");
    BRAvalancheAccount account = makeAccount(avaxTestAccount);
    //printf("%s", (char *)(account->xaddress.bytes));
   // printf("%s", (char *)(account->caddress.bytes));
    assert(0==memcmp(avaxTestAccount.xaddress, (char *)(account->xaddress.bytes), sizeof(account->xaddress.bytes)));
    
    uint8_t ethAddress[20];
    hex2bin(avaxTestAccount.caddress, ethAddress);
    assert(0==memcmp(ethAddress, (char*)account->caddress.bytes,sizeof(account->caddress.bytes)));
    
    
//    struct TxIdRecord parentTx;
//    memcpy(parentTx.base58,"XQYUrRZUMuHv6GXDer2oU9Gje6YkZWTVHLUdDWipWdMtpNVQh", 50);
//    parentTx.id = avaxTxidDecodeBase58(parentTx);
//
//    struct BRAssetRecord asset;
//    memcpy(asset.asset_id,
//    "3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa", 65);
//    asset.id = avaxAssetDecodeAssetId(asset);
//
//    BRArrayOf(struct AddressRecord) addresses;
//    array_new(addresses, 2);
//
//    struct AddressRecord addy;
//    size_t addressLen;
//    avax_addr_bech32_decode(&(addy.rmd160[0]), &addressLen, "fuji", "fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q");
//    array_add(addresses,addy);
//
//    struct BRAvaxUtxoRecord utxo1;
//    utxo1.amount=10;
//    utxo1.asset = asset;
//    utxo1.tx = parentTx;
//    utxo1.addresses = addresses;
//    utxo1.output_index= 2;
//
//
//
//
//
//    struct BRAvaxUtxoRecord utxo2;
//    utxo2.amount=2;
//    utxo2.asset = asset;
//    utxo2.tx = parentTx;
//    utxo2.addresses= addresses;
//    utxo2.output_index = 3;
//
//
//    BRArrayOf(struct BRAvaxUtxoRecord) utxos;
//    array_new (utxos, 2);
//    array_add(utxos, utxo1);
//    array_add(utxos, utxo2);
//
//    struct BaseTxRecord  tx = avaxTransactionCreate("fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q", "fuji1k3lf9kxsmyf9jyx4dlq7hffvyu4eppmv89w2f0","fuji1escwyq2hsznvwth6au3gpc77f225uacvzdfh3q","3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa",11, utxos);
//    //TODO typically need to free? :tx.inputs[0].input.secp256k1.address_indices;
//    array_clear(tx.inputs[0].input.secp256k1.address_indices);
//    array_clear(tx.inputs[1].input.secp256k1.address_indices);
//    array_free(tx.inputs[0].input.secp256k1.address_indices);
//    array_free(tx.inputs[1].input.secp256k1.address_indices);
//    array_clear(tx.inputs);
//    array_clear(tx.outputs);
//    array_free(tx.inputs);
//    array_free(tx.outputs);
    
    //releaseTransaction(tx);
    testBasicSend();
    testAddressDecode();
    testSignatureGeneration();
    printf("DONE");
}




