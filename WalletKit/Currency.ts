type CoreCurrencyType = any;

export default class Currency {
  private core: CoreCurrencyType;

  public /* module protected */ constructor (core: CoreCurrencyType) {
    this.core = core;
  }

  // public static create (uids: string,
  // 			name: string,
  // 			code: string,
  // 			type: string,
  // 			issuer: string?): Currency? {
  //     return undefined;
  //
  //   }
  
  get uids(): string {
    this.core;
    //return this.core.cryptoCurrencyGetUids()
    return "uids";
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
