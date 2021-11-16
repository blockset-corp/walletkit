/*
 * BRLog.c
 *
 *  Created on: Nov. 12, 2021
 *      Author: bryangoring
 */
#include "BRLog.h"
#include "BROSCompat.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#define MAX_LEVEL_DESC_LEN  (4)
static const char* levelDescriptions[MAX_LEVELS] = 
{
    "FATL",
    "ERRR",
    "WARN",
    "INFO",
    "DEBG",
    "DBG2"
};

#define MAX_LOG_LEN     (1024) // Or as deemed suitable

struct ModuleInfo {
    struct Module       base;
    
    struct ModuleInfo   *parent;
    char                tag[MAX_LOG_MOD_NAME_LEN+1];
    uint32_t            numModules;
    struct ModuleInfo   **modules;
};

struct Base {
    pthread_mutex_t     log_lock;
    struct ModuleInfo   all;
};

static struct Base systemLogInfo;
LogModule SYSTEM_LOG = (LogModule)&systemLogInfo.all;

static void initLog() {
    pthread_mutex_init_brd(&systemLogInfo.log_lock, PTHREAD_MUTEX_RECURSIVE);
    systemLogInfo.all.numModules = 0;
    systemLogInfo.all.modules = NULL;
    systemLogInfo.all.parent = NULL;
    systemLogInfo.all.base.level = DEFAULT_LOG_LEVEL;
    strcpy(systemLogInfo.all.base.name, "SYSTEM");
    strcpy(systemLogInfo.all.tag, "SYSTEM");
}

static pthread_once_t logInitOnlyOnce = PTHREAD_ONCE_INIT;
void initLogSystem() {
    pthread_once (&logInitOnlyOnce, initLog);
}

void registerLogModule(
    const char  *moduleName,
    LogModule   *ownerModule,
    LogModule   *module  ) {
    
    assert (NULL != ownerModule && NULL != *ownerModule);
    assert (NULL != module);
    assert (MAX_LOG_MOD_NAME_LEN >= strlen(moduleName));
    
    struct ModuleInfo **ownerModuleInfo = (struct ModuleInfo**)ownerModule;
    struct ModuleInfo **moduleInfo = (struct ModuleInfo**)module;
    
    pthread_mutex_lock(&systemLogInfo.log_lock);
    
    if (NULL == *moduleInfo) {
        *moduleInfo = (struct ModuleInfo*) calloc (1, sizeof (struct ModuleInfo));
        assert (NULL != *moduleInfo);
        (*moduleInfo)->base.level = DEFAULT_LOG_LEVEL;
        (*moduleInfo)->parent = *ownerModuleInfo;
    }
    
    // Resize, if needed
    struct ModuleInfo **tmp = (*ownerModuleInfo)->modules;
    (*ownerModuleInfo)->modules = (struct ModuleInfo**) calloc ((*ownerModuleInfo)->numModules + 1, 
                                                                sizeof (struct ModuleInfo*));
    if (NULL != tmp) {
        for (int i=0; i < (*ownerModuleInfo)->numModules; i++) {
            (*ownerModuleInfo)->modules[i] = tmp[i];
        }
    }
    (*ownerModuleInfo)->modules[(*ownerModuleInfo)->numModules++] = *moduleInfo;
    strncpy((*moduleInfo)->tag, moduleName, MAX_LOG_MOD_NAME_LEN);
    printf("Module name \"%s\"\n", (*moduleInfo)->tag);
    
    // Formulate the external name of the log
    char* nm = (*moduleInfo)->base.name;
    if ((*ownerModule) != SYSTEM_LOG) {
        size_t sz = sprintf (nm, "%s_", (*ownerModuleInfo)->tag);
        nm += sz;
    }
    sprintf (nm, "%s", (*moduleInfo)->tag);
    
    if (NULL != tmp)
        free (tmp);
    
    pthread_mutex_unlock(&systemLogInfo.log_lock);
}

// '[hh:mm:ss.SSS][LVL][MOD-MAX_LOG_MOD_NAME_LEN][SUBMOD-MAX_LOG_MOD_NAME_LEN]: '
#define MAX_OCCURS_ON_LEN   ((2+1+2+1+2+1+3)+2)                             // [hh:mm:ss.SSS]
#define MAX_LVL_LOG_LEN     (MAX_LEVEL_DESC_LEN + 2)                        // [lvl]
#define MAX_MOD_LOG_LEN     (MAX_LOG_MOD_NAME_LEN + 2)                      // [submod]
#define MAX_HDR_LEN         ((MAX_OCCURS_ON_LEN + MAX_LVL_LOG_LEN + MAX_MOD_LOG_LEN * 2) + 2) // +2 for ': '
void doLog(
    BRLogLevel      lvl, 
    LogModule       mod, 
    const char*     fmt, ... ) {
    
    assert (NULL != mod);
    
    struct ModuleInfo *modInfo = (struct ModuleInfo*)mod;
    struct ModuleInfo *submodInfo = NULL;
    if (modInfo->parent != (struct ModuleInfo*)SYSTEM_LOG) {
        // A submodule is passed
        submodInfo = modInfo;
        modInfo = submodInfo->parent;
    }
    
    char smod[MAX_MOD_LOG_LEN + 1];
    char occursOn[MAX_OCCURS_ON_LEN + 1];
    char logMsg[MAX_LOG_LEN + 1];
    time_t t; 
    struct tm local;
    struct timeval tv;
    
    t = time(NULL);
    local = *localtime(&t);
    gettimeofday (&tv, NULL);
    snprintf(occursOn, 
             MAX_OCCURS_ON_LEN + 1, 
             "[%02d:%02d:%02d.%03d]",
             local.tm_hour,
             local.tm_min,
             local.tm_sec,
             (tv.tv_usec / 1000));
    
    if ( NULL != submodInfo)
        snprintf (smod, MAX_MOD_LOG_LEN + 1, "[%*s]", MAX_LOG_MOD_NAME_LEN, submodInfo->tag);
    else {
        memset (smod, ' ', MAX_MOD_LOG_LEN);
        smod[MAX_MOD_LOG_LEN] = 0;
    }
    snprintf (logMsg, 
              MAX_HDR_LEN + 1,
              "%s[%s][%*s]%s: ", 
              occursOn, 
              levelDescriptions[lvl],
              MAX_LOG_MOD_NAME_LEN,
              modInfo->tag,
              smod);

    char *p = logMsg + MAX_HDR_LEN;
    
    va_list args;
    va_start (args, fmt);
    int len = vsnprintf (p, MAX_LOG_LEN - MAX_HDR_LEN, fmt, args);
    va_end (args);
    
    if (len > (MAX_LOG_LEN - MAX_HDR_LEN)) {
        // log too large for static buffer, ignore OR heap to help
        // p = malloc ...
        // etc
    } else {
        *(p + len) = 0;
    }
    
    // Per system target type...
    printf("%s\n", logMsg);
}

void setLogLevel(
    BRLogLevel  newLevel,
    const char  *modName,
    const char  *submodName )
{
    
}
