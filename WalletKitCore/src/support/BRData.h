//
//  BRData.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/26/21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRData_h
#define BRData_h

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Data32 / Data16

typedef struct {
    uint8_t data[256/8];
} BRData32;

static inline void data32Clear (BRData32 *data32) {
    memset (data32, 0, sizeof (BRData32));
}

typedef struct {
    uint8_t data[128/8];
} BRData16;

static inline void data16Clear (BRData16 *data16) {
    memset (data16, 0, sizeof (BRData16));
}

/// MARK: - Variable Size Data

typedef struct {
    uint8_t * bytes;
    size_t size;
} BRData;

static inline BRData dataNew (size_t size) {
    BRData data;
    data.size = size;
    if (size < 1) data.size = 1;
    data.bytes = calloc (data.size, sizeof(uint8_t));
    assert (data.bytes != NULL);
    return data;
}

static inline BRData dataCreate (uint8_t *bytes, size_t bytesCount) {
    BRData data = dataNew (bytesCount);
    memcpy (data.bytes, bytes, bytesCount);
    return data;
}

static inline BRData dataCreateEmpty (void) {
    return (BRData) { NULL, 0};
}

static inline BRData dataCopy (uint8_t * bytes, size_t size) {
    BRData data = { NULL, 0 };

    if (NULL != bytes && 0 != size) {
        data.bytes = malloc (size * sizeof(uint8_t));
        memcpy (data.bytes, bytes, size);
        data.size = size;
    }

    return data;
}

static inline BRData dataClone (BRData data) {
    return dataCopy (data.bytes, data.size);
}

static inline BRData
dataConcat (BRData * fields, size_t numFields) {
    size_t totalSize = 0;
    for (int i=0; i < numFields; i++) {
        totalSize += fields[i].size;
    }
    BRData concat = dataNew (totalSize);
    totalSize = 0;
    for (int i=0; i < numFields; i++) {
        memcpy (&concat.bytes[totalSize], fields[i].bytes, fields[i].size);
        totalSize += fields[i].size;
    }
    return concat;
}

static inline BRData
dataConcatTwo (BRData data1, BRData data2) {
    BRData data[2] = { data1, data2 };
    return dataConcat (data, 2);
}

static inline void dataFree (BRData data) {
    if (data.bytes) free(data.bytes);
    data.bytes = NULL;
    data.size = 0;
}

#ifdef __cplusplus
}
#endif

#endif /* BRData_h */
