//
//  AppDelegate.swift
//  CoreXDemo
//
//  Created by Ed Gamble on 11/8/18.
//  Copyright © 2018-2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import UIKit
import WalletKitKotlin

protocol SharedSystem {
    static var sharedSystem: System { get }
}

extension UIApplication: SharedSystem {
    static var sharedSystem : System {
        return (UIApplication.shared.delegate as! CoreDemoAppDelegate).system
    }
}

@UIApplicationMain
class CoreDemoAppDelegate: UIResponder, UIApplicationDelegate, UISplitViewControllerDelegate {

    var window: UIWindow?
    var summaryController: SummaryViewController!

    var listener: CoreDemoListener!
    var system: System!
    var mainnet = true

    var storagePath: String!

    var accountSpecification: AccountSpecification!
    var account: Account!
    var accountSerialization: Data!
    var accountUids: String!

    var query: BdbService2!

    var currencyCodesToMode: [String:WalletManagerMode]!
    var registerCurrencyCodes: [String]!

    var btcPeerSpec = (address: "103.99.168.100", port: UInt16(8333))
    var btcPeer: NetworkPeer? = nil
    var btcPeerUse = false

    var clearPersistentData: Bool = false

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.
        let splitViewController = window!.rootViewController as! UISplitViewController

        let walletNavigationController  = splitViewController.viewControllers[1] as! UINavigationController
        walletNavigationController.topViewController!.navigationItem.leftBarButtonItem = splitViewController.displayModeButtonItem
        splitViewController.delegate = self

        let summaryNavigationController = splitViewController.viewControllers[0] as! UINavigationController
        summaryController = (summaryNavigationController.topViewController as! SummaryViewController)

        print ("APP: Bundle Path       : \(Bundle(for: CoreDemoAppDelegate.self).bundlePath)")

        let accountSpecificationsPath = Bundle(for: CoreDemoAppDelegate.self).path(forResource: "CoreTestsConfig", ofType: "json")!
        let accountSpecifications     = AccountSpecification.loadFrom(configPath: accountSpecificationsPath)
        let accountIdentifier         = (CommandLine.argc >= 2 ? CommandLine.arguments[1] : "ginger")

        guard let accountSpecification = accountSpecifications.first (where: { $0.identifier == accountIdentifier })
            ?? (accountSpecifications.count > 0 ? accountSpecifications[0] : nil)
            else { preconditionFailure ("APP: No AccountSpecification: \(accountIdentifier)"); }

        self.accountSpecification = accountSpecification

        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "yyyy-MM-dd"
        dateFormatter.locale = Locale(identifier: "en_US_POSIX") // set locale to reliable US_POSIX

        accountUids = UUID (uuidString: "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96")!.uuidString

        account = Account.Companion().createFromPhrase(phrase: SwiftShims().stringToByteArray(string: accountSpecification.paperKey),
                                                       timestamp: Int64(accountSpecification.timestamp.asUnixTimestamp),
                                      uids: accountUids)
        guard nil != account
            else { preconditionFailure ("APP: No account") }

        mainnet = (accountSpecification.network == "mainnet")

        // Ensure the storage path
        storagePath = FileManager.default
            .urls(for: .documentDirectory, in: .userDomainMask)[0]
            .appendingPathComponent("Core").path

        do {
            // If data shouild be cleared, then remove `storagePath`
            if clearPersistentData {
                if FileManager.default.fileExists(atPath: storagePath) {
                    try FileManager.default.removeItem(atPath: storagePath)
                }
            }

            // Ensure that `storagePath` exists
            try FileManager.default.createDirectory (atPath: storagePath,
                                                     withIntermediateDirectories: true,
                                                     attributes: nil)
        }
        catch let error as NSError {
            print("APP: Error: \(error.localizedDescription)")
        }

        
        print ("APP: Account PaperKey  : \(accountSpecification.paperKey.components(separatedBy: CharacterSet.whitespaces).first ?? "<missed>") ...")
        print ("APP: Account Timestamp : \(account.timestamp)")
        print ("APP: Account UIDS      : \(account.uids)")
        print ("APP: StoragePath       : \(storagePath?.description ?? "<none>")");
        print ("APP: Mainnet           : \(mainnet)")

        currencyCodesToMode = [
            "btc" : WalletManagerMode.apiOnly,
            "eth" : WalletManagerMode.apiOnly,
            "bch" : WalletManagerMode.p2pOnly,
            "xrp" : WalletManagerMode.apiOnly,
            "hbar" : WalletManagerMode.apiOnly
            ]
        if mainnet {

        }
        else {

        }

        registerCurrencyCodes = [
//            "zla",
//            "adt"
        ]

        print ("APP: CurrenciesToMode  : \(currencyCodesToMode!)")

        // Create the listener
        listener = CoreDemoListener (networkCurrencyCodesToMode: currencyCodesToMode,
                                     registerCurrencyCodes: registerCurrencyCodes,
                                     isMainnet: mainnet)

        // Create the BlockChainDB
        let httpClient = SwiftShims().createHttpClient()
        query = BdbService2.Companion().createForTest(httpClient: httpClient, bdbAuthToken: "TODO", bdbBaseURL: "https://api.blockset.com")

        // Create the system
        self.system = System.Companion().create (
            executor: "",
            listener: listener,
             account: account,
             isMainnet: mainnet,
             storagePath: storagePath,
             query: BdbService.Companion().createForTest(httpClient: httpClient, bdbAuthToken: "TODO", bdbBaseURL: "https://api.blockset.com", apiBaseURL: ""))

        System.Companion().wipeAll(storagePath: storagePath, exemptSystems: [self.system])
        
        self.system.configureFromSwift(appCurrencies: [])

        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
        system.disconnectAll()
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        // Called as part of the transition from the background to the active state; here you can
        // undo many of the changes made on entering the background.
        system.connectAll()
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was
        // inactive. If the application was previously in the background, optionally refresh the
        // user interface.
    }

    func applicationWillTerminate(_ application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }

    // MARK: - Split view

    func splitViewController(_ splitViewController: UISplitViewController, collapseSecondary secondaryViewController:UIViewController, onto primaryViewController:UIViewController) -> Bool {
        guard let secondaryAsNavController = secondaryViewController as? UINavigationController else { return false }
        guard let topAsDetailController = secondaryAsNavController.topViewController as? WalletViewController else { return false }
        if topAsDetailController.wallet == nil {
            // Return true to indicate that we have handled the collapse by doing nothing; the secondary controller will be discarded.
            return true
        }
        return false
    }

}


extension UIApplication {
    static var paperKey: String {
        return (UIApplication.shared.delegate as! CoreDemoAppDelegate).accountSpecification.paperKey
    }

    static var accountSpecification: AccountSpecification {
        return (UIApplication.shared.delegate as! CoreDemoAppDelegate).accountSpecification
    }

    static func doSync () {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Syncing")
        app.system.walletManagers.forEach { $0.sync() }
    }

    static func doUpdateFees() {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Updating fees")
        app.system.updateNetworkFees()
    }

    static func doSleep () {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Disconnecting")
        app.system.walletManagers.forEach { $0.disconnect() }
        DispatchQueue.main.asyncAfter(deadline: .now() + 15.0) {
            print ("APP: Connecting")
            app.system.walletManagers.forEach { $0.connect(peer: nil) }
        }
    }

    static func doReset () {

    }

    static func doWipe () {
    }

    static func doError (network: Network) {
        DispatchQueue.main.async {
            guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
            print ("APP: Error (network)")

            let alert = UIAlertController (title: "Block Chain Access",
                                           message: "Can't access '\(network.name)'",
                preferredStyle: UIAlertController.Style.alert)

            alert.addAction (UIAlertAction (title: "Okay", style: UIAlertAction.Style.cancel))
            app.summaryController.present (alert, animated: true) {}
        }
    }

    static func peer (network: Network) -> NetworkPeer? {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return nil }
        guard "btc" == network.currency.code else { return nil }
        guard app.btcPeerUse else { return nil }

        if nil == app.btcPeer {
            app.btcPeer = network.createPeer (address: app.btcPeerSpec.address,
                                              port: app.btcPeerSpec.port,
                                              publicKey: nil)
        }

        return app.btcPeer
    }
}

extension Network {
    var scheme: String? {
        switch type {
        case .BTC(): return "bitcoin"
        case .BCH(): return (isMainnet ? "bitcoincash" : "bchtest")
        case .ETH(): return "ethereum"
        //case .xrp: return "ripple"
        //case .hbar: return "hedera"
//        case .xlm:  return "stellar"
        default:
            return nil
        }
    }
}
