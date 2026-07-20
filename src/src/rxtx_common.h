#pragma once

#include "targets.h"
#include "rxtx_intf.h"
#include "config.h"
#include "FHSS.h"
#include "helpers.h"
#include "hwTimer.h"
#include "logging.h"
#include "LBT.h"
#include "LQCALC.h"
#include "OTA.h"
#include "POWERMGNT.h"
#include "deferred.h"

#include <stdint.h>

void setupTargetCommon();
#if !defined(UNIT_TEST)
bool beginRadioWithRetries(uint32_t minimumFrequency, uint32_t maximumFrequency);
#endif
void checkRebootTime(unsigned long now);
