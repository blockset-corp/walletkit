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
import WalletKit

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

    var blocksetAccess: BlocksetAccess!

    var client: SystemClient!

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

        let testConfiguration     = TestConfiguration.loadFromBundle(resource: "WalletKitTestsConfig")!
        let accountSpecifications = testConfiguration.accountSpecifications
        let accountIdentifier     = (CommandLine.argc >= 2 ? CommandLine.arguments[1] : "ginger")

        guard let accountSpecification = accountSpecifications.first (where: { $0.identifier == accountIdentifier })
            ?? (accountSpecifications.count > 0 ? accountSpecifications[0] : nil)
            else { preconditionFailure ("APP: No AccountSpecification: \(accountIdentifier)"); }

        self.accountSpecification = accountSpecification
        self.blocksetAccess = testConfiguration.blocksetAccess

        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "yyyy-MM-dd"
        dateFormatter.locale = Locale(identifier: "en_US_POSIX") // set locale to reliable US_POSIX

        accountUids = UUID (uuidString: "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96")!.uuidString

        account = Account.createFrom (phrase: accountSpecification.paperKey,
                                      timestamp: accountSpecification.timestamp,
                                      uids: accountUids)
        guard nil != account
            else { preconditionFailure ("APP: No account") }
        accountSerialization = account.serialize

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
            "btc" : .api_only,
            "bch" : .api_only,
            "bsv" : .p2p_only,
            "eth" : .api_only,
            "xrp" : .api_only,
            "hbar" : .api_only,
            "xtz": .api_only
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
        client = BlocksetSystemClient.createForTest (bdbBaseURL: self.blocksetAccess.baseURL,
                                                    bdbToken:   self.blocksetAccess.token)

        // Create the system
        self.system = System.create (client: client,
                                     listener: listener,
                                     account: account,
                                     onMainnet: mainnet,
                                     path: storagePath)

        System.wipeAll (atPath: storagePath, except: [self.system])
        
        // Subscribe to notificiations or not (Provide an endpoint if notifications are enabled).
        let subscriptionId = UIDevice.current.identifierForVendor!.uuidString
        let subscriptionEp = BlocksetSystemClient.SubscriptionEndpoint (environment: "env", kind: "knd", value: "val")
        let subscription = BlocksetSystemClient.Subscription (id: subscriptionId,
                                                              device: "ignore",
                                                              endpoint: subscriptionEp,
                                                              currencies: []);
        self.system.subscribe (using: subscription)

        self.system.configure(withCurrencyModels: [])

        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        system.pause()
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        system.resume()
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
        app.system.managers.forEach { $0.sync() }
    }

    static func doUpdateFees() {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Updating fees")
        app.system.updateNetworkFees()
    }

    static func doSleep () {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Disconnecting")
        app.system.managers.forEach { $0.disconnect() }
        DispatchQueue.main.asyncAfter(deadline: .now() + 15.0) {
            print ("APP: Connecting")
            app.system.managers.forEach { $0.connect() }
        }
    }

    static func doReset () {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Resetting")

        // Clear out the UI.  Note: there is a race here on callbacks to the listener.  Something
        // in the UI *might* be created if an event occurs before the subsequent wipe completes.
        app.summaryController.reset()

        // Destroy the current system.  This is an internal interface, hence the above:
        // `@testable import WalletKit`.  We don't want to wipe the file system.
        // System.destroy(system: app.system)

        // Or maybe we do want to wip the file system.
        System.wipe(system: app.system);

        // Again
        app.summaryController.reset()

        // Remove the reference to the old system
        app.system = nil;

        guard let account = Account.createFrom(serialization: app.accountSerialization,
                                               uids: app.accountUids)
            else { preconditionFailure ("APP: No Account on Reset") }

        print ("APP: Account PaperKey  : \(app.accountSpecification.paperKey.components(separatedBy: CharacterSet.whitespaces).first ?? "<missed>") ...")
        print ("APP: Account Timestamp : \(account.timestamp)")
        print ("APP: Account UIDS      : \(account.uids)")
        print ("APP: StoragePath       : \(app.storagePath?.description ?? "<none>")");
        print ("APP: Mainnet           : \(app.mainnet)")

        // Create a new system
        app.system = System.create (client: app.client,
                                    listener: app.listener!,
                                    account: account,
                                    onMainnet: app.listener.isMainnet,  // Wipe might change.
                                    path: app.storagePath)

        // Passing `[]`... it is a demo app...
        app.system.configure(withCurrencyModels: [])

        // Too soon...
        app.summaryController.update()
    }

    static func doWipe () {
        guard let app = UIApplication.shared.delegate as? CoreDemoAppDelegate else { return }
        print ("APP: Wiping")

        let accountSpecificationsPath = Bundle(for: CoreDemoAppDelegate.self).path(forResource: "CoreTestsConfig", ofType: "json")!
        let accountSpecifications     = AccountSpecification.loadFrom(configPath: accountSpecificationsPath)

        let alert = UIAlertController (title: "Select Paper Key",
                                       message: nil,
                                       preferredStyle: UIAlertController.Style.actionSheet)

        accountSpecifications
            .forEach { (accountSpecification) in
                let action = UIAlertAction (
                    title: "\(accountSpecification.identifier) (\(accountSpecification.network))",
                    style: .default)
                { (action) in
                    app.summaryController.reset()

                    System.wipe (system: app.system)

                    let mainnet = (accountSpecification.network == "mainnet")

                    app.accountSpecification = accountSpecification
                    app.account = Account.createFrom (phrase: accountSpecification.paperKey,
                                                      timestamp: accountSpecification.timestamp,
                                                      uids: "WalletID: \(accountSpecification.identifier)")

                    print ("APP: Account PaperKey  : \(accountSpecification.paperKey.components(separatedBy: CharacterSet.whitespaces).first ?? "<missed>") ...")
                    print ("APP: Account Timestamp : \(app.account.timestamp)")
                    print ("APP: Mainnet           : \(mainnet)")

                    // Create the listener
                    app.listener.isMainnet = mainnet

                    app.system = System.create (client: app.client,
                                                listener: app.listener!,
                                                account: app.account,
                                                onMainnet: mainnet,
                                                path: app.storagePath)

                    app.system.configure(withCurrencyModels: [])
                    alert.dismiss (animated: true) {
                        app.summaryController.reset()
                        app.summaryController.update()
                    }
                }
                alert.addAction (action)
        }
        alert.addAction (UIAlertAction (title: "Cancel", style: UIAlertAction.Style.cancel))

        app.summaryController.present (alert, animated: true) {}
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
        case .btc: return "bitcoin"
        case .bch: return (isMainnet ? "bitcoincash" : "bchtest")
        case .bsv: return "bitcoinsv"
        case .eth: return "ethereum"
        case .xrp: return "ripple"
        case .hbar: return "hedera"
        case .xtz: return "tezos"
//        case .xlm:  return "stellar"
        }
    }
}
