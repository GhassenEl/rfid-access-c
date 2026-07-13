#ifndef ALERT_H
#define ALERT_H

#include "types.h"

void alert_init(void);
void alert_process(const AccessStatus *st, const SensorSample *s, uint32_t now_ms);
const char *alert_level_name(AlertLevel level);

#endif
