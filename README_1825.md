# Elektronischer Würfel — PIC16F1825 (3V Edition)

## Original-kompatibel: 2×AAA, 11 Bauteile

### Stückliste (ORIGINAL Layout)

| Ref     | Wert         | Beschreibung                               |
|---------|--------------|-------------------------------------------|
| U1      | PIC16F1825   | Mikrocontroller, DIP-14                   |
| C1      | 3,3 µF/6,3V  | VDD-Bypass, Elko                          |
| LED1–7  | **Rot/Gelb** | ⚠️ NUR Vf<2,1V (Weiß/Blau geht NICHT!)   |
| R1–R7   | **47 Ω**     | LED-Vorwiderstände @ 3V (~8-10mA)         |
| SW1     | Drucktaster  | Würfeln                                    |
| JP1     | 2-Pin Jumper | Power On/Off                               |
| BT1     | 2× AAA       | 3V Versorgung (2,2–3,0V)                  |

### Wichtige Unterschiede zu PIC16F84A

| Feature          | PIC16F84A        | PIC16F1825           |
|------------------|------------------|----------------------|
| Gehäuse          | DIP-18           | **DIP-14**           |
| Flash            | 1K Words         | **8K Words**         |
| RAM              | 68 Bytes         | **1K Bytes**         |
| Ports            | Port A + Port B  | Port A + **Port C**  |
| Oszillator       | EXTERN (RC/XTAL) | **INTERN (bis 16 MHz)!** |
| MCLR             | immer aktiv      | **abschaltbar (MCLRE=OFF)** |
| Pull-Ups         | nur Port B       | **alle Ports (WPUA/WPUC)** |
| ANSEL-Register   | –                | **PFLICHT löschen!** |
| Min. Spannung    | 2,0V             | **1,8V** (besser für Low-Batt) |
| Extras           | –                | USART, PWM, CMP, SPI, I2C |

### Pin-Belegung PIC16F1825 (DIP-14)

```
         ┌────────────────┐
  VDD  1─┤                ├─14 VSS (GND)
  RA5  2─┤  PIC16F1825    ├─13 RA0
  RA4  3─┤   (DIP-14)     ├─12 RA1
  RA3  4─┤  MCLR=OFF→N/C  ├─11 RA2
  RC5  5─┤                ├─10 RC0
  RC4  6─┤                ├─ 9 RC1
  RC3  7─┤                ├─ 8 RC2
         └────────────────┘

LED-Zuordnung:
  RC0 → LED a (oben-links)       RC3 → LED d (Zentrum)
  RC1 → LED b (oben-rechts)      RC4 → LED e (mitte-rechts)
  RC2 → LED c (mitte-links)      RC5 → LED f (unten-links)
  RA5 → LED g (unten-rechts)

Taster: RA4  (interner Pull-Up via WPUA, kein R!)
Power:  JP1  zwischen Batterie+ und VDD (Jumper = On)
```

### LED-Muster (Würfel 1-6)

```
Layout:
 [a] [b]        Würfel-Werte:
 [c][d][e]      1 = d              (0x08)
 [f] [g]        2 = b + f          (0x22)
                3 = b + d + f      (0x2A)
                4 = a + b + f + g  (0x63)
                5 = a+b+d+f+g      (0x6B)
                6 = a+b+c+e+f+g    (0x77)
```

### LED-Widerstände @ 3V (2×AAA)

**Berechnung:**
- VDD = 3,0V (frisch), 2,2V (entladen)
- VOH = VDD - 0,7V ≈ 2,3V (bei 8mA Source-Strom)
- VLED(rot) = 1,8–2,0V
- R = (VOH - VLED) / I_target

```
LED-Typ   Vf      R=47Ω    Strom     Empfehlung
─────────────────────────────────────────────────
Rot       1,8V    47Ω      ~10mA     ✅ ideal
Gelb      1,9V    47Ω      ~8mA      ✅ gut
Grün      2,0V    47Ω      ~6mA      ✅ OK (dunkler)
Weiß      3,1V    –        –         ❌ geht NICHT!
Blau      3,0V    –        –         ❌ geht NICHT!
```

⚠️ **Wichtig:** Nur rote, gelbe oder grüne LEDs verwenden!

### Batterie-Laufzeit (mit Auto-Sleep!)

- 2×AAA ≈ 1200 mAh
- Aktiv (6 LEDs): 6 × 8mA = 48mA → ~25h Dauerbetrieb
- **Sleep-Modus: ~1µA** → mehrere **JAHRE** Standby!
- Normalbetrieb (10× würfeln/Tag, Auto-Sleep): **mehrere MONATE**

### Auto-Sleep Feature

- Nach **10 Sekunden** Inaktivität → SLEEP-Modus (~1µA)
- Tastendruck → Wake-Up via Interrupt-on-Change (IOC)
- Nach Wake-Up: **letzte Zahl 800ms anzeigen**, dann neu würfeln
- Stromersparnis: ~99,998% im Standby!

### Kompilieren (MPLAB-X)

1. Neues Projekt → **PIC16F1825** (nicht 16F84A!) → XC8
2. `main_1825.c` hinzufügen
3. Optimization Level: 1 oder s
4. Build → Hex → PICkit 3/4 flashen

### Code-Besonderheiten

1. **ANSELA/ANSELC löschen!** (sonst funktioniert keine digitale I/O)
2. **OSCCON setzen** (`IRCF = 0b1101` für 4 MHz intern)
3. **WPUA aktivieren** für RA4 Pull-Up (kein externer Widerstand!)
4. **Port C statt Port B** (TRISC, PORTC, ANSELC, WPUC)

### Jumper JP1

```
Batterie+ ──┬── JP1 ──┬── VDD (PIC pin 1)
            │         │
        [Jumper]      C1 (3,3µF)
            │         │
        Batterie- ────┴── VSS (PIC pin 14)
```

Jumper **eingesteckt** = Gerät AN  
Jumper **gezogen** = Gerät AUS
