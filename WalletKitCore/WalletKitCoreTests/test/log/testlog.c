/*
 * testlog.c
 *
 *  Created on: Nov. 12, 2021
 *      Author: bryangoring
 */
#include <stdio.h>
#include <string.h>

// For use of logs in the system of loggers
#include "support/BRLog.h"

// For use of 'module' functionality
#include "module/module.h"

static void checkLevels(
    const char* logName, 
    BRLogLevel  level   ) {
    
    BRLogModule* list;
    size_t numDescs = getAllLogLevelsList (&list);
    
    for (size_t i=0; i < numDescs; i++) {
        if (NULL == logName)
            // Display rather than check
            printf ("%lu) %s lvl %u\n", i+1, list[i]->name, list[i]->level);
        
        if (logName != NULL && 0 == strcmp (logName, list[i]->name)) {
            if (list[i]->level == level)
                printf("OK %s matches level %d\n", logName, level);
            assert (list[i]->level == level);
        }
    }
    releaseAllLogLevelsList(list);
}
int main (int argc, char* argv[]) {
    
    // Need only be done once, from anywhere
    INIT_LOG();
    
    // Initialize a modular user of the log system, as perhaps 
    // within a blockchain
    LOG(LL_INFO, SYSTEM, "Initializing module");
    initModule();
    
    printf("------ERRR+WARN     -----\n");
    userFunction();
    printf("------DESCS         -----\n");
    checkLevels(NULL, 0);
    printf("------LEVEL SETTING -----\n");
    
    // Check setting a submodule level
    printf("  Set SAMPLE_SUBB LL_INFO\n");
    assert (0 == setLogLevel(LL_INFO, "SAMPLE", "SUBB"));
    checkLevels("SAMPLE_SUBB", LL_INFO);
    
    // Check setting a module level
    printf("  Set SAMPLE LL_VERBOSE\n");
    assert (0 == setLogLevel(LL_VERBOSE, "SAMPLE", UNSPECIFIED_LOG_MOD));
    checkLevels("SAMPLE", LL_VERBOSE);
    
    // Reset all levels and check (tests module wildcard * ie everything)
    printf("  Reset all levels\n");
    resetAllLogLevels();
    checkLevels("SAMPLE", LL_WARN);
    checkLevels("SAMPLE_SUBB", LL_WARN);
    printf("\nDone\n");
    
    // Check setting all submodules of a module with submodule wildcard
    printf("  Set all submodule levels\n");
    assert (0 == setLogLevel(LL_VERBOSE, "MODULAR", "*"));
    checkLevels("MODULAR", LL_VERBOSE);
    checkLevels("MODULAR_ONE", LL_VERBOSE);
    checkLevels("MODULAR_TWO", LL_VERBOSE);
    checkLevels("MODULAR_THREE", LL_VERBOSE);
    checkLevels("MODULAR_FOUR", LL_VERBOSE);
    
    // Check correct return on non-existant module set
    printf ("  Set level of non-existent module\n");
    assert (0 > setLogLevel(LL_VERBOSE, "AMPLE", UNSPECIFIED_LOG_MOD));
    
    return 0;
}



