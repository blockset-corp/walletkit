import  Currency from './currency';

type CoreUnitType = any;

export default class Unit {
  private core: CoreUnitType;

  protected constructor (core: CoreUnitType) {
    this.core = core;
  }

  get currency(): Currency {
    this.core;
    return new Currency ('core');
  }

  get uids(): string {
    return 'uids';
  }

  get name(): string {
    return 'name'
  }

  get symbol(): string {
    return 'symbol';
  }

  get baseUnit(): Unit {
    return new Unit ('baseUnit');
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
