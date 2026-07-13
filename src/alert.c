#include "alert.h"
#include "access.h"
#include <stdio.h>

static AlertLevel g_last = ALERT_NONE;
static uint32_t g_last_ms = 0;

void alert_init(void) {
    g_last = ALERT_NONE;
    g_last_ms = 0;
}

const char *alert_level_name(AlertLevel level) {
    switch (level) {
    case ALERT_NONE: return "NONE";
    case ALERT_BEEP: return "BEEP";
    case ALERT_NOTIFY: return "NOTIFY";
    case ALERT_ALARM: return "ALARM";
    default: return "?";
    }
}

void alert_process(const AccessStatus *st, const SensorSample *s, uint32_t now_ms) {
    if (st->alert == ALERT_NONE) {
        g_last = ALERT_NONE;
        return;
    }
    if (st->alert == g_last && (now_ms - g_last_ms) < 2000) return;
    g_last = st->alert;
    g_last_ms = now_ms;

    int n = 0;
    CardRecord *cards = access_cards(&n);
    const char *name = (st->matched_idx >= 0 && st->matched_idx < n)
                           ? cards[st->matched_idx].name : "?";

    switch (st->alert) {
    case ALERT_BEEP:
        if (st->state == RFID_ENROLL)
            printf("  !! BEEP — nouvelle carte enrolee: %s UID=%s (total=%u)\n",
                   name, s->uid, st->enrolled_count);
        else
            printf("  !! BEEP — acces ACCORDE (%s) UID=%s — serrure OUVERTE\n",
                   name, s->uid[0] ? s->uid : "-");
        break;
    case ALERT_NOTIFY:
        if (st->state == RFID_ENROLL)
            printf("  !! NOTIF — mode ENROLL actif (presenter nouvelle carte)\n");
        else
            printf("  !! NOTIF — acces REFUSE UID=%s (echecs=%u)\n",
                   s->uid[0] ? s->uid : "-", st->failed_attempts);
        break;
    case ALERT_ALARM:
        if (st->state == RFID_TAMPER)
            printf("  !!! ALARME — effraction / porte forcee\n");
        else
            printf("  !!! ALARME — LOCKDOWN RFID (trop de badges invalides)\n");
        break;
    default:
        break;
    }
}
