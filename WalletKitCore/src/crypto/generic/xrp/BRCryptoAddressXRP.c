
#include "BRCryptoXRP.h"
#include "ripple/BRRippleAddress.h"

/// A GEN address
struct BRCryptoAddressXRPRecord {
    BRRippleAddress xrp;
};

private_extern BRCryptoAddress
cryptoAddressCreateAsGEN (OwnershipGiven BRGenericAddress gen);

private_extern BRGenericAddress
cryptoAddressAsGEN (BRCryptoAddress address);



private_extern BRCryptoAddress
cryptoAddressCreateAsGEN (OwnershipGiven BRGenericAddress gen) {
    BRCryptoAddress address = cryptoAddressCreate (BLOCK_CHAIN_TYPE_GEN);
    address->u.gen = gen;
    return address;
}

private_extern BRGenericAddress
cryptoAddressAsGEN (BRCryptoAddress address) {
    assert (BLOCK_CHAIN_TYPE_GEN == address->type);
    return address->u.gen;
}

static BRCryptoAddress
cryptoAddressCreateFromStringAsGEN (BRGenericNetwork network, const char *string) {
    BRGenericAddress address = genAddressCreate (genNetworkGetType(network), string);
    return (NULL != address
            ? cryptoAddressCreateAsGEN (address)
            : NULL);
}




static BRGenericAddressRef
genericRippleAddressCreate (const char *string) {
    return (BRGenericAddressRef) rippleAddressCreateFromString (string, true);
}

static char *
genericRippleAddressAsString (BRGenericAddressRef address) {
    return rippleAddressAsString ((BRRippleAddress) address);
}

static int
genericRippleAddressEqual (BRGenericAddressRef address1,
                           BRGenericAddressRef address2) {
    return rippleAddressEqual ((BRRippleAddress) address1,
                               (BRRippleAddress) address2);
}

static void
genericRippleAddressFree (BRGenericAddressRef address) {
    rippleAddressFree ((BRRippleAddress) address);
}

// Release
//            genAddressRelease (address->u.gen);

// extern char *
// cryptoAddressAsString (BRCryptoAddress address) {
//            return genAddressAsString (address->u.gen);
//    }


// Equal
// genAddressEqual (a1->u.gen, a2->u.gen))

// createFromString
//            BRGenericNetwork gen = cryptoNetworkAsGEN (network);
//            return cryptoAddressCreateFromStringAsGEN (gen, string);

