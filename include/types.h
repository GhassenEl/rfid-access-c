#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    RFID_IDLE = 0,
    RFID_READING,
    RFID_GRANTED,
    RFID_DENIED,
    RFID_LOCK_OPEN,
    RFID_ENROLL,
    RFID_TAMPER,
    RFID_LOCKDOWN
} RfidState;

typedef enum {
    ALERT_NONE = 0,
    ALERT_BEEP,
    ALERT_NOTIFY,
    ALERT_ALARM
} AlertLevel;

typedef enum {
    CARD_USER = 0,
    CARD_MASTER,
    CARD_DISABLED
} CardRole;

typedef struct {
    bool card_present;
    char uid[16];
    bool door_ajar;
    bool tamper;
    bool enroll_btn;          /* mode inscription (master) */
    uint32_t timestamp_ms;
} SensorSample;

typedef struct {
    char uid[16];
    char name[16];
    CardRole role;
    bool active;
    uint16_t access_count;
} CardRecord;

typedef struct {
    RfidState state;
    AlertLevel alert;
    uint8_t security_score;
    int8_t matched_idx;       /* -1 si inconnu */
    uint8_t failed_attempts;
    bool lock_open;
    uint8_t enrolled_count;
} AccessStatus;

#endif
