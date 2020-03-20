//import CoreJS from "./libcorecrypto.js"
//import CoreJS = require ("./libcorecrypto.js");

import { Currency } from "./Currency"

type Unit = number;

const UnitInterface = {
  "unitCreateAsBase": (currency: Currency, code: string, name: string, symbol: string): Unit => {
    currency; code; name; symbol;
    return 1;
  },
  "unitGetCurrency": (core: Unit): Currency => { core; return 1; },
  "unitGetUids":     (core: Unit): string   => { core; return 'uids'; },
  "unitGetBaseUnit": (core: Unit): Unit     => { core; return 2; },
};

export {
  Unit,
  UnitInterface
}
