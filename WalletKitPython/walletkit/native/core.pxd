from libc.stdint cimport uint8_t


cdef extern from "BRCryptoBase.h":
    ctypedef enum BRCryptoBoolean:
        CRYPTO_FALSE,
        CRYPTO_TRUE


cdef extern from "BRCryptoHasher.h":
    ctypedef enum BRCryptoHasherType:
        CRYPTO_HASHER_SHA1,
        CRYPTO_HASHER_SHA224,
        CRYPTO_HASHER_SHA256,
        CRYPTO_HASHER_SHA256_2,
        CRYPTO_HASHER_SHA384,
        CRYPTO_HASHER_SHA512,
        CRYPTO_HASHER_SHA3,
        CRYPTO_HASHER_RMD160,
        CRYPTO_HASHER_HASH160,
        CRYPTO_HASHER_KECCAK256,
        CRYPTO_HASHER_MD5

    ctypedef struct BRCryptoHasherRecord:
        pass

    ctypedef BRCryptoHasherRecord *BRCryptoHasher

    BRCryptoHasher cryptoHasherCreate(BRCryptoHasherType type)
    size_t cryptoHasherLength(BRCryptoHasher hasher)
    BRCryptoBoolean cryptoHasherHash(BRCryptoHasher hasher,
                                     uint8_t *dst,
                                     size_t dst_len,
                                     const uint8_t *src,
                                     size_t src_len)
    BRCryptoHasher cryptoHasherTake(BRCryptoHasher instance)
    BRCryptoHasher cryptoHasherTakeWeak(BRCryptoHasher instance)
    void cryptoHasherGive(BRCryptoHasher instance)
