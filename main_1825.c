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
 *  Gesamt           14 Bauteile  →  11 Bauteile  ← absolutes Minimum!
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
 *  LED-Layout:
 *    [a=RC0] [b=RC1]
 *    [c=RC2] [d=RC3] [e=RC4]
 *    [f=RC5]         [g=RA5]
 *
 *  MINIMALES BOM (ORIGINAL-kompatibel, 3V):
 *   U1   PIC16F1825     ×1
 *   C1   3,3 µF / 6,3V  ×1   (VDD-Bypass, Elko)
 *   LED1–LED7 (rot!)    ×7   ⚠️ NUR rot/gelb/grün! (Vf<2,1V, kein Weiß/Blau!)
 *   R1–R7  47 Ω         ×7   (LED-Vorwiderstände @ 3V, ~8-10mA)
 *   SW1  Drucktaster    ×1
 *   JP1  Jumper 2-Pin   ×1   (Power On/Off)
 *   BT1  2×AAA Halter   ×1   (3V nominal, 2,2V min)
 *   ────────────────────────────────────────────────────────────────
 *   TOTAL  11 Bauteile  ← ORIGINAL-Layout, nur PIC getauscht!
 *
 * ═══════════════════════════════════════════════════════════════════════
 */

// ─── CONFIG BITS (PIC16F1825 — komplett anders als PIC16F84A!) ────────
// CONFIG1
#pragma config FOSC   = INTOSC  // Interner Oszillator  ← kein externer Takt!
#pragma config WDTE   = OFF     // Watchdog Timer aus
#pragma config PWRTE  = ON      // Power-Up Timer an
#pragma config MCLRE  = OFF     // RA3/MCLR → digitaler I/O (kein Pullup-R nötig!)
#pragma config CP     = OFF     // Kein Code-Schutz
#pragma config CPD    = OFF     // Kein EEPROM-Schutz
#pragma config BOREN  = ON      // Brown-Out Reset aktiv
#pragma config CLKOUTEN = OFF   // CLKOUT aus (RA4 als I/O verfügbar)
#pragma config IESO   = OFF     // Intern/Extern-Umschaltung aus
#pragma config FCMEN  = OFF     // Fail-Safe Clock Monitor aus

// CONFIG2
#pragma config WRT    = OFF     // Flash Write-Schutz aus
#pragma config PLLEN  = OFF     // PLL aus (4x Multiplikator)
#pragma config STVREN = ON      // Stack-Overflow → Reset
#pragma config BORV   = LO     // Brown-Out Low Trip Point
#pragma config LVP    = OFF     // Low-Voltage Programming aus

#include <xc.h>
#include <stdint.h>

// Interner Oszillator auf 4 MHz → _XTAL_FREQ = 4000000UL (exakt!)
// Kein RC-Toleranzproblem mehr!
#define _XTAL_FREQ  4000000UL

// ─── LED-Bit-Definitionen ─────────────────────────────────────────────
// Bits 0–5 = PORTC (RC0–RC5), Bit 6 = RA5
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

// ─── Zufallsquelle: TMR0 freilaufend ─────────────────────────────────
volatile uint8_t tmr0_count = 0;

void __interrupt() isr(void) {
    if (INTCONbits.TMR0IF) {
        tmr0_count++;
        INTCONbits.TMR0IF = 0;
    }
}

// ─── Variable Delay ───────────────────────────────────────────────────
static void delay_ms(uint16_t ms) {
    while (ms--) __delay_ms(1);
}

// ─── LED-Ausgabe: PORTC (bits 0–5) + RA5 (bit 6) ────────────────────
static void show(uint8_t pattern) {
    PORTC = pattern & 0x3Fu;                   // RC0–RC5
    PORTAbits.RA5 = (pattern >> 6u) & 1u;      // RA5 = bit 6 (LED g)
}

// ─── Taster auf RA4: aktiv LOW, internen Pull-Up via WPUA ─────────────
static uint8_t button_pressed(void) {
    if (!PORTAbits.RA4) {
        delay_ms(25);
        if (!PORTAbits.RA4) {
            while (!PORTAbits.RA4);
            delay_ms(15);
            return 1;
        }
    }
    return 0;
}

// ─── Animation ───────────────────────────────────────────────────────
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

static void blink_result(uint8_t pattern) {
    uint8_t i;
    for (i = 0; i < 3u; i++) {
        show(0x00);     delay_ms(100);
        show(pattern);  delay_ms(200);
    }
}

static void startup_seq(void) {
    uint8_t i;
    for (i = 0; i < 6u; i++) { show(DICE[i]); delay_ms(140); }
    show(DICE[0]); delay_ms(400);
    show(0x00);    delay_ms(200);
    show(DICE[0]);
}

// ─── MAIN ─────────────────────────────────────────────────────────────
void main(void) {
    uint8_t result;

    // ── Interner Oszillator auf 4 MHz ─────────────────────────────────
    // OSCCON: IRCF<3:0> = 1101 → 4 MHz (Datasheet DS41440E, Table 5-1)
    OSCCONbits.IRCF = 0b1101;   // 4 MHz
    OSCCONbits.SCS  = 0b10;     // Interner Oszillator als Taktquelle

    // ── ANALOG-DISABLE: PFLICHT auf PIC16F1825! ───────────────────────
    // Ohne diese Zeilen funktioniert digitale I/O nicht!
    // (Gibt es NICHT auf PIC16F84A — häufige Fehlerquelle!)
    ANSELA = 0x00;   // Port A: alle Pins digital
    ANSELC = 0x00;   // Port C: alle Pins digital

    // ── Port-Konfiguration ────────────────────────────────────────────
    // TRISA: 1=Eingang, 0=Ausgang
    // RA3=1 (immer Input/MCLR), RA4=1 (Taster), RA5=0 (LED g), Rest=0
    TRISA = 0b00011000;   // RA4=in, RA3=in(always), andere=out
    TRISC = 0b00000000;   // Port C: alle Ausgänge (LEDs a–f)

    PORTA = 0x00;
    PORTC = 0x00;

    // ── Interne Pull-Ups aktivieren ───────────────────────────────────
    // Global Pull-Up Enable: OPTION_REG.nWPUEN = 0
    OPTION_REGbits.nWPUEN = 0;   // Pull-Ups global aktivieren
    WPUA = 0b00010000;            // WPUA4 = 1 → Pull-Up auf RA4 (Taster)
    WPUC = 0b00000000;            // Port C: keine Pull-Ups nötig

    // ── Timer0: freilaufend für Zufallsgenerator ──────────────────────
    // OPTION_REG: T0CS=0 (intern), PSA=0 (Prescaler→TMR0), PS=111 (1:256)
    OPTION_REG = (OPTION_REG & 0xC0u) | 0b00000111u;
    //            ^^^^^^^^^^^^^ nWPUEN + INTEDG beibehalten!

    INTCONbits.TMR0IE = 1;   // Timer0 Interrupt aktivieren
    INTCONbits.GIE    = 1;   // Global Interrupt Enable

    // ── Startup & Hauptschleife ───────────────────────────────────────
    startup_seq();

    while (1) {
        if (button_pressed()) {
            result = tmr0_count % 6u;
            roll_animation(result);
            delay_ms(500);
            blink_result(DICE[result]);
        }
    }
}
