#include "access.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

static CardRecord g_cards[MAX_CARDS];
static int g_ncards;
static uint8_t g_failed;
static bool g_lock_open;
static uint32_t g_lock_until;
static bool g_lockdown;
static uint32_t g_lockdown_until;
static bool g_enroll_mode;
static uint32_t g_enroll_until;
static char g_last_uid[16];
static uint32_t g_last_uid_ms;

static void seed_cards(void) {
    g_ncards = 0;
#define ADD(u, n, r) do { \
    snprintf(g_cards[g_ncards].uid, sizeof(g_cards[g_ncards].uid), "%s", u); \
    snprintf(g_cards[g_ncards].name, sizeof(g_cards[g_ncards].name), "%s", n); \
    g_cards[g_ncards].role = (r); \
    g_cards[g_ncards].active = true; \
    g_cards[g_ncards].access_count = 0; \
    g_ncards++; \
} while (0)
    ADD("MASTER01", "Admin", CARD_MASTER);
    ADD("A1B2C3D4", "Ahmed", CARD_USER);
    ADD("E5F6A7B8", "Sara", CARD_USER);
    ADD("11223344", "Youssef", CARD_USER);
#undef ADD
}

void access_init(void) {
    access_reset();
    seed_cards();
}

void access_reset(void) {
    g_failed = 0;
    g_lock_open = false;
    g_lock_until = 0;
    g_lockdown = false;
    g_lockdown_until = 0;
    g_enroll_mode = false;
    g_enroll_until = 0;
    g_last_uid[0] = '\0';
    g_last_uid_ms = 0;
}

CardRecord *access_cards(int *count) {
    if (count) *count = g_ncards;
    return g_cards;
}

const char *rfid_state_name(RfidState s) {
    switch (s) {
    case RFID_IDLE: return "IDLE";
    case RFID_READING: return "READING";
    case RFID_GRANTED: return "GRANTED";
    case RFID_DENIED: return "DENIED";
    case RFID_LOCK_OPEN: return "LOCK_OPEN";
    case RFID_ENROLL: return "ENROLL";
    case RFID_TAMPER: return "TAMPER";
    case RFID_LOCKDOWN: return "LOCKDOWN";
    default: return "?";
    }
}

const char *card_role_name(CardRole r) {
    switch (r) {
    case CARD_USER: return "USER";
    case CARD_MASTER: return "MASTER";
    case CARD_DISABLED: return "DISABLED";
    default: return "?";
    }
}

static int find_card(const char *uid) {
    for (int i = 0; i < g_ncards; i++)
        if (!strcmp(g_cards[i].uid, uid)) return i;
    return -1;
}

static bool enroll_card(const char *uid) {
    if (g_ncards >= MAX_CARDS) return false;
    if (find_card(uid) >= 0) return false;
    snprintf(g_cards[g_ncards].uid, sizeof(g_cards[g_ncards].uid), "%s", uid);
    snprintf(g_cards[g_ncards].name, sizeof(g_cards[g_ncards].name), "User%u",
             (unsigned)g_ncards);
    g_cards[g_ncards].role = CARD_USER;
    g_cards[g_ncards].active = true;
    g_cards[g_ncards].access_count = 0;
    g_ncards++;
    return true;
}

static void open_lock(uint32_t now) {
    g_lock_open = true;
    g_lock_until = now + LOCK_OPEN_MS;
    g_failed = 0;
}

AccessStatus access_update(const SensorSample *s) {
    AccessStatus st;
    memset(&st, 0, sizeof(st));

    RfidState state = RFID_IDLE;
    AlertLevel alert = ALERT_NONE;
    int score = 100;
    int8_t matched = -1;

    if (g_lockdown) {
        if (s->timestamp_ms >= g_lockdown_until) {
            g_lockdown = false;
            g_failed = 0;
        } else {
            state = RFID_LOCKDOWN;
            alert = ALERT_ALARM;
            score -= 40;
        }
    }

    if (g_enroll_mode && s->timestamp_ms > g_enroll_until)
        g_enroll_mode = false;

    /* Effraction */
    if (s->tamper || (s->door_ajar && !g_lock_open)) {
        state = RFID_TAMPER;
        alert = ALERT_ALARM;
        score -= 50;
    }

    /* Activer enroll après badge MASTER (même si serrure encore ouverte) */
    if (!g_lockdown && state != RFID_TAMPER && s->enroll_btn) {
        int last = find_card(g_last_uid);
        if (last >= 0 && g_cards[last].role == CARD_MASTER &&
            (s->timestamp_ms - g_last_uid_ms) < 5000) {
            g_enroll_mode = true;
            g_enroll_until = s->timestamp_ms + 8000;
            state = RFID_ENROLL;
            if (alert < ALERT_NOTIFY) alert = ALERT_NOTIFY;
        }
    }

    if (!g_lockdown && state != RFID_TAMPER && (!g_lock_open || g_enroll_mode)) {
        if (s->card_present && s->uid[0]) {
            bool same = (strcmp(g_last_uid, s->uid) == 0) &&
                        (s->timestamp_ms - g_last_uid_ms) < 1200;
            if (!same) {
                snprintf(g_last_uid, sizeof(g_last_uid), "%s", s->uid);
                g_last_uid_ms = s->timestamp_ms;
                state = RFID_READING;

                int idx = find_card(s->uid);

                if (g_enroll_mode) {
                    state = RFID_ENROLL;
                    if (idx >= 0) {
                        alert = ALERT_NOTIFY; /* déjà connu */
                    } else if (enroll_card(s->uid)) {
                        alert = ALERT_BEEP;
                        matched = (int8_t)(g_ncards - 1);
                    } else {
                        alert = ALERT_NOTIFY;
                    }
                    g_enroll_mode = false;
                } else if (g_lock_open) {
                    /* ignorer badges pendant ouverture (sauf enroll ci-dessus) */
                } else if (idx < 0 || !g_cards[idx].active ||
                           g_cards[idx].role == CARD_DISABLED) {
                    g_failed++;
                    state = RFID_DENIED;
                    alert = ALERT_NOTIFY;
                    score -= 15;
                    if (g_failed >= MAX_FAILED) {
                        g_lockdown = true;
                        g_lockdown_until = s->timestamp_ms + LOCKDOWN_MS;
                        state = RFID_LOCKDOWN;
                        alert = ALERT_ALARM;
                    }
                } else {
                    matched = (int8_t)idx;
                    g_cards[idx].access_count++;

                    if (g_cards[idx].role == CARD_MASTER && s->enroll_btn) {
                        g_enroll_mode = true;
                        g_enroll_until = s->timestamp_ms + 8000;
                        state = RFID_ENROLL;
                        alert = ALERT_NOTIFY;
                    } else {
                        open_lock(s->timestamp_ms);
                        state = RFID_GRANTED;
                        alert = ALERT_BEEP;
                    }
                }
            }
        }
    }

    if (g_lock_open) {
        if (s->timestamp_ms >= g_lock_until)
            g_lock_open = false;
        else if (!g_enroll_mode)
            state = RFID_LOCK_OPEN;
    }

    if (g_enroll_mode)
        state = RFID_ENROLL;

    if (score < 0) score = 0;
    if (score > 100) score = 100;

    st.state = state;
    st.alert = alert;
    st.security_score = (uint8_t)score;
    st.matched_idx = matched;
    st.failed_attempts = g_failed;
    st.lock_open = g_lock_open;
    st.enrolled_count = (uint8_t)g_ncards;
    return st;
}
