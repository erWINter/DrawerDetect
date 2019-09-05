// Board: Pro Trinket 5V/16MHz (USB)
//
// available Trinket Pro 5V pins:     v pin 13 is blink-LED
// RX TX 3~ 4  5~ 6~ 8 9~ 10~ 11~ 12 13* A0 A1 A2 A3 A4/SDA A5/SCL (A6 A7)
//
// Software written for ProTrinket 5V to check the opening of cabinet 
//      drawers in 2-3 cabinets by usage of up to 3 Ultra Sonic sensors
//      per cabinet module
//
// The module HC-SR04 is triggered by sending an US-pulse to TRIG input:
//      done by generating a high signal pulse with 10us length
//      the module has 4 pin connectors:
//          [+5V,TRIG,Echo,Gnd]         front-view
//
// The red onboard LED is a short blinking LED, showing running board
//      The final version blinks <TRINKET_NR> times at startup.
//
// An optional onboard RGB-LED placed on the PCB signals an open drawer
//      by light-up red for Oben, green for Mitte, blue for Unten.
//      The brightness is set via PWM proportional to drawer open-width
//
// New: A 4pin connector for I2C_LCD allows to connect an LCD module,
//      showing on an LCD 16x2 display following information:
// !only available if include file "LCD_BarGraph.h" is included!
//
// LevelMode:
//          +----------------+
//          |Tr#1,Cab/Drw: 1u|   1u open 43%
//          |!!!!!!!       43|  draweropen bargraph in percentage
//          +----------------+
// SerialMode:
//          +----------------+
//          |TN1 1u 2umo 3umo| fix from setup
//          |>1u99 2m88 3o09<| sent-out serial info to Feather
//          +----------------+
//
//-------------------------------------------------------------------------------

// individual settings for all the 18 drawers are configured  by TRINKET_NR
// set 1-3  for individual ProTrinket Nr. 1-3 ==> 3 different Pro Trinkets!
#define TRINKET_NR 2

// uncomment next, when tested and finally released
//#define DEBUG

#ifdef DEBUG
#warning "DEBUG VERSION *****************************" 
#define DEBUG_PRINT(var) Serial.print( #var "= "); Serial.println( var);
#define BLINK_FREQ(x) (((x)%16) < 1)
#else
#warning "FINAL PRODUCT VERSION with blinking red LED" 
#define DEBUG_PRINT(var)
#define BLINK_FREQ(x) (((x)%64) < 1)
#endif

// import include fie DrawerDetect.h, which holds all setups
#include "DrawerDetect.h"

// optional include 
//!!! #include "LCD_I2C_optional.h"

// global used variables
int  NCabinets;                  // served Cabinets, normally 2 or 3

//######################################################################################
void setup()
{
    // first setup serial line to send open drawer message to next ProTrinket
    // also listen for messages from previous ProTrinket and pass it through
    Serial.begin( 19200);       // KitchenControl must use same baudrate

    pinMode( PIN_RED_LED, OUTPUT);
    setRedLED( false);       // LED of
    /////////

    // use an extra RGB-LED mounted on PCB to show drawer-state
    for (int i = 0; i< 3; i++) {
        analogWrite( PIN_PWM4RGB[i], PWM_AK(0));   // set the RGB-PWM off
    }

    while (!Serial);        // idle, until Serial available

#ifdef DEBUG
    // print out information to identify program/code
    Serial.println( infoLOC( "open drawer detection with US sensor HC-SR04") );
    Serial.print( "Trinket-Nr.= ");
    Serial.println( TRINKET_NR);
#endif

    NCabinets = sizeof( CabinetSetup) / sizeof( CabinetSetup[0]);
    DEBUG_PRINT( NCabinets);

    // setup all the pins for each cabinet with drawers
    String lcdInfo = "TN" + String( TRINKET_NR);
 
    for (int nC = 0; nC < NCabinets; nC++) {
        String pinInfos =
            setupCabinetDrawers( nC, &lcdInfo);
            ///////////////////
#ifdef DEBUG
        Serial.println( pinInfos);
#endif
    }

#ifdef USED_LCD_I2C
    LCDfound =
        LCD_I2Csetup( TRINKET_NR, lcdInfo);
        ////////////
#endif
    delay( 100);     // allow all US sensor electronics to power-up

#ifndef DEBUG
    signalNrByBlinks( TRINKET_NR);      // TBD
    ////////////////
#endif
}

//######################################################################################
// setup all GPIO pins for one cabinet and preset by zero
String setupCabinetDrawers( int nC, String *lcdMsg)
{
    sDrawerType *p2Setup = &CabinetSetup[nC];
    String msg = "\nCabNr= " + String( p2Setup->cabNr);

    *lcdMsg += " " + String(p2Setup->cabNr);

    int *PinArr = p2Setup->US_pins;     // [0]:Trig, [1-3] 3 Echos
    for (int i=0; i <= 3; i++) {
        int pin4US = PinArr[i];
        if (pin4US <= 0)
            continue;                       // skip undefined pin

        msg += "\n\t" + String( i);
        msg += ". pin4US('" + String( DrawNum2x[i]) + "')";
        *lcdMsg += String(DrawNum2x[i]);
        
        if (i == 0) {
            // [0] is trigger pin
            pinMode( pin4US, INPUT_PULLUP); // define Echo input
            msg += "\tTrigger";
        }
        else {
            // echo pins (u m o)
            pinMode( pin4US, OUTPUT);       // set trigger to output
            digitalWrite( pin4US, LOW);     // and initialized to LOW
            msg += "\tEcho";
        }
    }
    msg += "\n";
    return msg;
}

//######################################################################################
void loop()
{
static int FixDelay_ms = (240 + 6*TRINKET_NR) / NCabinets;  // avoid collisions
    // above delay is fix forever
    
static uint8_t blinky = 0;
    // use onboard red LED to signal running prorgam
    blinky++;
    
    setRedLED( BLINK_FREQ(blinky));
    /////////

static int openPercent;                             // returned value
static int drawerNumLast[3] = { -1, -1, -1 };       // max. 3 cabinets

    String message = "";    // reset at loop start
    
    for (int nC = 0; nC < NCabinets; nC++) {
        
        delay( FixDelay_ms);    // 90ms or 60ms

        // find any open drawer
        int drawerNum =
            openedDrawer( nC, &openPercent);
            ////////////       ########### returns also openPercent value

        // show only changes in state
        if (drawerNumLast[nC] != drawerNum) {
            drawerNumLast[nC]  = drawerNum;    // store last pulled-out/moved drawer

            if ((nC > 0) && (message == "")) {
                message += "\t\t\t\t";
            }

            message += CabinetSetup[nC].cabNr;
            message += "-Unterschrank ";
            if (drawerNum > 0) {
                message += drawerNum;
                message += ". SL offen";
            }
            else {
                message += "alle SL zu";     // no drawer open
            }
#ifdef USED_I2C_LCD
            LCDshowCabDrwBar( CabinetSetup[nC].cabNr, DrawNum2x[drawerNum], openPercent);
            ////////////////
#endif
        }
        // send the current state via serial over ONE line to WS8192 controler MCU
        sendInfo( nC, drawerNum, openPercent);
        ////////
    }

#ifdef DEBUG
    // print out message when changes occured
    if (message != "") {
        Serial.println( message);
    }
#endif
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ forever main-loop

//######################################################################################

////////////////////////////////////////////////////////////////////////////////////////
// used special subroutines (except setup and loop)
////////////////////////////////////////////////////////////////////////////////////////

//######################################################################################
// set onboard red LED on or off
void setRedLED( bool setOn)
{
    digitalWrite( PIN_RED_LED, setOn ? HIGH : LOW);
}

//######################################################################################
// send the current state information about drawer
//      via  one serial line to WS2812b controller MCU
// the protocol contains the following information:
//
void sendInfo( int nC, int drNum, int openPerc)
{
    // read in possible messages sent by previous trinkets
    String previousInfo = "";
    while (Serial.available()) {
        // character is available
        char chAscii = Serial.read();       // read one character from RX
        if (chAscii < '0') {
            chAscii = ' ';
        }
        previousInfo += chAscii;

        if (chAscii == ' ') {
            int len = previousInfo.length();
            // eg. "1251 2000 "
            //            ^^^
            String last3zeros = previousInfo.substring(len-4);
            if (last3zeros == "000 ") {
                // cut off last drawer-Info
                previousInfo = previousInfo.substring(0,len-5);
                // eg. "1251 "
            }
        }
    }

    if (openPerc < 5)           // 5% of 50 cm = 2,5cm means close
        drNum = 0;

    if (drNum == 0)             // if no drawer number is given,
        openPerc = 0;           // then not open

    char fourDigits[5];

    sprintf( fourDigits, "%d%d%02d", CabinetSetup[nC].cabNr, DrawNum2x[drNum], openPerc);

    // lowest two digits contain percentage of drawer pulled-out 00-99
    // eg. = "0195"   means 1st drawer of left cabinet '0' is 95% open
    // eg. = "1000"   means none of the drawers of right cabinet '1' are open

    // the thousand digit holds the cabinet number 0-9 defined at the top of code
    // the hundred digit contains the opened drawer number 0 or 1-4 of the cabinet

    String CxPO = String( fourDigits);
    // now CxPO contains always 4 digits
    //        PO = Percentage Open 00-99
    //      x   = Drawer-Position: o, m, u
    //     C    = nr.of Cabinet 0-9

    if (drNum > 0) {
        previousInfo += CxPO;
        // eg. "xxxx yyyy 1u95"
    }

    // use an extra RGB-LED mounted on PCB to show drawer-state
    if ( (drNum >= 1)
      && (drNum <= 3) ) {

        int pwmVal = map( openPerc,0,99, 0,255);
        analogWrite( PIN_PWM4RGB[drNum-1], PWM_AK(pwmVal));
    }

#ifdef USED_I2C_LCD
    LCDshowSerial( nC, fourDigits);
    /////////////
#endif
#ifdef DEBUG
    return;
#endif
    Serial.println( previousInfo);
    // the main MCU will receive (every 100ms) serial infos like this :
/*
 * full information is sent, if OPEN_ONLY is false:
 *  "1000 2000 300 4000 5192 6210 7000 8000"
 *                      !!!! !!!!
 *                      !!!! 2nd drawer 10% open of cab#6
 *                      1st drawer 92% open of cab#5
 *
 *  only send open cabinets information, if OPEN_ONLY is true:
 *  "5192 6210"
 *
 */
}

//######################################################################################
// for one cabinet detect the all 3 drawer pull-outs by usage of 3 US-sensors
// if none of the drawers is open, then return the drawer-number 0
// otherwise detect the widest opend drawer, return the drawer-nr. and
// return via given pointer-address the value of open in percent, 99 means maximal opened
int openedDrawer( int nC, int *percentOpen)
{
    const int TWO_EXP_NUS = 8;      // max 3 drawers per cabinet!

    const int Mask2DrawerNum[TWO_EXP_NUS] = {
         0, 1, 2,2, 3,3,3,3    // bit-mask to drawer nr.
    };

    // avoid microSecond negative difference
    const uint32_t MICROS_OVERFLOW = 0xFFFFFFF - 4095; // ~4ms means 330m/s*0.004= 1,3m

    int openDrawer;

    sDrawerType *p2Setup = &CabinetSetup[nC];

    int maskAll = p2Setup->mskOMU;
    uint8_t mALL = (uint8_t)maskAll;          // = 7 or 0b111 for NUS=3

    int *PinArr = p2Setup->US_pins;     // [0]:Trig, [1-3] 3 Echos
    int pinTrig = PinArr[0];
    if (pinTrig < 0) {
        // if < 0 then not used as trigger

        // just read the digital state of one switch
        int i = 1;
        if (maskAll == 2) i = 2;
        if (maskAll == 4) i = 3;

        openDrawer = 0;             // none of the drawers is open
        if (digitalRead( PinArr[i]) == LOW) {
            openDrawer = i;
        }

        // pass back percentage opened
        *percentOpen = openDrawer != 0 ? 99 : 0;   // generate 0-99%

        // finally return the number of open drawer
        return openDrawer;      // returns 1, 2, 3 (,4 ) or 0, if no drawer is open
    }

    // normal US sensor

    // very special code to avoid getting negative us-difference due to overflow
    uint32_t us = micros();         // overflows from 4,294,967,295 to 0 every 70m
    if (us > MICROS_OVERFLOW) {
        // micros is shortly (4ms) before overflow
        while (us > 1100UL) {
            delay( 1);              // wait 1ms= 1000us
            us = micros();
        }
        // now micros counter is over the edge
    }

    // generate positive start pulse with 10 microsecs pulse length for all 3 US-sensors
    digitalWrite( pinTrig, HIGH);
    delayMicroseconds( 10);
    digitalWrite( pinTrig, LOW);

    // array for measuring the pulse duration in micro-seconds
    uint32_t tDurMicros[3];

    // wait until all 3 sensors receive their first pulse signal
    uint8_t mSet;
    uint8_t maskSeen = 0;
    uint32_t timedOut = micros() + CLOSED_US + MAX_US;
    
    while (maskSeen != mALL) {
        for (int i=1; i <= 3; i++) {
            mSet = 1<<(i-1);
            if (maskSeen & mSet)
            	continue;
            if (micros() > timedOut) {
                *percentOpen = 1;   // timed-out
                return 0;           // timed-out !
            }
            if (digitalRead( PinArr[i+1]) != HIGH)
            	continue;

            tDurMicros[i] = micros();
            maskSeen |= mSet;   		// set the bit
        }
    }
    // now all 3 US sensors are high

    uint8_t maskLast = maskSeen;
    // now wait until all 3 sensors pulses are low again
    while (maskSeen > 0) {
        maskLast = maskSeen;
        for (int i=1; i <= 3; i++) {
            mSet = 1<<(i-1);            // = 1,2,4
            if (!(maskSeen & mSet))
            	continue;

            if (micros() > timedOut) {
                *percentOpen = 1;   // timed-out
                return 0;           // timed-out !
            }
            if (digitalRead( PinArr[i+1]) != LOW)
            	continue;

             tDurMicros[i] = micros() - tDurMicros[i];
             maskSeen = mSet^0xFF;   	// take bit away
        }
    }

    openDrawer = Mask2DrawerNum[maskLast & 0b111];
    // openDrawer = 1,2 or 3 (or 4)

    uint16_t dur_us = tDurMicros[openDrawer-1];  // one less due to array-index

    // now dur_us measured the widest open drawer
#ifdef DEBUG
    char line[80];
    sprintf( line, "dur_us[%d]= %u  openDrawer= %d", nC,dur_us, openDrawer);
    Serial.println( line);
#endif
    // map measured duration to 0..MAX_DC for analog output level(dutyCycle)
    uint16_t min_us = MIN_US;
    uint16_t max_us = MAX_US;

    // map sonic runtime to percentage of opening (max.99%)
    int percentage = map( dur_us, min_us,max_us, 0,99);

    percentage = constrain( percentage, 0,99);  // must be between 0 and 99%

    if (dur_us < CLOSED_US) {
        // drawer closed
        percentage = 0;             // reset all drawers to be closed
        openDrawer = 0;             // none of the drawers is open
    }

    // pass back percentage opened
    *percentOpen = percentage;

    // finally return the number of open drawer
    return openDrawer;      // returns 1, 2, 3 (,4 ) or 0, if no drawer is open
}

#ifndef DEBUG
//######################################################################################
// lets identify Trinket Pro Nr. 1,2 or 3 by LED blinking nTimes at startup
void signalNrByBlinks( int nTimes)
{
    if (nTimes >= 0) return;    // to be removed !! FIXME!!
    while (nTimes-- > 0) {
        setRedLED( false);
        delay( 800);            // 0.8sec OFF
        
        setRedLED( true);
        delay( 1200);           // 1.2sec ON
        
        setRedLED( false);
        delay( 800);            // 0.8sec OFF
    }
    delay( 2000);               // finally wait 2 secs
}
#endif

#ifdef DEBUG
//######################################################################################
String infoLOC( String text)     // must be always located at the end of source code
{
    text += "\n";
    text += "FILE: " __FILE__ "\n";
    text += "DATE: " __DATE__ "  TIME: " __TIME__ "\n";

    text += "\n ***** TRINKET_NR= ";
    text += TRINKET_NR;
    text += "\n\nLOC= ";

    int loc = __LINE__ + 4;
    text += loc;
    text += "\t; Lines of Code";
    return text;
}
#endif

//######################################################################################
//end_of_code   E_Winter@web.de started 2019-08-22 (final: 08-31)
//
