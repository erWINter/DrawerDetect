#ifndef _DrawerDetect_h
#define _DrawerDetect_h

#ifndef TRINKET_NR
#error "-ERROR- no TRINKET_NR 1-3 defined in DrawerDetect.ino !"
#else
// check to be between 1 and 3
#if TRINKET_NR < 1 || TRINKET_NR > 3
#error "-ERROR- bad TRINKET_NR !"
#endif
#endif

// 50cm Schubladen maximale Auszug Laenge
#define DRAWER_LEN_CM 50

// set 0 for common Kathode or 1 for common Anode for RGB onboard LED
#define RGBLED_COM_ANODE 0

// define structure for easy defining cabinet/drawer configuration
struct sDrawerType {
    int cabNr;      // 1-7  Identify Cabinet Nr. as in Manuels IKEA-plan

        // a bit-mask is used to identify the unten/mitte/oben drawers
        // bit-mask for existing drawers 1:u, 2:m, 4:o, (eg. 5: u & o)
    int mskOMU;     // = 0b001 .. 0b111

        // define ProTrinket Pin GPIO (negativ if not used)
    int US_pins[4]; // [0]:commonTRIG, [1]:EchoU, [2]:EchoM, [3]:EchoU
};

//                          0    1    2    3
const char DrawNum2x[] = { 'T', 'u', 'm', 'o' };

//-------------------------------------------------------------------------------
// available Pro Trinket 5V pins:
// RX TX 3~ 4  5~ 6~ 8 9~ 10~ 11~ 12 13* A0 A1 A2 A3 A4 A5

// ATTENTION ! I2C uses the 2 I2C-pins: SDA(A4) and SCL(A5)

//  pin 13 is red onboard LED_BUILTIN (active HIGH)
//-------------------------------------------------------------------------------
// define all used GPIO pins:

const int PIN_RED_LED = LED_BUILTIN;        // red LED_BUILTIN = 13

// an additional RGB LED shows currently open drawer
const int PIN_PWM4RGB[3] = { 9, 10, 11 };   // all must be PWM outputs ~
//                           r   g   b
#if RGBLED_COM_ANODE==0
#define PWM_AK(x) (x)
#else
#define PWM_AK(x) (255-(x))
#endif

//  with help of above defined structure we setup the individual pins
sDrawerType CabinetSetup[] = {

//-------------------------------------------------------------------------------
#if TRINKET_NR == 1
#warning "Trinket #1 Herd"
// RX TX 3~ 4  5~ 6~ 8 9~ 10~ 11~ 12 13* A0 A1 A2 A3 A4 A5 (A6 A7)
//          T0 T1 T2   r   g   b  e2     e0 e0 e0 e1 e1 e1
    //C    omu    t    u  m  o
    { 2, 0b111,  {12, A2,A1,A0} },    //L 90cm Unterschrank, 3 SL
    { 3, 0b111,  {5,   3, 4, 6} }     //R 90cm Unterschrank, 3 SL
    { 1, 0b001,  {8,  -1,-1,A3} },    //X untere Auszieh Schublade Herd
#endif
//-------------------------------------------------------------------------------
#if TRINKET_NR == 2
#warning "Trinket #2 Ecke"
// RX TX 3~ 4  5~ 6~ 8 9~ 10~ 11~ 12 13* A0 A1 A2 A3 A4 A5 (A6 A7)
//          T0         r   g   b         e0 e0 e0    e1
    //C    omu    t    u  m  o
    { 5, 0b111,  {12, A2,A1,A0} },    //L 40cm Unterschrank, 3SL
    { 4, 0b010, {-5,  -3, 4,-6} }     //R 0:noTrig! Eck-Auszieh Fach (reed-switch)
#endif
//-------------------------------------------------------------------------------
#if TRINKET_NR == 3
#warning "Trinket #3 Spuele"
// RX TX 3~ 4  5~ 6~ 8 9~ 10~ 11~ 12 13* A0 A1 A2 A3 A4 A5 (A6 A7)
//          T0 T1      r   g   b         e0 e0 e0 e1 e1 e1
    //C    omu    t    u  m  o
    { 6, 0b111,  {12, A2,A1,A0} },    //L 40cm Unterschrank, 3 SL
    { 7, 0b101,  {5,   3,-4, 6} }     //R 80cm Spuelen-Unterschrank, SLunten&oben
#endif
//-------------------------------------------------------------------------------

};      // end of structure setup

//--------------------------------------------------------------------
// macro definition

// define the speed of sound in air: 343.2 m/sec at 20 degCelsius
#define SPEEDofSOUND_CMpSEC 34320

// define macro to convert cm to microseconds
#define CONVERT_cm2us(cm) ((1000000*(cm))/SPEEDofSOUND_CMpSEC)
// BTW: lower u means shortcut for greek character 'micro'
//      (m could not be used, because it's confusing with millis)

#define MIN_CM 3
// drawer closed value
#define CLOSED_CM (MIN_CM + 2)
#define MAX_CM (MIN_CM + DRAWER_LEN_CM)

// twice (2*) because sound must go foreward AND backward too
const uint16_t MIN_US = CONVERT_cm2us(2*MIN_CM);        //  174us (3cm)
const uint16_t MAX_US = CONVERT_cm2us(2*MAX_CM);        // 3088us (50cm)
const uint16_t CLOSED_US = CONVERT_cm2us(2*CLOSED_CM);  //  290us (5cm)

#endif

/*
    from DrawerDetect.net produced by KiCad 2019-09-02
    -this table helps to re-arrange the GPIO pins-

(pin (num 7) (name A0) (type 3state))
(pin (num 8) (name A1) (type 3state))
(pin (num 9) (name A2) (type 3state))
(pin (num 10) (name A3) (type 3state))
(pin (num 11) (name A4) (type 3state))
(pin (num 12) (name A5) (type 3state))

(pin (num 16) (name P3) (type BiDi))
(pin (num 18) (name P5) (type BiDi))
(pin (num 19) (name P6) (type BiDi))
(pin (num 1) (name P9) (type BiDi))
(pin (num 2) (name P10) (type BiDi))
(pin (num 3) (name P11) (type BiDi))

(pin (num 17) (name D4) (type BiDi))
(pin (num 20) (name D8) (type BiDi))
(pin (num 4) (name D12) (type BiDi))

(pin (num 5) (name D13) (type BiDi)) // red onboard BUILTIN_LED

(pin (num 14) (name RX) (type input))
(pin (num 15) (name TX) (type output))

*/
