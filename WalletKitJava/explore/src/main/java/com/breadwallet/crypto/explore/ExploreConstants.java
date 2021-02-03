package com.breadwallet.crypto.explore;

public interface ExploreConstants {

    /// @brief Common definition for logger names and file tags
    final String            ExploreTag          = "xplr-";

    /// @brief Logs collected within this folder of application run root
    final String            ExploreLogFolder    = "/run/";

    /// @brief Application run root
    final String            ExploreHome         = System.getProperty("user.dir");
}
