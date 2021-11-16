/*
 * module_log_user.c
 *
 *  Created on: Nov. 12, 2021
 *      Author: bryangoring
 */

#include "module_logs.h"

void userFunction() {
    const char *err = "error";
            
    LOG(LL_ERROR, SAMPLE, "Some high level module error!");
    LOG(LL_ERROR, SAMPLE, "Another %s", err);
    LOG(LL_ERROR, SAMPLE_SUBA, "Submodule %s", err);
    LOG(LL_ERROR, SAMPLE_SUBB, "Another submodule error");
    LOG(LL_WARN, SAMPLE, "Take heed!");
    LOG(LL_INFO, SAMPLE_SUBC, "An info log");
}


