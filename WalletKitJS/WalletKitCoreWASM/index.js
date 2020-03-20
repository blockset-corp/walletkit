import { Currency, CurrencyInterface } from "./Currency"
export { Currency }

import { Unit, UnitInterface} from "./Unit"
export { Unit }

export const Interface = {
  ...CurrencyInterface,
  ...UnitInterface
};
