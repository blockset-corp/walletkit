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

int main (int argc, char* argv[]) {
    
    // Need only be done once, from anywhere
    INIT_LOG();
    
    // Initialize a modular user of the log system, as perhaps 
    // within a blockchain
    LOG(LL_INFO, SYSTEM, "Intializing module");
    initModule();
    
    printf("---ERRR+WARN-----\n");
    userFunction();
    printf("------DESCS------\n");
    BRLogModule* list;
    size_t numDescs = getAllLogLevelsList (&list);
    printf ("Number of system logs: %lu", numDescs);
    for (size_t i=0; i < numDescs; i++) 
        printf ("%lu) %s lvl %u\n", i, list[i]->name, list[i]->level);
    releaseAllLogLevelsList(list);
    printf("\nDone\n");
    return 0;
}


