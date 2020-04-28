from core cimport *
from enum import Enum


class HasherType(Enum):
    SHA1 = CRYPTO_HASHER_SHA1
    SHA224 = CRYPTO_HASHER_SHA224
    SHA256 = CRYPTO_HASHER_SHA256
    SHA256_2 = CRYPTO_HASHER_SHA256_2
    SHA384 = CRYPTO_HASHER_SHA384
    SHA512 = CRYPTO_HASHER_SHA512
    SHA3 = CRYPTO_HASHER_SHA3
    RMD160 = CRYPTO_HASHER_RMD160
    HASH160 = CRYPTO_HASHER_HASH160
    KECCAK256 = CRYPTO_HASHER_KECCAK256
    MD5 = CRYPTO_HASHER_MD5


cdef class Hasher:
    cdef BRCryptoHasher _hasher

    def __init__(self, hasher_type: HasherType):
        pass

    def __cinit__(self, kind: HasherType):
        self._hasher = cryptoHasherCreate(kind.value)
        if self._hasher is NULL:
            raise MemoryError

    def __dealloc__(self):
        if self._hasher is not NULL:
            cryptoHasherGive(self._hasher)

    def hash(self, data: bytes) -> bytes:
        hashlen = cryptoHasherLength(self._hasher)
        out = bytes(1) * hashlen
        cryptoHasherHash(self._hasher, out, hashlen, data, len(data))
        return out
