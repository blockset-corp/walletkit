//
//  testStellar.c
//  Core
//
//  Created by Carl Cherry on 5/21/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "support/BRArray.h"
#include "support/BRCrypto.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRBIP39WordsEn.h"
#include "support/BRKey.h"
#include "support/BRInt.h"

#include "hedera/BRHederaTransaction.h"
#include "hedera/BRHederaAccount.h"
#include "hedera/BRHederaWallet.h"

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

static void getPublicKey(const char * paper_key) {
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);
    BRKey publicKey = hederaAccountGetPublicKey(account);
    char publicKeyString[65] = {0};
    bin2HexString(publicKey.pubKey, 32, publicKeyString);
    printf("Public key: %s\n", publicKeyString);
    hederaAccountFree(account);
}
/*
const char * paper_key_24 = "inmate flip alley wear offer often piece magnet surge toddler submit right radio absent pear floor belt raven price stove replace reduce plate home";
const char * public_key_24 = "b63b3815f453cf697b53b290b1d78e88c725d39bde52c34c79fb5b4c93894673";
const char * paper_key_12 = "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone";
const char * paper_key_patient = "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone";
const char * public_key_12 = "ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f675585";
const char * paper_key_target = "choose color rich dose toss winter dutch cannon over air cash market";
const char * public_key_target = "372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d454404";
*/

struct account_info {
    const char * name;
    const char * account_string;
    const char * paper_key;
    const char * public_key;
};

struct account_info accounts[] = {
    {"none", "0.0.0", "", ""} ,
    {"patient", "0.0.114008", "patient doctor olympic frog force glimpse endless antenna online dragon bargain someone",
        "ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f675585"
    } ,
    {"choose", "0.0.114009", "choose color rich dose toss winter dutch cannon over air cash market",
        "372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d454404" },
    {"38618", "0.0.38618", "", ""}, // Our production operator account
    {"38230", "0.0.38230", "inmate flip alley wear offer often piece magnet surge toddler submit right radio absent pear floor belt raven price stove replace reduce plate home", "b63b3815f453cf697b53b290b1d78e88c725d39bde52c34c79fb5b4c93894673"},
    {"37664", "0.0.37664", "inmate flip alley wear offer often piece magnet surge toddler submit right radio absent pear floor belt raven price stove replace reduce plate home", "b63b3815f453cf697b53b290b1d78e88c725d39bde52c34c79fb5b4c93894673"},
    {"42725", "0.0.42725", "", ""},
};
size_t num_accounts = sizeof (accounts) / sizeof (struct account_info);

struct account_info find_account (const char * accountName) {
    if (strcmp(accountName, "default_account") == 0) {
        // If we get to here just return the first account
        return accounts[1];
    }
    for (size_t i = 0; i < num_accounts; i++) {
        if (strcmp(accounts[i].name, accountName) == 0)
            return accounts[i];
    }
    return accounts[0];
}

static int checkAddress (BRHederaAddress address, const char * accountName)
{
    struct account_info accountInfo = find_account (accountName);
    BRHederaAddress accountAddress = hederaAddressCreateFromString(accountInfo.account_string, true);
    int equal = hederaAddressEqual(address, accountAddress);
    hederaAddressFree (accountAddress);
    return equal;
}

static BRHederaAccount getDefaultAccount()
{
    // There is no account named "default_account" BUT the find function
    // will give me back a valid account anyway.
    struct account_info default_account = find_account("default_account");
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, default_account.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);
    BRHederaAddress address = hederaAddressCreateFromString(default_account.account_string, true);
    hederaAccountSetAddress(account, address);
    return account;
}

static BRHederaAccount getAccount(const char * name)
{
    // There is no account named "default_account" BUT the find function
    // will give me back a valid account anyway.
    struct account_info accountInfo = find_account(name);
    if (strcmp(accountInfo.name, "none") == 0) return NULL;

    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, accountInfo.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);
    BRHederaAddress address = hederaAddressCreateFromString(accountInfo.account_string, true);
    hederaAccountSetAddress(account, address);
    return account;
}

// For testing purposes just in case the Blockset code needs to be changed and we
// want to test it...
// - modify the code in hederaTransactionSignTransactionV0 to determine which node to use
extern size_t hederaTransactionSignTransactionV0 (BRHederaTransaction transaction, BRKey publicKey, UInt512 seed);
static BRHederaTransaction createSignedTransactionV0 (const char * source, const char * target,
                                  int64_t amount, int64_t seconds, int32_t nanos,
                                  int64_t fee, const char * memo)
{
    struct account_info source_account = find_account (source);
    struct account_info target_account = find_account (target);

    // Create a hedera account
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, source_account.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);

    BRHederaAddress sourceAddress = hederaAddressCreateFromString (source_account.account_string, true);
    BRHederaAddress targetAddress = hederaAddressCreateFromString (target_account.account_string, true);
    BRHederaTimeStamp timeStamp;
    timeStamp.seconds = seconds;
    timeStamp.nano = nanos;
    BRHederaFeeBasis feeBasis;
    feeBasis.costFactor = 1;
    feeBasis.pricePerCostFactor = fee;
    BRHederaTransaction transaction = hederaTransactionCreateNew(sourceAddress, targetAddress, amount,
                                                                 feeBasis, &timeStamp);

    if (memo) hederaTransactionSetMemo(transaction, memo);

    // Sign the transaction
    BRKey publicKey = hederaAccountGetPublicKey(account);
    hederaTransactionSignTransactionV0 (transaction, publicKey, seed);

    // Cleaup
    hederaAddressFree (sourceAddress);
    hederaAddressFree (targetAddress);
    hederaAccountFree (account);

    return transaction;
}

static BRHederaTransaction createSignedTransaction (const char * source, const char * target,
                                  int64_t amount, int64_t seconds, int32_t nanos,
                                  int64_t fee, const char * memo)
{
    struct account_info source_account = find_account (source);
    struct account_info target_account = find_account (target);

    // Create a hedera account
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, source_account.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);

    BRHederaAddress sourceAddress = hederaAddressCreateFromString (source_account.account_string, true);
    BRHederaAddress targetAddress = hederaAddressCreateFromString (target_account.account_string, true);
    BRHederaTimeStamp timeStamp;
    timeStamp.seconds = seconds;
    timeStamp.nano = nanos;
    BRHederaFeeBasis feeBasis;
    feeBasis.costFactor = 1;
    feeBasis.pricePerCostFactor = fee;
    BRHederaTransaction transaction = hederaTransactionCreateNew(sourceAddress, targetAddress, amount,
                                                                 feeBasis, &timeStamp);

    if (memo) hederaTransactionSetMemo(transaction, memo);

    // Sign the transaction
    BRKey publicKey = hederaAccountGetPublicKey(account);
    hederaTransactionSignTransaction (transaction, publicKey, seed);

    // Cleaup
    hederaAddressFree (sourceAddress);
    hederaAddressFree (targetAddress);
    hederaAccountFree (account);

    return transaction;
}

static void createNewTransaction (const char * source, const char * target,
                                   int64_t amount, int64_t seconds, int32_t nanos,
                                   int64_t fee, const char * memo, const char * expectedOutput, bool printOutput,
                                  const char * expectedHash)
{
    BRHederaTransaction transaction = createSignedTransaction(source, target, amount, seconds, nanos, fee, memo);

    // Get the signed bytes
    size_t serializedSize = 0;
    uint8_t * serializedBytes = hederaTransactionSerialize(transaction, &serializedSize);
    char * transactionOutput = NULL;
    if (printOutput || expectedOutput) {
        transactionOutput = calloc (1, (serializedSize * 2) + 1);
        bin2HexString (serializedBytes, serializedSize, transactionOutput);
        if (expectedOutput) {
            assert (strcmp (expectedOutput, transactionOutput) == 0);
        }
        if (printOutput) {
            printf("Transaction output:\n%s\n", transactionOutput);
        }
        free (transactionOutput);
    }
    // Get the transaction ID and hash
    BRHederaTransactionHash hash = hederaTransactionGetHash(transaction);
    printByteString("Transaction Hash: ", hash.bytes, sizeof(hash.bytes));
    if (expectedHash) {
        char hashString[97] = {0};
        bin2HexString(hash.bytes, 48, hashString);
        assert(strcasecmp(hashString, expectedHash) == 0);
    }
    char * transactionID = hederaTransactionGetTransactionId(transaction);
    printf("Transaction ID: %s\n", transactionID);

    // Cleanup
    hederaTransactionFree (transaction);
    free(transactionID);
}

static void transaction_value_test(const char * source, const char * target, int64_t amount,
                                   int64_t seconds, int32_t nanos, int64_t fee)
{
    BRHederaTransaction transaction = createSignedTransaction(source, target, amount, seconds, nanos, fee, NULL);
    // Check the fee and amount
    BRHederaUnitTinyBar txFee = hederaTransactionGetFee (transaction);
    assert (txFee == fee);
    BRHederaUnitTinyBar txAmount = hederaTransactionGetAmount (transaction);
    assert (txAmount == amount);

    // Check the addresses
    BRHederaAddress sourceAddress = hederaTransactionGetSource (transaction);
    BRHederaAddress targetAddress = hederaTransactionGetTarget (transaction);
    assert (1 == checkAddress(sourceAddress, source));
    assert (1 == checkAddress(targetAddress, target));

    hederaAddressFree (sourceAddress);
    hederaAddressFree (targetAddress);
    hederaTransactionFree (transaction);
}

static void createExistingTransaction(const char * sourceUserName, const char *targetUserName, int64_t amount)
{
    struct account_info sourceAccountInfo  = find_account (sourceUserName);
    struct account_info targetAccountInfo = find_account (targetUserName);
    // Create a hedera account
    BRHederaAccount account = getDefaultAccount();

    BRHederaAddress source = hederaAddressCreateFromString (sourceAccountInfo.account_string, true);
    BRHederaAddress target = hederaAddressCreateFromString (targetAccountInfo.account_string, true);

    const char * txId = "hedera-mainnet:0.0.14623-1568420904-460838529-0";
    BRHederaTransactionHash expectedHash;
    // Create a fake hash for this transaction
    BRSHA256(expectedHash.bytes, sourceAccountInfo.paper_key, strlen(sourceAccountInfo.paper_key));
    BRHederaUnitTinyBar fee = 500000;
    uint64_t timestamp = 1000;
    uint64_t blockHeight = 1000;
    BRHederaTransaction transaction = hederaTransactionCreate(source, target, amount, fee, txId, expectedHash,
                                                              timestamp, blockHeight, 0);

    // Check the values
    BRHederaTransactionHash hash = hederaTransactionGetHash(transaction);
    assert(memcmp(hash.bytes, expectedHash.bytes, 32) == 0);
    BRHederaAddress address = hederaTransactionGetSource(transaction);
    assert(hederaAddressEqual(source, address) == 1);
    hederaAddressFree(address);

    address = hederaTransactionGetTarget(transaction);
    assert(hederaAddressEqual(target, address) == 1);
    hederaAddressFree(address);

    BRHederaUnitTinyBar txAmount = hederaTransactionGetAmount(transaction);
    assert(amount == txAmount);

    char * transactionId = hederaTransactionGetTransactionId(transaction);
    assert(strcmp(txId, transactionId) == 0);
    free (transactionId);

    hederaTransactionFree(transaction);
    hederaAccountFree(account);
    hederaAddressFree (source);
    hederaAddressFree (target);
}

static void hederaAccountCheckPublicKey(const char * userName)
{
    struct account_info accountInfo = find_account (userName);
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, accountInfo.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);
    BRKey publicKey = hederaAccountGetPublicKey(account);
    // Validate the public key
    uint8_t expected_public_key[32];
    hex2bin(accountInfo.public_key, expected_public_key);
    assert(memcmp(expected_public_key, publicKey.pubKey, 32) == 0);
}

static void hederaAccountCheckSerialize(const char * userName)
{
    struct account_info accountInfo = find_account (userName);
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, accountInfo.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed(seed);
    assert (hederaAddressIsUninitializedAddress (hederaAccountGetPrimaryAddress(account)));
    BRHederaAddress address = hederaAddressCreateFromString(accountInfo.account_string, true);
    if (0 == strcmp(userName, "none"))
        assert (hederaAddressIsUninitializedAddress(address));
    BRKey key1 = hederaAccountGetPublicKey(account);
    hederaAccountSetAddress(account, address);
    size_t accountByteSize = 0;
    uint8_t * accountBytes = hederaAccountGetSerialization(account, &accountByteSize);
    // Now create a new account
    BRHederaAccount account2 = hederaAccountCreateWithSerialization(accountBytes, accountByteSize);
    BRHederaAddress account2Address = hederaAccountGetAddress(account2);
    assert( 1 == hederaAddressEqual(address, account2Address));

    BRKey key2 = hederaAccountGetPublicKey(account2);
    assert(0 == memcmp(key1.pubKey, key2.pubKey, 32));

    hederaAddressFree(address);
    hederaAddressFree(account2Address);
    hederaAccountFree(account);
    hederaAccountFree(account2);
}

static void accountStringTest(const char * userName) {
    struct account_info accountInfo = find_account (userName);
    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey(seed.u8, accountInfo.paper_key, NULL); // no passphrase
    BRHederaAccount account = hederaAccountCreateWithSeed (seed);
    BRHederaAddress inAddress = hederaAddressCreateFromString (accountInfo.account_string, true);
    hederaAccountSetAddress (account, inAddress);

    // Now get the address from the account
    BRHederaAddress outAddress = hederaAccountGetAddress (account);
    assert(hederaAddressEqual(inAddress, outAddress) == 1);

    char * outAddressString = hederaAddressAsString (outAddress);
    assert(strcmp(outAddressString, accountInfo.account_string) == 0);

    // Cleanup
    hederaAddressFree (inAddress);
    hederaAddressFree (outAddress);
    free (outAddressString);
    hederaAccountFree (account);
}

static void addressEqualTests()
{
    BRHederaAddress a1 = hederaAddressCreateFromString("0.0.1000000", true);
    BRHederaAddress a2 = hederaAddressClone (a1);
    BRHederaAddress a3 = hederaAddressCreateFromString("0.0.1000000", true);
    assert(1 == hederaAddressEqual(a1, a2));
    assert(1 == hederaAddressEqual(a1, a3));

    // now check no equal
    BRHederaAddress a4 = hederaAddressCreateFromString("0.0.1000001", true);
    assert(0 == hederaAddressEqual(a1, a4));

    hederaAddressFree (a1);
    hederaAddressFree (a2);
    hederaAddressFree (a3);
    hederaAddressFree (a4);
}

static void addressCloneTests()
{
    BRHederaAddress a1 = hederaAddressCreateFromString("0.0.1000000", true);
    BRHederaAddress a2 = hederaAddressClone (a1);
    BRHederaAddress a3 = hederaAddressClone (a1);
    BRHederaAddress a4 = hederaAddressClone (a1);
    BRHederaAddress a5 = hederaAddressClone (a2);
    BRHederaAddress a6 = hederaAddressClone (a3);

    assert(1 == hederaAddressEqual(a1, a2));
    assert(1 == hederaAddressEqual(a1, a3));
    assert(1 == hederaAddressEqual(a1, a4));
    assert(1 == hederaAddressEqual(a1, a5));
    assert(1 == hederaAddressEqual(a1, a6));
    assert(1 == hederaAddressEqual(a2, a3));

    // If we indeed have copies then we should not get a double free
    // assert error here.
    hederaAddressFree (a1);
    hederaAddressFree (a2);
    hederaAddressFree (a3);
    hederaAddressFree (a4);
    hederaAddressFree (a5);
    hederaAddressFree (a6);
}

static void addressFeeTests()
{
    BRHederaAddress feeAddress = hederaAddressCreateFromString("__fee__", false);
    assert (1 == hederaAddressIsFeeAddress(feeAddress));
    char * feeAddressString = hederaAddressAsString (feeAddress);
    assert(0 == strcmp(feeAddressString, "__fee__"));
    free (feeAddressString);
    hederaAddressFree(feeAddress);

    BRHederaAddress address = hederaAddressCreateFromString("0.0.3", true);
    assert (0 == hederaAddressIsFeeAddress(address));
    hederaAddressFree(address);
}

static void addressValueTests() {
    BRHederaAddress address = hederaAddressCreateFromString("0.0.0", true);
    assert(0 == hederaAddressGetShard (address));
    assert(0 == hederaAddressGetRealm (address));
    assert(0 == hederaAddressGetAccount (address));
    hederaAddressFree (address);

    // Check a max int64 account number
    address = hederaAddressCreateFromString("0.0.9223372036854775807", true);
    assert(0 == hederaAddressGetShard (address));
    assert(0 == hederaAddressGetRealm (address));
    assert(9223372036854775807 == hederaAddressGetAccount (address));
    hederaAddressFree (address);

    // Check when all numbers are max int64
    address = hederaAddressCreateFromString("9223372036854775807.9223372036854775807.9223372036854775807", true);
    assert(9223372036854775807 == hederaAddressGetShard (address));
    assert(9223372036854775807 == hederaAddressGetRealm (address));
    assert(9223372036854775807 == hederaAddressGetAccount (address));
    hederaAddressFree (address);

    // TODO - Check when the number (string) is too long
    //address = hederaAddressCreateFromString("0.0.9223372036854775807999");
    //assert(address == NULL);

    // TODO - Check when the number is the the correct length but is greater than max int64
    //address = hederaAddressCreateFromString("0.0.9993372036854775807");
    //assert(address == NULL);

    // Check when the string is invalid
    address = hederaAddressCreateFromString("0.0", true);
    assert(address == NULL);

    // Check an empty string
    address = hederaAddressCreateFromString("", true);
    assert(address == NULL);

    // Check an null - cannot check this since due to assert in function
    // address = hederaAddressCreateFromString(NULL);
    // assert(address == NULL);
}

static void createAndDeleteWallet()
{
    BRHederaAccount account = getDefaultAccount();

    BRHederaWallet wallet = hederaWalletCreate(account);

    // Source and target addresses should be the same for Hedera
    struct account_info defaultAccountInfo = find_account("default_account");
    BRHederaAddress expectedAddress = hederaAddressCreateFromString(defaultAccountInfo.account_string, true);

    BRHederaAddress sourceAddress = hederaWalletGetSourceAddress(wallet);
    assert(hederaAddressEqual(sourceAddress, expectedAddress) == 1);

    BRHederaAddress targetAddress = hederaWalletGetTargetAddress(wallet);
    assert(hederaAddressEqual(targetAddress, expectedAddress) == 1);

    hederaAccountFree(account);
    hederaWalletFree(wallet);
    hederaAddressFree (expectedAddress);
    hederaAddressFree (sourceAddress);
    hederaAddressFree (targetAddress);
}

static void walletBalanceTests()
{
    BRHederaAccount account = getAccount ("patient"); // Our wallet account
    BRHederaWallet wallet = hederaWalletCreate (account);
    BRHederaUnitTinyBar expectedBalance = 0;

    // Now add a few transfers for this wallet (3 TO and 1 FROM)
    BRHederaTransaction tx1 = createSignedTransaction("choose", "patient", 2000000000, 1, 0, 500000, NULL);
    BRHederaTransaction tx2 = createSignedTransaction("choose", "patient", 1500000000, 2, 0, 500000, NULL);
    BRHederaTransaction tx3 = createSignedTransaction("choose", "patient", 1400000000, 3, 0, 500000, NULL);
    BRHederaTransaction tx4 = createSignedTransaction("patient", "choose", 1400000000, 4, 0, 500000, NULL);
    expectedBalance = 2000000000L + 1500000000L + 1400000000L;
    hederaWalletAddTransfer(wallet, tx1);
    hederaWalletAddTransfer(wallet, tx2);
    hederaWalletAddTransfer(wallet, tx3);
    BRHederaUnitTinyBar balance = hederaWalletGetBalance (wallet);
    assert(balance == expectedBalance);

    hederaWalletAddTransfer(wallet, tx4);
    balance = hederaWalletGetBalance (wallet);
    expectedBalance -= (1400000000L + 500000L);
    assert(balance == expectedBalance);

    hederaAccountFree (account);
    hederaWalletFree (wallet);
    hederaTransactionFree(tx1);
    hederaTransactionFree(tx2);
    hederaTransactionFree(tx3);
    hederaTransactionFree(tx4);
}

static void create_real_v0_transactions() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // Create one with a memo
    const char * memo = "Version 0 transaction";
    BRHederaTransaction transaction = createSignedTransactionV0("37664", "38230", 20000, tv.tv_sec, tv.tv_usec, 500000, memo);
    size_t size = 0;
    uint8_t * signedBytes = hederaTransactionSerialize(transaction, &size);

    printf("V0 transaction start\n===========================\n");
    for (int i = 0; i < size; i++) {
        printf("%02X", signedBytes[i]);
    }
    printf("\n===========================\nV0 transaction end\n");

    hederaTransactionFree(transaction);
}

static void create_real_v1_transactions() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // Create one with a memo
    const char * memo = "Version 1 transaction";
    BRHederaTransaction transaction = createSignedTransaction("37664", "38230", 20000, tv.tv_sec, tv.tv_usec, 500000, memo);
    size_t size = 0;
    uint8_t * signedBytes = hederaTransactionSerialize(transaction, &size);

    printf("V1 transaction start\n===========================\n");
    for (int i = 0; i < size; i++) {
        printf("%02X", signedBytes[i]);
    }
    printf("\n===========================\nV1 transaction end\n");

    hederaTransactionFree(transaction);
}

static void multi_serialization_test()
{
    BRHederaTransaction transaction = createSignedTransaction("choose", "patient", 50000000,
                                                              1571928273, 123456789, 500000, NULL);
    size_t size = 0;
    uint8_t * signedBytes = hederaTransactionSerialize(transaction, &size);

    for (int i = 0; i < 10; i++) {
        printf("%02X ", signedBytes[i]);
    }
    printf("\n");

    // Check the version
    char version = signedBytes[0];
    assert(version == 1);

    uint16_t numNodes = UInt16GetBE(&signedBytes[1]);
    assert(numNodes == 10);

    uint8_t * pBuffer = signedBytes + 3;
    for (int i = 0; i < numNodes; i++) {
        // Get the node number and length of the buffer
        uint16_t nodeNumber = UInt16GetBE(pBuffer);
        assert(nodeNumber >= 3 && nodeNumber <= 12);
        pBuffer += 2;
        uint32_t serializationSize = UInt32GetBE(pBuffer);
        pBuffer += serializationSize + 4;
    }

    assert(pBuffer - signedBytes == size);
    free(signedBytes);
}

static void create_new_transactions() {
    const char * testOneOutput = "01000a0003000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a40e9086013e266e779a08a6b5f56efef98a1d9a9a5d3dce2f40dba01b35ea429247872c98e2fe0f6150ba3d82e7b9848a2c95d118d9f8bc66ae285be42d1e94407223b0a0e0a060889f0c6ed05120418d8fa061202180318a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac4090004000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a4083b843f3ee19c153bc633d3ac9124f0d86714e6020cdb79410cc75db7aceb275ae7fc4522b4df230f3478c9cc7436477b0a416a9a7ae8db3d99219b50232d702223b0a0e0a060889f0c6ed05120418d8fa061202180418a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac4090005000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a406b9a5fd8bdba1f978dd7d57d84a609a75dbd17f4d4d544094f34dbae6a1b6b36488b2c902146c2220c7260159f69ae8e46b27f9c634bed3f72a8d24fb5069004223b0a0e0a060889f0c6ed05120418d8fa061202180518a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac4090006000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a40326ca4a0557b7daad9d3acef8a7ef04883c428548fd875a35dab884ccc351381c235e0f3d3aaa11abe16dfd520cb8fcf79dcca31a052b635775a795dbc71f306223b0a0e0a060889f0c6ed05120418d8fa061202180618a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac4090007000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a4096709493797e5e479c21f922ba990cbf821c19d89a122bcf1644a261ccf1a67bc2f2e6b5c0d3d81ea89aaf3f785e54ee6f7f4ac6713fb489602f741531323404223b0a0e0a060889f0c6ed05120418d8fa061202180718a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac4090008000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a408ccabc857ac46d60880645df94063d1afc616288538751dc94377abb17d44fb93ce6c5cadc5a42c3c244896a600c470ed844c1f472aea1665233d2309d736c09223b0a0e0a060889f0c6ed05120418d8fa061202180818a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac4090009000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a40d0c36c063ba36c55f00afe5c99c664d6d9510f0c436a9e34cacacf0cf2d9a28b597abbb7f3fa598e6baccd7f9fbef803b54d365da680d780720d168e5788a50a223b0a0e0a060889f0c6ed05120418d8fa061202180918a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac409000a000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a407710b69ae3d25237e3c0d880690a8e72a2b3be76e34b833c33f3ed0e0f02d648581b8c632c3ee355610fd7a5658486ab3fe76bdd480da55b284a104fa83d0807223b0a0e0a060889f0c6ed05120418d8fa061202180a18a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac409000b000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a400865e4812e0a8d5991553342f25686864e5fee4b9db08cbab9990555945ac326c5450da3b66e122f36660e54f44ea1c76433fe60b1b9c7dadcdc76602406d101223b0a0e0a060889f0c6ed05120418d8fa061202180b18a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac409000c000000a51a660a640a20ec7554cc83ba25a9b6ca44f491de24881af4faba8805ba518db751d62f6755851a40b80fa3a198ce5300735b09fc8feb4b9b869946901794de1c82db9631b154eb7030d6e99f0444f23b47b9bc0322c35d1cbbef47b1c10cba73a62c2f872c070503223b0a0e0a060889f0c6ed05120418d8fa061202180c18a0c21e220308b401721c0a1a0a0b0a0418d8fa0610ffd9c4090a0b0a0418d9fa061080dac409";
    // Send 10,000,000 tiny bars to "choose" from "patient" via node3.
    char * hash = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    createNewTransaction ("patient", "choose", 10000000, 1571928073, 0, 500000, NULL, testOneOutput, false, hash);

    const char * testTwoOutput = "01000a0003000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40be090d58fb3926c5e3e3f8bd19badca4189a42d7ce336bf4e736738bf3932c8b9a12e79bcab3e94beeca17e2acd027c6baedc8b74d70b63669319927bb39f700223b0a0e0a0608d1f1c6ed05120418d9fa061202180318a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f0004000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40ac13d0011874105ddf09d40362616c1fe529f85cf13e05764c563ecfe4daac6b91541c171c92346eac525eea5c83a47eb2a73f7c8251ca317fbb704839a09909223b0a0e0a0608d1f1c6ed05120418d9fa061202180418a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f0005000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40cfa79836161be6504d735eedb587a699956b5bce84f5519f258d213c27c2d693726bab354f0e8ec5e94db08e46d59fc0a0e6a7ab36ba2f3b901e8c3d3bd7ef06223b0a0e0a0608d1f1c6ed05120418d9fa061202180518a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f0006000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40995b7345099b2247654e95d1675126acc7de7d0ca03750ee47fc20d217538642a393c9eb8e0deca9c67bc71006af653f7a2e00f06b4a38a4337b332beeab0200223b0a0e0a0608d1f1c6ed05120418d9fa061202180618a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f0007000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40e4ffe742416d5188adf42baad54c85aa1e6bdcbf9328d3d60684a3896a32cccd66c68ac17d8521cc2e1de4fc3a20d18f0673154d292d2544da7d7cdaa7a45f0b223b0a0e0a0608d1f1c6ed05120418d9fa061202180718a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f0008000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40e1c4c43eeeb0456cc53052dbcf2d57441a035c781ed0f0b9997d4bee38840725adacd777ae4992631e7d5d8685b1e452318ba45e50dbf27801413f82324ff708223b0a0e0a0608d1f1c6ed05120418d9fa061202180818a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f0009000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a4084aa0821d1a527461ea8c877a672780b1bbd332e20767b083922394e92e16da7dcba26bfb8ba566351aba8b4c323819bf5475cff7bb602f7e56b15124ace7f0a223b0a0e0a0608d1f1c6ed05120418d9fa061202180918a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f000a000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a404d7fad58ab19cb898fc05910699cbb65b12f8b55853610f5f8971f056c5bedf130d00aaa0b4ab167306fe91b9f510aa5d867402fb70cf5377f732ace1ba5880a223b0a0e0a0608d1f1c6ed05120418d9fa061202180a18a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f000b000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a40daeb4fc4caf74170f253935682fdd6363cac8b10897c6f6e91da8aee1d4433b61f9bf6373f46325683556caf709ce7e467aa885eff8f160ffe019ae292a3750a223b0a0e0a0608d1f1c6ed05120418d9fa061202180b18a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f000c000000a51a660a640a20372c41776cbdb5cacc7c41ec75b17ad9bd3f242f5c4ab13a1bbeef274d4544041a4060fb5849e129ca0d5bf962c3e8e9dcfedee17709dad9633eac43e3e5bff725f34a25e4235e3a9ac7f6d50655c0ede1e440af429e168d8702b259f066ff6e0b05223b0a0e0a0608d1f1c6ed05120418d9fa061202180c18a0c21e220308b401721c0a1a0a0b0a0418d9fa0610ffc1d72f0a0b0a0418d8fa061080c2d72f";
    // Send 50,000,000 tiny bars to "patient" from "choose" via node3
    const char * hash2 = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    createNewTransaction ("choose", "patient", 50000000, 1571928273, 0, 500000, NULL, testTwoOutput, false, hash2);
}

static void address_tests() {
    addressEqualTests();
    addressValueTests();
    addressFeeTests();
    addressCloneTests();
}

static void account_tests() {
    hederaAccountCheckPublicKey("38230");
    hederaAccountCheckPublicKey("patient");
    hederaAccountCheckPublicKey("choose");

    hederaAccountCheckSerialize("patient");
    hederaAccountCheckSerialize("none");

    accountStringTest("patient");
}

static void wallet_tests()
{
    createAndDeleteWallet();
    walletBalanceTests();
}

static void transaction_tests() {
    createExistingTransaction("patient", "choose", 400);
    create_new_transactions();
    transaction_value_test("patient", "choose", 10000000, 25, 4, 500000);
    multi_serialization_test();
}

static void txIDTests() {
    const char * txID1 = "hedera-mainnet:0.0.14222-1569828647-256912000-0";
    BRHederaTimeStamp ts1 = hederaParseTimeStamp(txID1);
    assert(ts1.seconds == 1569828647);
    assert(ts1.nano == 256912000);
}

extern void
runHederaTest (void /* ... */) {
    printf("Running hedera unit tests...\n");
    address_tests();
    account_tests();
    wallet_tests();
    transaction_tests();
    txIDTests();

    getPublicKey("");

    //create_real_v0_transactions();
    //create_real_v1_transactions();
}
