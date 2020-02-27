import * as Core from "../WalletKitCoreWASM";
import Currency from "./Currency"

export default class Unit {
  private core: Core.Unit;

  protected constructor (core: Core.Unit) {
    this.core = core;
  }

  get currency(): Currency {
    return new Currency (Core.Interface.unitGetCurrency (this.core))
  }

  get uids(): string {
    return Core.Interface.unitGetUids (this.core);
  }

  get name(): string {
    return 'name'
  }

  get symbol(): string {
    return 'symbol';
  }

  get baseUnit(): Unit {
    return new Unit (Core.Interface.unitGetBaseUnit(this.core));
  }

  get decimals(): number {
    return 18;
  }

  isCompatibleWith (unit: Unit): boolean {
    unit;
    return true;
  }

  hasCurrency (currency: Currency): boolean {
    currency;
    return true;
  }

  // equality
  // hash
}
