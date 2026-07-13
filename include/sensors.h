#ifndef SENSORS_H
#define SENSORS_H

#include "types.h"

typedef enum {
    SCENARIO_VALID = 0,
    SCENARIO_DENIED,
    SCENARIO_ENROLL,
    SCENARIO_TAMPER,
    SCENARIO_MASTER
} SimScenario;

void sensors_init(SimScenario scenario);
void sensors_set_scenario(SimScenario scenario);
SensorSample sensors_read(uint32_t now_ms);
const char *scenario_name(SimScenario s);

#endif
