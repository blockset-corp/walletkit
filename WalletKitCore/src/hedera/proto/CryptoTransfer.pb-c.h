/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: CryptoTransfer.proto */

#ifndef PROTOBUF_C_CryptoTransfer_2eproto__INCLUDED
#define PROTOBUF_C_CryptoTransfer_2eproto__INCLUDED

#include "protobuf-c.h"

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003002 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

#include "BasicTypes.pb-c.h"

typedef struct _Proto__AccountAmount Proto__AccountAmount;
typedef struct _Proto__TransferList Proto__TransferList;
typedef struct _Proto__CryptoTransferTransactionBody Proto__CryptoTransferTransactionBody;


/* --- enums --- */


/* --- messages --- */

/*
 * An account, and the amount that it sends or receives during a cryptocurrency transfer. 
 */
struct  _Proto__AccountAmount
{
  ProtobufCMessage base;
  /*
   * The Account ID that sends or receives cryptocurrency
   */
  Proto__AccountID *accountid;
  /*
   * The amount of tinybars that the account sends(negative) or receives(positive)
   */
  int64_t amount;
};
#define PROTO__ACCOUNT_AMOUNT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__account_amount__descriptor) \
    , NULL, 0 }


/*
 * A list of accounts and amounts to transfer out of each account (negative) or into it (positive). 
 */
struct  _Proto__TransferList
{
  ProtobufCMessage base;
  /*
   * Multiple list of AccountAmount pairs, each of which has an account and an amount to transfer into it (positive) or out of it (negative)
   */
  size_t n_accountamounts;
  Proto__AccountAmount **accountamounts;
};
#define PROTO__TRANSFER_LIST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__transfer_list__descriptor) \
    , 0,NULL }


/*
 * Transfer cryptocurrency from some accounts to other accounts. The accounts list can contain up to 10 accounts. The amounts list must be the same length as the accounts list. Each negative amount is withdrawn from the corresponding account (a sender), and each positive one is added to the corresponding account (a receiver). The amounts list must sum to zero. Each amount is a number of tinyBars (there are 100,000,000 tinyBars in one Hbar). If any sender account fails to have sufficient hbars to do the withdrawal, then the entire transaction fails, and none of those transfers occur, though the transaction fee is still charged. This transaction must be signed by the keys for all the sending accounts, and for any receiving accounts that have receiverSigRequired == true. The signatures are in the same order as the accounts, skipping those accounts that don't need a signature. 
 */
struct  _Proto__CryptoTransferTransactionBody
{
  ProtobufCMessage base;
  /*
   * Accounts and amounts to transfer
   */
  Proto__TransferList *transfers;
};
#define PROTO__WK_TRANSFER_TRANSACTION_BODY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__crypto_transfer_transaction_body__descriptor) \
    , NULL }


/* Proto__AccountAmount methods */
void   proto__account_amount__init
                     (Proto__AccountAmount         *message);
size_t proto__account_amount__get_packed_size
                     (const Proto__AccountAmount   *message);
size_t proto__account_amount__pack
                     (const Proto__AccountAmount   *message,
                      uint8_t             *out);
size_t proto__account_amount__pack_to_buffer
                     (const Proto__AccountAmount   *message,
                      ProtobufCBuffer     *buffer);
Proto__AccountAmount *
       proto__account_amount__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__account_amount__free_unpacked
                     (Proto__AccountAmount *message,
                      ProtobufCAllocator *allocator);
/* Proto__TransferList methods */
void   proto__transfer_list__init
                     (Proto__TransferList         *message);
size_t proto__transfer_list__get_packed_size
                     (const Proto__TransferList   *message);
size_t proto__transfer_list__pack
                     (const Proto__TransferList   *message,
                      uint8_t             *out);
size_t proto__transfer_list__pack_to_buffer
                     (const Proto__TransferList   *message,
                      ProtobufCBuffer     *buffer);
Proto__TransferList *
       proto__transfer_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__transfer_list__free_unpacked
                     (Proto__TransferList *message,
                      ProtobufCAllocator *allocator);
/* Proto__CryptoTransferTransactionBody methods */
void   proto__crypto_transfer_transaction_body__init
                     (Proto__CryptoTransferTransactionBody         *message);
size_t proto__crypto_transfer_transaction_body__get_packed_size
                     (const Proto__CryptoTransferTransactionBody   *message);
size_t proto__crypto_transfer_transaction_body__pack
                     (const Proto__CryptoTransferTransactionBody   *message,
                      uint8_t             *out);
size_t proto__crypto_transfer_transaction_body__pack_to_buffer
                     (const Proto__CryptoTransferTransactionBody   *message,
                      ProtobufCBuffer     *buffer);
Proto__CryptoTransferTransactionBody *
       proto__crypto_transfer_transaction_body__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__crypto_transfer_transaction_body__free_unpacked
                     (Proto__CryptoTransferTransactionBody *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Proto__AccountAmount_Closure)
                 (const Proto__AccountAmount *message,
                  void *closure_data);
typedef void (*Proto__TransferList_Closure)
                 (const Proto__TransferList *message,
                  void *closure_data);
typedef void (*Proto__CryptoTransferTransactionBody_Closure)
                 (const Proto__CryptoTransferTransactionBody *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor proto__account_amount__descriptor;
extern const ProtobufCMessageDescriptor proto__transfer_list__descriptor;
extern const ProtobufCMessageDescriptor proto__crypto_transfer_transaction_body__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_CryptoTransfer_2eproto__INCLUDED */
