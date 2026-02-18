/*
 * ═══════════════════════════════════════════════════════════════════════
 *  Elektronischer Würfel  ·  PIC16F84A  ·  Minimum-Hardware Edition
 * ═══════════════════════════════════════════════════════════════════════
 *
 *  Compiler:     MPLAB XC8 v2.x
 *  Ziel-IC:      PIC16F84A (DIP-18)
 *  Takt:         RC-Modus  ~4 MHz  (KEIN Quarz, KEIN 22pF!)
 *  Versorgung:   3× AA  =  4,5 V  direkt an VDD  (KEIN 7805!)
 *
 * ─── PIN-BELEGUNG ────────────────────────────────────────────────────
 *
 *  RB0 → LED a  (oben  links)
 *  RB1 → LED b  (oben  rechts)
 *  RB2 → LED c  (mitte links)
 *  RB3 → LED d  (mitte center) ← Auge für Würfelzahlen 1,3,5
 *  RB4 → LED e  (mitte rechts)
 *  RB5 → LED f  (unten links)
 *  RB6 → LED g  (unten rechts)
 *  RB7 ← TASTER (aktiv LOW  →  INTERNER Pull-Up via OPTION_REG!)
 *
 *  Würfel-Layout:
 *    [ a ]     [ b ]
 *    [ c ] [d] [ e ]
 *    [ f ]     [ g ]
 *
 * ─── HARDWARE-EINSPARUNGEN vs. v1.3 ─────────────────────────────────
 *
 *  ENTFERNT    → GELÖST MIT:
 *  7805         → 3×AA direkt (PIC läuft 2–5,5 V)
 *  Quarz        → RC-Modus  #pragma config FOSC = RC
 *  2× 22pF      → RC-Modus  (obige Zeile reicht)
 *  Taster-R     → OPTION_REG /RBPU = 0  (interne Pull-Ups RB7)
 *  HW-Entprell  → Software-Entprellung (25 ms + Flanke)
 *  MCLR-Button  → WDT-Reset optional / nur MCLR-Pullup-R bleibt
 *
 *  MINIMALES BOM (absolut):
 *   U1   PIC16F84A      ×1
 *   C1   10 µF / 10 V   ×1   (VDD-Bypass)
 *   R1   10 kΩ          ×1   (MCLR Pull-Up, unvermeidbar)
 *   R2   4,7 kΩ         ×1   (RC-Oszillator OSC1↔VDD) ← sicherer als 3,3kΩ!
 *   C2   100 pF         ×1   (RC-Oszillator OSC1↔GND)
 *        → f ≈ 1/(3×4700×100e-12) ≈ 709 kHz  → _XTAL_FREQ = 700000UL anpassen
 *   LED1–LED7           ×7
 *   R3–R9  270 Ω        ×7   (LED-Vorwiderstände für 4,5 V)
 *   SW1  Drucktaster    ×1
 *   BT1  3×AA Halter    ×1
 *   ─────────────────────────
 *   TOTAL  ≈ 14 Bauteile  (vs. ~24 in v1.3)
 *
 * ═══════════════════════════════════════════════════════════════════════
 */

// ─── CONFIG BITS ─────────────────────────────────────────────────────
#pragma config FOSC  = RC     // RC-Oszillator  → kein Quarz!
#pragma config WDTE  = OFF    // Watchdog Timer aus
#pragma config PWRTE = ON     // Power-Up Timer an  (stabiler Start)
#pragma config CP    = OFF    // Kein Code-Schutz

#include <xc.h>
#include <stdint.h>

// RC-Oszillator Frequenz (Datasheet DS40001440E, Table 14-2):
// Formel: f ≈ 1 / (3 × REXT × CEXT)
//
//  R2=3,3kΩ + C2=100pF → f ≈ 1,01 MHz  (REXT an Minimum-Grenze! 3kΩ min lt. DS)
//  R2=4,7kΩ + C2=100pF → f ≈  709 kHz  (empfohlen: sicherer Abstand!)
//  R2=10kΩ  + C2=100pF → f ≈  333 kHz  (sicherste Option)
//
// WICHTIG: _XTAL_FREQ MUSS zur tatsächlichen RC-Frequenz passen,
//          sonst sind alle __delay_ms() Werte falsch!
#define _XTAL_FREQ  700000UL    // ~700 kHz (RC: 4,7kΩ + 100pF per DS40001440E)
                                // Bei 3,3kΩ: 1000000UL verwenden

// ─── LED-Bit-Definitionen ─────────────────────────────────────────────
#define A  (1u<<0)   // RB0
#define B  (1u<<1)   // RB1
#define C  (1u<<2)   // RB2
#define D  (1u<<3)   // RB3 – Zentrum
#define E  (1u<<4)   // RB4
#define F  (1u<<5)   // RB5
#define G  (1u<<6)   // RB6

// ─── Würfelmuster ─────────────────────────────────────────────────────
//   Index 0 = Würfelzahl 1, Index 5 = Würfelzahl 6
const uint8_t DICE[6] = {
    D,                  // ⚀  1: Zentrum
    B|F,                // ⚁  2: oben-rechts + unten-links
    B|D|F,              // ⚂  3: Diagonale + Mitte
    A|B|F|G,            // ⚃  4: vier Ecken
    A|B|D|F|G,          // ⚄  5: vier Ecken + Mitte
    A|B|C|E|F|G,        // ⚅  6: alle außer Zentrum
};

// ─── Zufallsquelle ────────────────────────────────────────────────────
// TMR0 läuft seit Power-On frei durch.
// Beim Tastendruck lesen wir den aktuellen Zählerstand →
// Die menschliche Reaktionszeit variiert im µs-Bereich → effektiv zufällig.
volatile uint8_t tmr0_count = 0;

void __interrupt() isr(void) {
    if (T0IF) {
        tmr0_count++;   // ~15 Hz Overflow @ 4 MHz / 4 / 256
        T0IF = 0;
    }
}

// ─── Variable Delay (XC8: __delay_ms() braucht Konstante) ────────────
static void delay_ms(uint16_t ms) {
    while (ms--) __delay_ms(1);
}

// ─── Taster-Polling: Software-Entprellung + Flanken-Erkennung ────────
static uint8_t button_pressed(void) {
    if (!(PORTB & (1u<<7))) {           // RB7 = LOW → Taster gedrückt
        delay_ms(25);                   // Entprell-Fenster
        if (!(PORTB & (1u<<7))) {       // Bestätigung nach 25 ms
            while (!(PORTB & (1u<<7))); // Warten bis Taster losgelassen
            delay_ms(15);               // Nachprellen abwarten
            return 1;
        }
    }
    return 0;
}

// ─── Animationshelfer: ein Frame ausgeben ────────────────────────────
static void show(uint8_t pattern) {
    PORTB = (PORTB & 0x80u) | (pattern & 0x7Fu);
    // RB7 (Taster-Eingang) bleibt unverändert!
}

// ─── Würfel-Roll-Animation ────────────────────────────────────────────
// Simuliert rollendes Würfelgefühl: schnell → langsam
static void roll_animation(uint8_t final_idx) {
    uint8_t  i;
    uint16_t pause;

    for (i = 0; i < 24u; i++) {
        show(DICE[(tmr0_count + i) % 6u]);

        if      (i <  8u) pause = 40;   // Phase 1: schnell
        else if (i < 16u) pause = 90;   // Phase 2: mittel
        else               pause = 160;  // Phase 3: langsam

        delay_ms(pause);
    }
    // Finales Ergebnis einblenden
    show(DICE[final_idx]);
}

// ─── Ergebnis-Blinken (3× = Würfeln abgeschlossen) ────────────────────
static void blink_result(uint8_t pattern) {
    uint8_t i;
    for (i = 0; i < 3u; i++) {
        show(0x00);     delay_ms(100);
        show(pattern);  delay_ms(200);
    }
}

// ─── Einschalt-Animation (zeigt alle 6 Seiten) ────────────────────────
static void startup_seq(void) {
    uint8_t i;
    for (i = 0; i < 6u; i++) {
        show(DICE[i]);
        delay_ms(140);
    }
    show(DICE[0]);  // Bereit-Anzeige: "1"
    delay_ms(400);
    show(0x00);
    delay_ms(200);
    show(DICE[0]);
}

// ─── MAIN ─────────────────────────────────────────────────────────────
void main(void) {
    uint8_t result;

    // Port-Konfiguration
    TRISA = 0xFFu;           // Port A: alle Eingänge (N/C)
    TRISB = 0x80u;           // RB7 = Eingang (Taster), RB0..6 = Ausgang

    PORTA = 0x00u;
    PORTB = 0x00u;

    // OPTION_REG:
    //  Bit7  /RBPU  = 0  → Interne Pull-Ups AKTIV  (kein Widerstand nötig!)
    //  Bit6  INTEDG = 0  → (egal, INT nicht benutzt)
    //  Bit5  T0CS   = 0  → Timer0 Takt = intern (Fosc/4)
    //  Bit4  T0SE   = 0  → (egal)
    //  Bit3  PSA    = 0  → Prescaler → Timer0
    //  Bit2:0 PS    = 111 → 1:256
    OPTION_REG = 0b00000111;

    // Timer0 Interrupt aktivieren (Zufallsquelle)
    T0IE = 1;
    GIE  = 1;

    // Startup
    startup_seq();

    // ─── Hauptschleife ───────────────────────────────────────────────
    while (1) {
        if (button_pressed()) {
            // Zufallswert: TMR0-Wert im Moment des Tastendrucks
            result = tmr0_count % 6u;

            // Animation + Ergebnis anzeigen
            roll_animation(result);

            // Kurze Pause, dann dreimal blinken
            delay_ms(500);
            blink_result(DICE[result]);
        }
    }
}
