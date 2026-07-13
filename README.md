# RfidAccess-C — Contrôle d'accès RFID (embarqué C)

Projet **embarqué en C**, dédié au badge RFID (complément de [biometric-access-c](https://github.com/GhassenEl/biometric-access-c)).

## Fonctions

| Module | Rôle |
|--------|------|
| Lecteur RFID | Lecture UID (MIFARE / émulation) |
| Whitelist | Cartes USER / MASTER / DISABLED |
| Serrure | Ouverture temporisée |
| Enroll | Master + bouton → nouvelle carte |
| Lockdown | Après 3 badges invalides |
| Anti-effraction | Porte / boîtier forcé |
| Compteur accès | Par carte |

## Cartes démo

| UID | Nom | Rôle |
|-----|-----|------|
| MASTER01 | Admin | MASTER |
| A1B2C3D4 | Ahmed | USER |
| E5F6A7B8 | Sara | USER |
| 11223344 | Youssef | USER |

## Scénarios

- `valid` — badge Ahmed → serrure ouverte  
- `denied` — UIDs inconnus → lockdown  
- `enroll` — master + bouton + nouvelle carte  
- `tamper` — porte forcée  
- `master` — badge admin  

## Lancer (Windows)

```bat
cd mini-projects\embedded\rfid-access-c
build.bat demo
build.bat sim valid 10
build.bat sim enroll 12
build.bat cards
```

Portable STM32/ESP32 : RC522 / PN532 + relais serrure + contacteur porte.
