#include "sensors.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

static SimScenario g_sc = SCENARIO_VALID;
static uint32_t g_phase;

void sensors_init(SimScenario scenario) {
    g_sc = scenario;
    g_phase = 0;
}

void sensors_set_scenario(SimScenario scenario) {
    sensors_init(scenario);
}

const char *scenario_name(SimScenario s) {
    switch (s) {
    case SCENARIO_VALID: return "VALID";
    case SCENARIO_DENIED: return "DENIED";
    case SCENARIO_ENROLL: return "ENROLL";
    case SCENARIO_TAMPER: return "TAMPER";
    case SCENARIO_MASTER: return "MASTER";
    default: return "?";
    }
}

SensorSample sensors_read(uint32_t now_ms) {
    SensorSample s;
    memset(&s, 0, sizeof(s));
    g_phase++;
    s.timestamp_ms = now_ms;

    switch (g_sc) {
    case SCENARIO_VALID:
        if (g_phase == 8 || g_phase == 9) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "A1B2C3D4");
        }
        if (g_phase > 12 && g_phase < 28) s.door_ajar = true;
        break;
    case SCENARIO_DENIED:
        if (g_phase == 5) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "DEADBEEF");
        }
        if (g_phase == 15) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "BADCARD01");
        }
        if (g_phase == 25) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "FFFFFFFF");
        }
        break;
    case SCENARIO_ENROLL:
        if (g_phase == 4) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "MASTER01");
        }
        if (g_phase == 6) s.enroll_btn = true;
        if (g_phase == 18 || g_phase == 19) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "NEWUSER99");
        }
        if (g_phase == 40) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "NEWUSER99");
        }
        break;
    case SCENARIO_TAMPER:
        if (g_phase == 6) s.door_ajar = true; /* porte sans badge */
        if (g_phase > 20) s.tamper = true;
        break;
    case SCENARIO_MASTER:
        if (g_phase == 8) {
            s.card_present = true;
            snprintf(s.uid, sizeof(s.uid), "MASTER01");
        }
        break;
    }
    return s;
}
