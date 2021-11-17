/*
 * samplemodule.h
 *
 *  Created on: Nov. 12, 2021
 *      Author: bryangoring
 */

#ifndef SampleModule_h
#define SampleModule_h

// Declaration of the space of this module's logs
#include "support/BRLog.h"
LOG_DECLARE_MODULE(SAMPLE);
LOG_DECLARE_SUBMODULE(SAMPLE,SUBA);
LOG_DECLARE_SUBMODULE(SAMPLE,SUBB);
LOG_DECLARE_SUBMODULE(SAMPLE,SUBC);

// Would normally be part of another separate module of the code...
LOG_DECLARE_MODULE(MODULAR);
LOG_DECLARE_SUBMODULE(MODULAR,ONE);
LOG_DECLARE_SUBMODULE(MODULAR,TWO);
LOG_DECLARE_SUBMODULE(MODULAR,THREE);
LOG_DECLARE_SUBMODULE(MODULAR,FOUR);

#undef MACRO_GENERATION

#endif /* SampleModule_h */
