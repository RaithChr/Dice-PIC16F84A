/*
 * ═══════════════════════════════════════════════════════════════════════
 *  Elektronischer Würfel  ·  PIC16F1825  ·  Absolute Minimum Edition
 * ═══════════════════════════════════════════════════════════════════════
 *
 *  Compiler:     MPLAB XC8 v2.x
 *  Ziel-IC:      PIC16F1825 (DIP-14)  ← NICHT PIC16F84A (DIP-18)!
 *  Takt:         INTERNER Oszillator 4 MHz  (KEIN externer RC, KEIN Quarz!)
 *  Versorgung:   2× AAA = 3,0 V  direkt an VDD (kompatibel zum Original!)
 *
 * ─── WICHTIGE UNTERSCHIEDE PIC16F84A vs. PIC16F1825 ─────────────────
 *
 *  Merkmal          PIC16F84A (DIP-18)    PIC16F1825 (DIP-14)
 *  ─────────────────────────────────────────────────────────────────────
 *  Gehäuse          18-pin DIP            14-pin DIP
 *  Flash            1K Words              8K Words
 *  RAM              68 Bytes              1K Bytes
 *  Ports            Port A + Port B       Port A + Port C
 *  RA3              Eingang/Ausgang       Input-ONLY! (MCLR)
 *  MCLR             immer aktiv           abschaltbar via MCLRE=OFF!
 *  Oszillator       EXTERN (RC/Quarz)     INTERN (bis 16 MHz)!
 *  Pull-Ups         nur Port B (/RBPU)    alle Ports (WPUA, WPUC)
 *  ANSEL Register   KEINS                 vorhanden (analog/digital!)
 *  LAT Register     KEINS                 vorhanden (kein RMW-Problem!)
 *  Timer            Timer0                Timer0 + Timer1 + Timer2
 *  Peripherie       –                     USART, SPI, I2C, PWM, CMP
 *  Spannung         2,0 – 5,5 V           1,8 – 5,5 V
 *  Core             Mid-Range             Enhanced Mid-Range
 *  Max. Takt        20 MHz                32 MHz (mit PLL)
 *
 * ─── HARDWARE-EINSPARUNG vs. PIC16F84A-Design ────────────────────────
 *
 *  Bauteil          16F84A-Design    16F1825-Design
 *  R1  MCLR-Pullup  10 kΩ        →  ENTFÄLLT! (MCLRE = OFF)
 *  R2  RC-Oszi      4,7 kΩ       →  ENTFÄLLT! (interner Oszi)
 *  C2  RC-Oszi      100 pF       →  ENTFÄLLT! (interner Oszi)
 *  ─────────────────────────────────────────────────────────────────────
 *  Gesamt           14 Bauteile  →  11–12 Bauteile ← absolutes Minimum!
 *
 * ─── AUTO-SLEEP FUNKTION ──────────────────────────────────────────────
 *
 *  Nach 10 Sekunden Inaktivität → SLEEP-Modus (~1 µA Stromaufnahme!)
 *  Tastendruck → Wake-Up via Interrupt-on-Change (IOC) auf RA4
 *  Nach Wake-Up: letzte Zahl kurz anzeigen (800ms), dann neu würfeln
 *
 *  Batterie-Laufzeit (2×AAA, 1200mAh):
 *   - Aktiv (6 LEDs):  ~48mA → 25h Dauerbetrieb
 *   - Sleep:           ~1µA  → mehrere JAHRE Standby!
 *   - Normal (10×/Tag): mehrere MONATE
 *
 * ─── PIN-BELEGUNG PIC16F1825 DIP-14 ─────────────────────────────────
 *
 *         ┌────────────────┐
 *  VDD  1─┤                ├─14 VSS (GND)
 *  RA5  2─┤  PIC16F1825    ├─13 RA0
 *  RA4  3─┤   (DIP-14)     ├─12 RA1
 *  RA3  4─┤  MCLR=OFF→N/C  ├─11 RA2
 *  RC5  5─┤                ├─10 RC0
 *  RC4  6─┤                ├─ 9 RC1
 *  RC3  7─┤                ├─ 8 RC2
 *         └────────────────┘
 *
 *  RA4 ← TASTER SW1       (WPUA = intern Pull-Up, kein R!)
 *  RC0 → LED a (oben-links)     RC3 → LED d (Zentrum)
 *  RC1 → LED b (oben-rechts)    RC4 → LED e (mitte-rechts)
 *  RC2 → LED c (mitte-links)    RC5 → LED f (unten-links)
 *  RA5 → LED g (unten-rechts)
 *
 *  LED-Layout (wie bei echtem Würfel):
 *    [a=RC0] [b=RC1]
 *    [c=RC2] [d=RC3] [e=RC4]
 *    [f=RC5]         [g=RA5]
 *
 * ─── MINIMALES BOM (ORIGINAL-kompatibel, 3V) ─────────────────────────
 *
 *   U1   PIC16F1825       ×1
 *   C1   100 nF / 50V     ×1   [FIX-9] Keramik! Nah an VDD/VSS-Pins!
 *   C2   3,3 µF / 6,3V    ×1   (Bulk-Puffer, Elko, optional)
 *   LED1–LED7 (rot!)      ×7   ⚠️ NUR rot/gelb/grün! (Vf<2,1V!)
 *   R1–R7  47 Ω           ×7   (LED-Vorwiderstände @ 3V, ~8-10mA)
 *   SW1  Drucktaster      ×1
 *   JP1  Jumper 2-Pin     ×1   (Power On/Off)
 *   BT1  2×AAA Halter     ×1   (3V nominal, 2,2V min)
 *   ────────────────────────────────────────────────────────────────────
 *   TOTAL  11–12 Bauteile ← ORIGINAL-Layout, nur PIC getauscht!
 *
 * ─── BUGFIXES gegenüber Originalversion ──────────────────────────────
 *
 *  [FIX-1] LATx statt PORTx für alle Ausgaben (kein RMW-Problem)
 *  [FIX-2] Atomare 16-Bit-Zugriffe auf sleep_counter (GIE-Klammer)
 *  [FIX-3] IOCIF ist read-only → Zeile entfernt, Kommentar ergänzt
 *  [FIX-4] Timeout in button_pressed() und nach Wake-Up
 *  [FIX-5] Makro-Namen LED_x statt A–G (Kollisionssicher)
 *  [FIX-6] _XTAL_FREQ vor #include <xc.h>
 *  [FIX-7] Startup endet jetzt mit LEDs AUS (Stromsparen)
 *  [FIX-8] Unbenutzte Pins RA0–RA2 als Ausgänge (definierter Zustand)
 *  [FIX-9] BOM: 100nF Keramik-C als Bypass empfohlen
 *
 * ═══════════════════════════════════════════════════════════════════════
 */

// ─── [FIX-6] _XTAL_FREQ VOR xc.h definieren ──────────────────────────
// Damit __delay_ms() bei der Makro-Expansion die Frequenz kennt.
#define _XTAL_FREQ  4000000UL

// ─── CONFIG BITS (PIC16F1825) ─────────────────────────────────────────
// CONFIG1
#pragma config FOSC     = INTOSC  // Interner Oszillator ← kein externer Takt!
#pragma config WDTE     = OFF     // Watchdog Timer aus
#pragma config PWRTE    = ON      // Power-Up Timer an
#pragma config MCLRE    = OFF     // RA3/MCLR → digitaler I/O (kein Pullup-R!)
#pragma config CP       = OFF     // Kein Code-Schutz
#pragma config CPD      = OFF     // Kein EEPROM-Schutz
#pragma config BOREN    = ON      // Brown-Out Reset aktiv
#pragma config CLKOUTEN = OFF     // CLKOUT aus (RA4 als I/O verfügbar)
#pragma config IESO     = OFF     // Intern/Extern-Umschaltung aus
#pragma config FCMEN    = OFF     // Fail-Safe Clock Monitor aus

// CONFIG2
#pragma config WRT    = OFF   // Flash Write-Schutz aus
#pragma config PLLEN  = OFF   // PLL aus (4x Multiplikator)
#pragma config STVREN = ON    // Stack-Overflow → Reset
#pragma config BORV   = LO   // Brown-Out Low Trip Point
#pragma config LVP    = OFF   // Low-Voltage Programming aus

#include <xc.h>
#include <stdint.h>

// ─── [FIX-5] LED-Bit-Definitionen (kollisionssichere Namen) ──────────
// Originale Makros A–G können mit Standard-Headern kollidieren!
// Bits 0–5 = LATC (RC0–RC5), Bit 6 = LATA5
#define LED_A  (1u<<0)   // RC0 – oben links
#define LED_B  (1u<<1)   // RC1 – oben rechts
#define LED_C  (1u<<2)   // RC2 – mitte links
#define LED_D  (1u<<3)   // RC3 – Zentrum (Auge)
#define LED_E  (1u<<4)   // RC4 – mitte rechts
#define LED_F  (1u<<5)   // RC5 – unten links
#define LED_G  (1u<<6)   // RA5 – unten rechts  ← anderer Port!

// ─── Würfelmuster ─────────────────────────────────────────────────────
//  LED-Layout:     Würfel-Augen:
//  [A] [B]         ● ●
//  [C] [D] [E]     ● ● ●
//  [F]     [G]     ●   ●
static const uint8_t DICE[6] = {
    LED_D,                                           // ⚀  1: Zentrum
    LED_B | LED_F,                                   // ⚁  2: oben-rechts + unten-links
    LED_B | LED_D | LED_F,                           // ⚂  3: Diagonale + Mitte
    LED_A | LED_B | LED_F | LED_G,                   // ⚃  4: vier Ecken
    LED_A | LED_B | LED_D | LED_F | LED_G,           // ⚄  5: Ecken + Mitte
    LED_A | LED_B | LED_C | LED_E | LED_F | LED_G,   // ⚅  6: alle außer Mitte
};

// ─── Zufallsquelle + Sleep-Timer ──────────────────────────────────────
volatile uint8_t  tmr0_count   = 0;
volatile uint16_t sleep_counter = 0;   // Zählt TMR0-Interrupts bis Sleep

// Timer0 @ 4MHz/4 mit Prescaler 1:256 → ~61 Hz Overflow
// 10 Sekunden = ~610 Interrupts
#define SLEEP_TIMEOUT  610u

// ─── [FIX-4] Timeout-Konstante für Taster-Warteschleifen ─────────────
// Verhindert Endlosschleife bei verklemmtem/falsch verlötetem Taster.
// 60000 × ca. 1 µs Schleifenzeit ≈ 60–120 ms max. Wartezeit
#define BUTTON_TIMEOUT  60000u

// ─── [FIX-2] Atomarer 16-Bit-Lesezugriff ─────────────────────────────
// Auf einem 8-Bit-PIC wird uint16_t in ZWEI Schritten gelesen.
// Die ISR kann genau dazwischen feuern → inkonsistenter Wert!
static uint16_t read_sleep_counter(void) {
    uint16_t val;
    INTCONbits.GIE = 0;   // Interrupts sperren
    val = sleep_counter;  // Atomar (kein ISR dazwischen)
    INTCONbits.GIE = 1;   // Interrupts freigeben
    return val;
}

// ─── [FIX-2] Atomarer 16-Bit-Schreibzugriff ──────────────────────────
static void reset_sleep_counter(void) {
    INTCONbits.GIE = 0;
    sleep_counter = 0;
    INTCONbits.GIE = 1;
}

// ─── Interrupt Service Routine ────────────────────────────────────────
void __interrupt() isr(void) {
    // Timer0: Zufallsquelle + Sleep-Counter
    if (INTCONbits.TMR0IF) {
        tmr0_count++;
        sleep_counter++;
        INTCONbits.TMR0IF = 0;
    }
    // Interrupt-on-Change RA4: Wake-Up vom Sleep
    // [FIX-3] IOCIF ist READ-ONLY auf PIC16F1825!
    // Es wird automatisch gelöscht, sobald alle IOCxFy-Flags = 0.
    // Manuelles Löschen von IOCIF hat KEINE Wirkung → nur IOCAF4 löschen!
    if (INTCONbits.IOCIF) {
        if (IOCAFbits.IOCAF4) {        // RA4 hat Flanke ausgelöst
            IOCAFbits.IOCAF4 = 0;     // Sub-Flag löschen → IOCIF geht auto auf 0
            sleep_counter = 0;        // Sleep-Timer zurücksetzen (in ISR atomar OK)
        }
    }
}

// ─── Variable Delay ───────────────────────────────────────────────────
static void delay_ms(uint16_t ms) {
    while (ms--) __delay_ms(1);
}

// ─── [FIX-1] LED-Ausgabe: LATx statt PORTx (kein RMW!) ───────────────
// Der PIC16F1825 hat LAT-Register → immer LATx für Ausgaben verwenden!
// PORTx schreibt über Read-Modify-Write und kann bei schnellen
// aufeinanderfolgenden Zugriffen falsche Pin-Zustände erzeugen.
// (PIC16F84A hatte KEINE LAT-Register — dort gab es das RMW-Problem!)
static void show(uint8_t pattern) {
    LATC = pattern & 0x3Fu;                    // RC0–RC5 via LATC
    LATAbits.LATA5 = (pattern >> 6u) & 1u;    // RA5 via LATA5
}

// ─── [FIX-4] Taster auf RA4: mit Timeout-Absicherung ─────────────────
static uint8_t button_pressed(void) {
    uint16_t timeout;
    if (!PORTAbits.RA4) {            // Erste Abfrage: gedrückt?
        delay_ms(25);                // Entprellung (25 ms)
        if (!PORTAbits.RA4) {        // Immer noch gedrückt?
            timeout = BUTTON_TIMEOUT;
            while (!PORTAbits.RA4 && --timeout);
            delay_ms(15);            // Loslassen entprellen
            return 1;
        }
    }
    return 0;
}

// ─── Würfel-Roll-Animation ────────────────────────────────────────────
static void roll_animation(uint8_t final_idx) {
    uint8_t  i;
    uint16_t pause;

    for (i = 0; i < 24u; i++) {
        show(DICE[(tmr0_count + i) % 6u]);
        if      (i <  8u) pause = 40;    // Schnell (erste 8 Bilder)
        else if (i < 16u) pause = 90;    // Mittel
        else               pause = 160;  // Langsam (letzte 8)
        delay_ms(pause);
    }
    show(DICE[final_idx]);   // Endergebnis anzeigen
}

// ─── Ergebnis-Blinken ─────────────────────────────────────────────────
static void blink_result(uint8_t pattern) {
    uint8_t i;
    for (i = 0; i < 3u; i++) {
        show(0x00);     delay_ms(100);  // Aus
        show(pattern);  delay_ms(200);  // An
    }
}

// ─── [FIX-7] Startup endet mit LEDs AUS ──────────────────────────────
// Original ließ DICE[0] dauerhaft leuchten → unnötiger Stromverbrauch
// bis zum ersten Tastendruck.
static void startup_seq(void) {
    uint8_t i;
    for (i = 0; i < 6u; i++) { show(DICE[i]); delay_ms(140); }
    show(DICE[0]); delay_ms(400);
    show(0x00);    delay_ms(200);   // LEDs aus → bereit
}

// ─── Sleep-Modus aktivieren ───────────────────────────────────────────
static void enter_sleep(void) {
    show(0x00);                      // LEDs aus

    // Interrupt-on-Change für RA4 aktivieren (negative Flanke = Tastendruck)
    IOCAPbits.IOCAP4 = 0;            // Positive Edge disabled
    IOCANbits.IOCAN4 = 1;            // Negative Edge enabled (Taster drücken)
    IOCAFbits.IOCAF4 = 0;            // Altes Flag löschen VOR Aktivierung!
    INTCONbits.IOCIE = 1;            // IOC Interrupt aktivieren

    // Sleep-Counter zurücksetzen
    sleep_counter = 0;               // Kein Interrupt-Risiko (schlafen gleich)

    SLEEP();                         // PIC schlafen legen (~1 µA)
    NOP();                           // Nach Wake-Up hier weitermachen

    // Nach Wake-Up: IOC deaktivieren (nur TMR0 + Button-Polling)
    INTCONbits.IOCIE = 0;
    IOCANbits.IOCAN4 = 0;
}

// ─── MAIN ─────────────────────────────────────────────────────────────
void main(void) {
    uint8_t  result;
    uint8_t  last_result = 0;   // Letzte gewürfelte Zahl merken
    uint16_t timeout;           // [FIX-4] Für Wake-Up-Taster-Timeout

    // ── Interner Oszillator auf 4 MHz ─────────────────────────────────
    // OSCCON: IRCF<3:0> = 1101 → 4 MHz (Datasheet DS41440E, Table 5-1)
    OSCCONbits.IRCF = 0b1101;   // 4 MHz
    OSCCONbits.SCS  = 0b10;     // Interner Oszillator als Taktquelle

    // ── ANALOG-DISABLE: PFLICHT auf PIC16F1825! ───────────────────────
    // Ohne diese Zeilen funktioniert digitale I/O NICHT!
    // (Gibt es NICHT auf PIC16F84A — häufigste Fehlerquelle beim Umstieg!)
    ANSELA = 0x00;   // Port A: alle Pins digital
    ANSELC = 0x00;   // Port C: alle Pins digital

    // ── Port-Konfiguration ────────────────────────────────────────────
    // RA3 = 1 (immer Input-Only auf PIC16F1825, egal was man schreibt)
    // RA4 = 1 (Taster-Eingang)
    // RA5 = 0 (LED g Ausgang)
    // [FIX-8] RA0–RA2 = 0 (Ausgang, LOW) → definierter Zustand!
    TRISA = 0b00011000;
    TRISC = 0b00000000;   // Port C: alle Ausgänge (LEDs a–f)

    // [FIX-1] LAT-Register für Ausgangszustand initialisieren
    LATA = 0x00;
    LATC = 0x00;

    // ── Interne Pull-Ups aktivieren ───────────────────────────────────
    OPTION_REGbits.nWPUEN = 0;   // Pull-Ups global aktivieren
    WPUA = 0b00010000;            // WPUA4 = 1 → Pull-Up auf RA4 (Taster)
    WPUC = 0b00000000;            // Port C: keine Pull-Ups (Ausgänge)

    // ── Timer0: freilaufend für Zufallsgenerator ──────────────────────
    // [FIX-7] Bits 7+6 (nWPUEN + INTEDG) korrekt beibehalten!
    OPTION_REG = (OPTION_REG & 0xC0u) | 0b00000111u;

    INTCONbits.TMR0IE = 1;   // Timer0 Interrupt aktivieren
    INTCONbits.GIE    = 1;   // Global Interrupt Enable

    // ── Startup ───────────────────────────────────────────────────────
    startup_seq();               // [FIX-7] Endet mit LEDs aus
    reset_sleep_counter();       // [FIX-2] Atomarer Reset

    // ── Hauptschleife mit Auto-Sleep ──────────────────────────────────
    while (1) {
        // [FIX-2] Atomarer 16-Bit-Vergleich (sleep_counter ist uint16_t!)
        if (read_sleep_counter() >= SLEEP_TIMEOUT) {
            enter_sleep();

            // Nach Wake-Up: letzte Zahl kurz anzeigen
            show(DICE[last_result]);
            delay_ms(800);

            // [FIX-4] Warten bis Taster losgelassen MIT Timeout
            timeout = BUTTON_TIMEOUT;
            while (!PORTAbits.RA4 && --timeout);
            delay_ms(25);

            reset_sleep_counter();    // [FIX-2] Atomar zurücksetzen
        }

        // Normal: auf Tastendruck warten
        if (button_pressed()) {
            reset_sleep_counter();    // [FIX-2] Activity → Sleep-Timer zurück
            result = tmr0_count % 6u;
            roll_animation(result);
            delay_ms(500);
            blink_result(DICE[result]);
            last_result = result;     // Ergebnis merken für Wake-Up
        }
    }
}
