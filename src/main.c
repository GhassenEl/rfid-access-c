#include "sensors.h"
#include "access.h"
#include "alert.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(unsigned ms) { Sleep(ms); }
#else
#include <unistd.h>
static void sleep_ms(unsigned ms) { usleep(ms * 1000); }
#endif

static void banner(void) {
    printf("====================================================\n");
    printf("  RfidAccess-C — Controle d'acces RFID embarque\n");
    printf("  Badge / serrure / enroll / lockdown\n");
    printf("====================================================\n");
}

static void help(const char *exe) {
    printf("Usage:\n");
    printf("  %s sim [valid|denied|enroll|tamper|master] [seconds]\n", exe);
    printf("  %s cards\n", exe);
    printf("  %s demo\n", exe);
    printf("  %s help\n", exe);
}

static SimScenario parse_sc(const char *s) {
    if (!s) return SCENARIO_VALID;
    if (!strcmp(s, "denied")) return SCENARIO_DENIED;
    if (!strcmp(s, "enroll")) return SCENARIO_ENROLL;
    if (!strcmp(s, "tamper")) return SCENARIO_TAMPER;
    if (!strcmp(s, "master")) return SCENARIO_MASTER;
    return SCENARIO_VALID;
}

static void print_cards(void) {
    int n = 0;
    CardRecord *c = access_cards(&n);
    printf("\n--- Cartes RFID (%d) ---\n", n);
    for (int i = 0; i < n; i++) {
        printf("  %-10s  %-10s  %-8s  acces=%u %s\n",
               c[i].uid, c[i].name, card_role_name(c[i].role),
               c[i].access_count, c[i].active ? "" : "[OFF]");
    }
}

static void run_sim(SimScenario sc, int seconds) {
    uint32_t now = 0;
    int ticks = (seconds * 1000) / TICK_MS;
    int log_every = 1000 / TICK_MS;

    sensors_init(sc);
    access_init();
    alert_init();

    printf("\n[SIM] Scenario=%s duree=%ds\n", scenario_name(sc), seconds);
    printf("----+------------+------+------+------+----------\n");
    printf(" t  | UID        | Fail | Lock |Cards | ETAT\n");
    printf("----+------------+------+------+------+----------\n");

    for (int i = 0; i < ticks; i++) {
        SensorSample s = sensors_read(now);
        AccessStatus st = access_update(&s);
        alert_process(&st, &s, now);

        if (i % log_every == 0) {
            printf("%3us | %-10s | %4u | %-4s | %4u | %s\n",
                   now / 1000,
                   s.card_present ? s.uid : "-",
                   st.failed_attempts,
                   st.lock_open ? "OPEN" : "shut",
                   st.enrolled_count,
                   rfid_state_name(st.state));
        }
        now += TICK_MS;
        if (seconds <= 6) sleep_ms(3);
    }
    printf("----+------------+------+------+------+----------\n");
    print_cards();
    printf("[SIM] Termine.\n");
}

static void run_demo(void) {
    printf("\n=== DEMO: VALID -> DENIED -> ENROLL -> TAMPER ===\n");
    run_sim(SCENARIO_VALID, 10);
    run_sim(SCENARIO_DENIED, 10);
    run_sim(SCENARIO_ENROLL, 12);
    run_sim(SCENARIO_TAMPER, 8);
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));
    banner();

    if (argc < 2 || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h")) {
        help(argv[0]);
        return 0;
    }
    if (!strcmp(argv[1], "cards")) {
        access_init();
        print_cards();
        return 0;
    }
    if (!strcmp(argv[1], "demo")) {
        run_demo();
        return 0;
    }
    if (!strcmp(argv[1], "sim")) {
        SimScenario sc = parse_sc(argc >= 3 ? argv[2] : "valid");
        int seconds = argc >= 4 ? atoi(argv[3]) : 10;
        if (seconds < 3) seconds = 3;
        if (seconds > 120) seconds = 120;
        run_sim(sc, seconds);
        return 0;
    }
    help(argv[0]);
    return 1;
}
