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

#undef MACRO_GENERATION

#endif /* SampleModule_h */
