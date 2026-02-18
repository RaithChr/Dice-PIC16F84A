# Elektronischer Würfel — PIC16F84A

## Hardware: Minimum-Edition (v2.0)

### Stückliste (14 Bauteile)

| Ref     | Wert       | Beschreibung                        |
|---------|------------|-------------------------------------|
| U1      | PIC16F84A  | Mikrocontroller, DIP-18             |
| C1      | 10 µF/10V  | VDD-Bypass, Elko                    |
| R1      | 10 kΩ      | MCLR Pull-Up (unvermeidbar)         |
| R2      | **4,7 kΩ** | RC-Oszillator (OSC1 → VDD) ⚠️ mind. 3kΩ lt. DS40001440E |
| C2      | 100 pF     | RC-Oszillator (OSC1 → GND)          |
| LED1–7  | Clear/red  | Würfel-Anzeige                      |
| R3–R9   | 270 Ω      | LED-Vorwiderstände (4,5 V Betrieb)  |
| SW1     | Drucktaster| Würfeln                             |
| BT1     | 3× AA      | 4,5 V Versorgung, direkt            |

### Was Software ersetzt

| v1.3 Hardware     | v2.0 Ersatz                        |
|-------------------|------------------------------------|
| 7805 + 3 Kondensatoren | 3×AA direkt an VDD (kein Regler!)   |
| 4 MHz Quarz       | `#pragma config FOSC = RC`         |
| 2× 22 pF (Quarz)  | entfallen (RC braucht das nicht)   |
| Taster-Pullup R   | `OPTION_REG /RBPU = 0` (intern!)   |
| Hardware-Entprellung | Software (25 ms Fenster)        |

### RC-Oszillator Verdrahtung

```
VDD ─── R2(4,7kΩ) ─── OSC1 ─── C2(100pF) ─── GND
                        │
                       PIC pin 16
       OSC2 (pin 15) → N/C (gibt Fosc/4 aus im RC-Modus)
```

Frequenz-Formel (DS40001440E, Table 14-2):
```
f ≈ 1 / (3 × REXT × CEXT)

R2=4,7kΩ + C2=100pF → f ≈  709 kHz  ← empfohlen (sicher)
R2=3,3kΩ + C2=100pF → f ≈ 1,01 MHz  (knapp über Minimum 3kΩ)
R2=10kΩ  + C2=100pF → f ≈  333 kHz  (sehr stabil)
```

⚠️ **_XTAL_FREQ im Code muss zur gewählten RC-Frequenz passen!**
- 4,7kΩ → `#define _XTAL_FREQ 700000UL`
- 3,3kΩ → `#define _XTAL_FREQ 1000000UL`
- 10kΩ  → `#define _XTAL_FREQ 330000UL`

### Pin-Belegung

```
PIC16F84A DIP-18
                 ┌─────────────────┐
 RA2 (N/C)  1 ──┤                 ├── 18  RA1 (N/C)
 RA3 (N/C)  2 ──┤                 ├── 17  RA0 (N/C)
 RA4 (N/C)  3 ──┤                 ├── 16  OSC1 ← R2+C2
 MCLR       4 ──┤   PIC16F84A     ├── 15  OSC2 (N/C)
 GND        5 ──┤                 ├── 14  VDD  +4,5V
 RB0 → LED a  6 ┤                 ├── 13  RB7 ← SW1 (intern Pull-Up)
 RB1 → LED b  7 ┤                 ├── 12  RB6 → LED g
 RB2 → LED c  8 ┤                 ├── 11  RB5 → LED f
 RB3 → LED d  9 ┤                 ├── 10  RB4 → LED e
                 └─────────────────┘
```

### LED-Würfelmuster

```
 [ a ] [ b ]      RB0  RB1
 [ c ][d][ e ]    RB2  RB3  RB4
 [ f ] [ g ]      RB5       RB6

Zahl   LEDs an          PORTB (hex)
  1    d                0x08
  2    b f              0x22
  3    b d f            0x2A
  4    a b f g          0x63
  5    a b d f g        0x6B
  6    a b c e f g      0x77
```

### Kompilieren (MPLAB-X)

1. Neues Projekt → PIC16F84A → XC8
2. `main.c` hinzufügen
3. Optimization Level: 1 (oder s)
4. Build → Hex → PICkit 3/4 flashen

### Zufallsprinzip

Timer0 läuft seit dem Einschalten frei mit ~15 Hz Overflow durch.
Beim Tastendruck wird `tmr0_count % 6` abgelesen.
Da die menschliche Reaktionszeit im Millisekunden-Bereich variiert,
ist das Ergebnis für ein Spielzeug vollkommen zufällig genug.
