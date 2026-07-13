#ifndef ACCESS_H
#define ACCESS_H

#include "types.h"

void access_init(void);
void access_reset(void);
AccessStatus access_update(const SensorSample *s);
CardRecord *access_cards(int *count);
const char *rfid_state_name(RfidState s);
const char *card_role_name(CardRole r);

#endif
