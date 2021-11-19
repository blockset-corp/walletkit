//
//  BRLog.h
//
//  Created by Bryan Goring on 11/10/21.
//  Copyright (c) 2021 breadwallet LLC.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
#ifndef BRLog_h
#define BRLog_h

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

/** Enumeration of possible log levels.
 * 
 *  It should be understood that the heirarchy is both 
 *  indicative of the weight of the messages, but also likely
 *  to foretell the volume of output. For example with LL_DEBUG
 *  enabled, there is likely to be far more information provided (let's hope)
 *  than with say LL_ERROR.
 */
typedef enum {
    
    /**
     * LL_FATAL: A first order disaster. In the wake of a FATAL, the application
     *           cannot continue (may raise assert for example)
     */
    LL_FATAL,
    
    /** 
     * LL_ERROR: Issues that on arising, must be dealt with immediately and 
     *           cannot be ignored. This indicates an outright failure of a central
     *           piece of business logic, and absolutely indicates an impairment
     *           of functionality. An ERROR may be survived in that it indicates serious, 
     *           but potentially localized issue with a specific area of functionality.
     */
    LL_ERROR,
    
    /** 
     * LL_WARN: An unexpected or unforeseen or undesirable condition has been 
     *          observed. The presence of a warning indicates that subsequent
     *          business logic not complete desired, or may not conclude properly
     *          at all. WARN may be used to indicate conditions outside of planned
     *          or predicted outlines, and should be used to call attention to 
     *          situations that should be investigated. A WARN situation need not 
     *          necessarily result in an ERROR.
     */
    LL_WARN,
    
    /**
     * LL_INFO: For reporting high level events or information that would be salient
     *          to reporting the progress of executing the business logic of an application, 
     *          if not the details by which this progress is achieved.
     */
    LL_INFO,
    
    /** 
     * LL_DEBUG: Information that supports understanding of execution of the code towards
     *           the fulfillment of a certain piece of business logic. DEBUG logs should provide
     *           detail pertinent to the execution of code or draw attention to particular 
     *           developer level events that are of importance. DEBUG logs help support 
     *           reconstruction of an understanding of the code or process state after the fact, 
     *           to aid understanding of its execution or for problem investigation. 
     */
    LL_DEBUG,
    
    /** 
     * LL_VERBOSE: Along the lines of DEBUG logs but providing an even clearer/finer illustration
     *             of events and data at runtime.
     */
    LL_VERBOSE
    
} BRLogLevel;
#define MAX_LEVELS          (LL_VERBOSE + 1)
#define DEFAULT_LOG_LEVEL   (LL_INFO)

#define UNSPECIFIED_LOG_MOD     (NULL)
#define MAX_LOG_MOD_NAME_LEN    (16)
#define MAX_LOG_ADDRESS_LEN     (2 * MAX_LOG_MOD_NAME_LEN + 1) // 'mod'_'submod'

typedef struct BRLogModuleStruct {
    BRLogLevel      level;
    char            name[MAX_LOG_ADDRESS_LEN + 1];
} *BRLogModule;

// A 'base' or system-wide log module for using as root
// of all first order modules
extern BRLogModule SYSTEM_LOG;

void registerLogModule  (   const char      *moduleName, 
                            BRLogModule     *module,
                            BRLogModule     *submodule);
void doLog              (   BRLogLevel      lvl, 
                            BRLogModule     mod, 
                            const char*     fmt, ...);

/* Log module and submodule declarations follow:
 * 
 * LOG_DECLARE_MODULE(mod):             creates a log module identifier which 
 *                                      can be passed to LOG(): 'mod'
 * LOG_DECLARE_SUBMODULE(mod,submod):   creates a log submodule identifier within
 *                                      the 'mod' scheme which can be passed to 
 *                                      LOG(): 'mod'_'submod'
 */

#ifdef MACRO_GENERATION

#define LOG_DECLARE_MODULE(mod) \
BRLogModule mod##_LOG

#define LOG_DECLARE_SUBMODULE(mod,submod) \
BRLogModule mod##_##submod##_LOG

#else 

#define LOG_DECLARE_MODULE(mod) \
extern BRLogModule mod##_LOG

#define LOG_DECLARE_SUBMODULE(mod,submod) \
extern BRLogModule mod##_##submod##_LOG

#endif

/*******    Functional Appearance of Logging *******/

// For per-module registration of modules and submodules
#define LOG_REGISTER_MODULE(mod) registerLogModule(#mod, & SYSTEM_LOG, & mod##_LOG)
#define LOG_ADD_SUBMODULE(mod,submod) registerLogModule(#submod, & mod##_LOG, & mod##_##submod##_LOG)

// Enter a log at 'lvl' for either top level module, or module-submodule
// Any non-existant 'mod' being specified will result in compilation error
//
// @param lvl The log level
// @param modOrSubmod The module or submodule name to log to
// @param fmt The log format string
// @param args ... variadac arguments
#define LOG(lvl, modOrSubmod, fmt, ...)                                             \
    assert (NULL != modOrSubmod##_LOG && "Log " #modOrSubmod " not registered!");   \
    assert (lvl < MAX_LEVELS);                                                      \
    if (modOrSubmod##_LOG->level >= lvl)                                            \
        doLog (lvl, modOrSubmod##_LOG, fmt, ##__VA_ARGS__); 

// Set the new log level for any module or module and submodule, or module and all its
// submodules
//
// @param newLevel      The new level to be set to
// @param modName       The parent module name:
//                          - a specific name as declared via LOG_DECLARE_MODULE
//                          - "*" indicating all modules defined with the system
// @param submodName    The child or submodule name OR 
//                          - 'UNSPECIFIED_LOG_MOD' indicating that only the parent changes
//                          - a specific name as declared via LOG_DECLARE_SUBMODULE
//                          - "*" indicating the parent and all its children down to submodules
// @return 0 When the module/submodule specification matched and existing module, < 0 otherwise
int setLogLevel(BRLogLevel newLevel, const char* modName, const char* submodName);

// Reset all system log levels to default (LL_WARN)
void resetAllLogLevels();

// Query current module, submodules and their active log level
// Query returned information can be used to decide which modName and submodName are
// available to 'setLogLevel()' and also indicate the current log levels
size_t  getAllLogLevelsList     (BRLogModule    **list);
void    releaseAllLogLevelsList (BRLogModule    *list);

#endif /* BRLog_h */
