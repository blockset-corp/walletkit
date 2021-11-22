/*
 * module.c
 *
 *  Created on: Nov. 12, 2021
 *      Author: bryangoring
 */

// Acts in the capacity of a module registering 
// the domain of all module logs...Only the module file that will
// LOG_REGISTER_MODULE need to include the module's log
// definition file with 'BR_LOG_MACRO_GENERATION'. This should be done once
// ...somewhere for each module. This represents the
// runtime aspect of the module logging setup

#define BR_LOG_MACRO_GENERATION
#include "module_logs.h"

void initModule() {
    LOG_REGISTER_MODULE(SAMPLE);
    LOG_ADD_SUBMODULE(SAMPLE, SUBA);
    LOG_ADD_SUBMODULE(SAMPLE, SUBB);
    LOG_ADD_SUBMODULE(SAMPLE, SUBC);
    

    // Would normally be part of another module...
    LOG_REGISTER_MODULE(MODULAR);
    LOG_ADD_SUBMODULE(MODULAR,ONE);
    LOG_ADD_SUBMODULE(MODULAR,TWO);
    LOG_ADD_SUBMODULE(MODULAR,THREE);
    LOG_ADD_SUBMODULE(MODULAR,FOUR);
}

