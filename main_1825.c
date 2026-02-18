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
 *  Gesamt           14 Bauteile  →  11–12 Bauteile  ← absolutes Minimum!
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
 *   C1   100 nF / 50V     ×1   (VDD-Bypass, Keramik! Nah an VDD/VSS!)
 *   C2   3,3 µF / 6,3V    ×1   (Bulk-Puffer, Elko, optional)
 *   LED1–LED7 (rot!)      ×7   ⚠️ NUR rot/gelb/grün! (kein Weiß/Blau!)
 *   R1–R7  47 Ω           ×7   (LED-Vorwiderstände @ 3V, ~8-10mA)
 *   SW1  Drucktaster      ×1
 *   JP1  Jumper 2-Pin     ×1   (Power On/Off)
 *   BT1  2×AAA Halter     ×1   (3V nominal, 2,2V min)
 *   ────────────────────────────────────────────────────────────────────
 *   TOTAL  11–12 Bauteile  ← ORIGINAL-Layout, nur PIC getauscht!
 *
 * ─── BUGFIXES gegenüber Originalversion ──────────────────────────────
 *
 *  [FIX-1] LAT-Register statt PORT für Ausgaben (RMW-Problem vermieden)
 *          LATC/LATA5 statt PORTC/RA5 in show()
 *  [FIX-2] Atomarer 16-Bit-Zugriff auf sleep_counter
 *          (volatile uint16_t → GIE kurz deaktivieren!)
 *  [FIX-3] SLEEP_TIMEOUT korrigiert: 153 statt 610
 *          (TMR0 @ 4MHz/4/256 overflow = ~15,26 Hz, nicht 61 Hz!)
 *  [FIX-4] Button-Release-Timeout nach Wake-Up (kein ewiges Warten)
 *  [FIX-5] IOC-Flag vor SLEEP() löschen (verhindert sofortiges Re-Wake)
 *  [FIX-6] Separate Helper-Funktionen reset_sleep_counter() / read_sleep_counter()
 *  [FIX-7] OPTION_REG Bits 7+6 korrekt beibehalten beim Timer0-Setup
 *  [FIX-8] Unbenutzte RA0-RA2 als Ausgänge (definierter LOW-Zustand)
 *
 * ═══════════════════════════════════════════════════════════════════════
 */

// ─── CONFIG BITS (PIC16F1825) ─────────────────────────────────────────
// CONFIG1
#pragma config FOSC   = INTOSC  // Interner Oszillator
#pragma config WDTE   = OFF     // Watchdog Timer aus
#pragma config PWRTE  = ON      // Power-Up Timer an
#pragma config MCLRE  = OFF     // RA3/MCLR → digitaler I/O (kein Pullup-R!)
#pragma config CP     = OFF     // Kein Code-Schutz
#pragma config CPD    = OFF     // Kein EEPROM-Schutz
#pragma config BOREN  = ON      // Brown-Out Reset aktiv
#pragma config CLKOUTEN = OFF   // CLKOUT aus
#pragma config IESO   = OFF     // Intern/Extern-Umschaltung aus
#pragma config FCMEN  = OFF     // Fail-Safe Clock Monitor aus

// CONFIG2
#pragma config WRT    = OFF     // Flash Write-Schutz aus
#pragma config PLLEN  = OFF     // PLL aus
#pragma config STVREN = ON      // Stack-Overflow → Reset
#pragma config BORV   = LO      // Brown-Out Low Trip Point
#pragma config LVP    = OFF     // Low-Voltage Programming aus

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ  4000000UL   // 4 MHz intern (exakt, kein RC-Toleranzproblem!)

// ─── Timing-Konstanten ────────────────────────────────────────────────
// TMR0 @ 4MHz: Fosc/4 = 1MHz → /256 (Prescaler) = 3906 Hz
// Overflow bei 256 Counts: 3906/256 = ~15,26 Hz
// [FIX-3] SLEEP_TIMEOUT korrigiert (war 610 — falsch!)
#define SLEEP_TIMEOUT   153u    // 10 Sekunden × 15,26 Hz = 152,6 ≈ 153
#define BUTTON_TIMEOUT  50000u  // [FIX-4] Button-Release-Timeout (~1-2s)

// ─── LED-Bit-Definitionen ─────────────────────────────────────────────
// Bits 0–5 = PORTC/LATC (RC0–RC5), Bit 6 = RA5/LATA5
#define A  (1u<<0)   // RC0 – oben links
#define B  (1u<<1)   // RC1 – oben rechts
#define C  (1u<<2)   // RC2 – mitte links
#define D  (1u<<3)   // RC3 – Zentrum (Auge)
#define E  (1u<<4)   // RC4 – mitte rechts
#define F  (1u<<5)   // RC5 – unten links
#define G  (1u<<6)   // RA5 – unten rechts  ← anderer Port!

// ─── Würfelmuster ─────────────────────────────────────────────────────
const uint8_t DICE[6] = {
    D,                           // ⚀  1: Zentrum
    B|F,                         // ⚁  2: oben-rechts + unten-links
    B|D|F,                       // ⚂  3: Diagonale + Mitte
    A|B|F|G,                     // ⚃  4: vier Ecken
    A|B|D|F|G,                   // ⚄  5: Ecken + Mitte
    A|B|C|E|F|G,                 // ⚅  6: alle außer Mitte
};

// ─── Zufallsquelle + Sleep-Timer ──────────────────────────────────────
// TMR0 läuft seit Power-On frei → beim Tastendruck ist der Zählerstand
// "zufällig" (menschliche Reaktionszeit >> Timer-Auflösung ~65µs)
volatile uint8_t  tmr0_count   = 0;
volatile uint16_t sleep_counter = 0;  // [FIX-2] uint16_t: braucht atomaren Zugriff!

void __interrupt() isr(void) {
    // Timer0: Zufallsquelle + Sleep-Countdown
    if (INTCONbits.TMR0IF) {
        tmr0_count++;
        sleep_counter++;           // ~15,26 Hz
        INTCONbits.TMR0IF = 0;
    }
    // Interrupt-on-Change RA4: Wake-Up aus Sleep
    if (INTCONbits.IOCIF) {
        if (IOCAFbits.IOCAF4) {
            IOCAFbits.IOCAF4 = 0;  // [FIX-5] Flag löschen → kein Re-Wake
            sleep_counter = 0;
        }
        INTCONbits.IOCIF = 0;
    }
}

// ─── [FIX-2] Atomarer 16-Bit-Zugriff auf sleep_counter ──────────────
// uint16_t-Zugriff ist auf 8-Bit-PIC NICHT atomar → kurz GIE sperren!
static void reset_sleep_counter(void) {
    INTCONbits.GIE = 0;
    sleep_counter = 0;
    INTCONbits.GIE = 1;
}

static uint16_t read_sleep_counter(void) {
    uint16_t val;
    INTCONbits.GIE = 0;
    val = sleep_counter;
    INTCONbits.GIE = 1;
    return val;
}

// ─── Variable Delay ───────────────────────────────────────────────────
static void delay_ms(uint16_t ms) {
    while (ms--) __delay_ms(1);
}

// ─── [FIX-1] LED-Ausgabe via LAT (kein RMW-Problem!) ─────────────────
// LATC/LATA statt PORTC/PORTA → schreibt direkt in Output-Latch,
// liest NICHT den Pin-Zustand zurück (Read-Modify-Write vermieden!)
static void show(uint8_t pattern) {
    LATC = pattern & 0x3Fu;                    // RC0–RC5 via LATC
    LATAbits.LATA5 = (pattern >> 6u) & 1u;     // RA5 via LATA5
}

// ─── Taster-Polling: Software-Entprellung + Flanken-Erkennung ─────────
// PORTAbits.RA4 bleibt korrekt für EINGÄNGE (PORT lesen = aktueller Pegel)
static uint8_t button_pressed(void) {
    if (!PORTAbits.RA4) {           // LOW = Taster gedrückt
        delay_ms(25);               // Entprell-Fenster
        if (!PORTAbits.RA4) {
            while (!PORTAbits.RA4); // Warten bis losgelassen
            delay_ms(15);
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
        if      (i <  8u) pause = 40;
        else if (i < 16u) pause = 90;
        else               pause = 160;
        delay_ms(pause);
    }
    show(DICE[final_idx]);
}

// ─── Ergebnis-Blinken ─────────────────────────────────────────────────
static void blink_result(uint8_t pattern) {
    uint8_t i;
    for (i = 0; i < 3u; i++) {
        show(0x00);     delay_ms(100);
        show(pattern);  delay_ms(200);
    }
}

// ─── Einschalt-Animation ──────────────────────────────────────────────
static void startup_seq(void) {
    uint8_t i;
    for (i = 0; i < 6u; i++) { show(DICE[i]); delay_ms(140); }
    show(DICE[0]); delay_ms(400);
    show(0x00);    delay_ms(200);
    show(DICE[0]);
}

// ─── Sleep-Modus aktivieren ───────────────────────────────────────────
static void enter_sleep(void) {
    show(0x00);                      // LEDs aus

    // [FIX-5] IOC-Flag VOR Sleep löschen (verhindert sofortiges Re-Wake)
    IOCAFbits.IOCAF4 = 0;
    INTCONbits.IOCIF = 0;

    // Negative Flanke auf RA4 aktivieren (Taster drücken = Wake)
    IOCAPbits.IOCAP4 = 0;            // Positive Edge disabled
    IOCANbits.IOCAN4 = 1;            // Negative Edge enabled
    INTCONbits.IOCIE = 1;            // IOC Interrupt an

    // Sleep-Counter zurücksetzen
    sleep_counter = 0;               // Kein Interrupt-Risiko (wir schlafen gleich)

    SLEEP();                         // PIC schlafen legen (~1 µA)
    NOP();                           // Nach Wake-Up hier weitermachen

    // Nach Wake-Up: IOC deaktivieren
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
    // RA3 = 1 (immer Input-Only auf PIC16F1825)
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
    //            ^^^^^^^^^^^^^ nWPUEN(7) + INTEDG(6) beibehalten
    //                                      T0CS=0, T0SE=0, PSA=0, PS=111(1:256)

    INTCONbits.TMR0IE = 1;   // Timer0 Interrupt aktivieren
    INTCONbits.GIE    = 1;   // Global Interrupt Enable

    // ── Startup ───────────────────────────────────────────────────────
    startup_seq();
    reset_sleep_counter();    // [FIX-2] Atomarer Reset

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
