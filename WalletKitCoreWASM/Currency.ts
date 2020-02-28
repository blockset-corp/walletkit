import CoreJS from "./libcorecrypto.js"

type Currency = number;

const CurrencyInterface = {
    "cryptoCurrencyCreate": CoreJS.cwrap('cryptoCurrencyCreate', 'int', [ 'string', 'string', 'string', 'string', 'string' ]),
    
    "currencyCreate": (uids: string, name: string, code: string, type: string, issuer: string | undefined): Currency => {
	uids; name; code; type; issuer;
	return 1;
    },
    "currencyGetUids": (core: Currency): string => {
	core;
	return 'uids';
    },
};

export {
    Currency,
    CurrencyInterface
}
