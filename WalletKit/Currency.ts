import * as Core from "../WalletKitCoreWASM";

export default class Currency {
  private core: Core.Currency;

  public /* module protected */ constructor (core: Core.Currency) {
    this.core = core;
  }

  public static create (uids: string,
   			name: string,
   			code: string,
   			type: string,
   			issuer: string | undefined): Currency {
			  return new Currency (Core.Interface.currencyCreate (uids, name, code, type, issuer));
     }
  
  get uids(): string {
    return Core.Interface.currencyGetUids (this.core)
  }

  get code(): string {
    return "code";
  }

  get name(): string {
    return "name";
  }

  get type(): string {
    return "type";
  }

  get issuer(): string | undefined {
    return undefined;
  }

  // equality
  // hash
}
