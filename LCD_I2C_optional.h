#define INCLUDED_LCD_I2C
#warning ">>>>>>>>>>>>>>> information via LCD_I2C will be available <<<<<<<<<<<<<<"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// I2C uses the 2 I2C-pins: SDA and SCL (aka A4,A5)
// connector to I2C-module: [GND,VCC,SDA(A4),SCL(A5)]

// LEVEL_MODE
//          +----------------+
//          |TN1: Cab/Drw= 1u|
//          |!!!!!!!       43|
//          +----------------+
// SERIAL_MODE:
//          +----------------+
//          |TN1 1u 2umo 3umo| from setup
//          |>1u99 2m88 3o09<| sent-out serial info
//          +----------------+

//---------------------------------------------------------
static bool LCDfound = false;
static bool LCDmodeSerial = true;

static uint8_t LCDposFixCol = 0;

// I2C connected to an LCD 16x2 module
const uint8_t LCD_COLS = 16;
const uint8_t LCD_LINES = 2;
const uint8_t DOTSIZE = 1;

const uint8_t LCD_I2C_ADDR = 0x27;  // I2C-Address = 0x27

LiquidCrystal_I2C lcd( LCD_I2C_ADDR, LCD_COLS, LCD_LINES); 

//---------------------------------------------------------
bool noI2Cconnected()
{
    // in absence of I2C SDA and SCL must be connected!
const int i2cPins[2] = { SCL, SDA };
    int failed = 0;
    for (int i = 0; i < 2; i++) {
    
        pinMode( i2cPins[i], INPUT);
        if (digitalRead( i2cPins[i]) != LOW)
        
        pinMode( i2cPins[1-i], OUTPUT);
        
        // put one of the connected outputs to LOW
        digitalWrite( i2cPins[1-i], LOW);

        // test other connected input to be LOW
        if (digitalRead( i2cPins[i]) != LOW)
            failed |= 1<<i;  // not LOW    
    }
    
    // SCL and SDA are not connected, both must be HIGH via pull-ups
    // reset both ports as inputs
    for (int i = 0; i < 2; i++) {
        pinMode( i2cPins[i], INPUT);
        if (digitalRead( i2cPins[i]) != HIGH)
            failed |= 1<<(2+i);     // not HIGH
    }
    
    return failed == 0;
}

//---------------------------------------------------------
void LCD_I2Csetup( int trinketNr, String msgHdr)
{
    uint8_t charmap[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; 
    uint8_t VUx[4] = { 0, 
        0b10000, // 1    '!'
        0b10100, // 2    '!!'
        0b10101  // 3    '!!!'
    };    
/*
    !
    ! !
    ! ! !
    ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
    00000011111122222233333444444
    01234501234501234501234501234
*/
    if (noI2Cconnected())
        return;
        
    LCDfound = true;
#ifdef DEBUG 
    LCDmodeSerial = false;
#warning "****** showing graphbar of open drawer ********"
#else
#warning "****** showing serial information generated for Feather ********"
#endif

    lcd.begin( LCD_COLS, LCD_LINES, DOTSIZE);

    if (! LCDmodeSerial) {
// LEVEL_MODE
//          +----------------+
//          |Tr#1,Cab/Drw: 1u|
//          |!!!!!!!       43|
//          +----------------+
        // setup char map used in showBar
        for (uint8_t i = 0; i < 4; i++) {
            for (int n = 0; n < 7; n++) {
                charmap[n] = VUx[i];
            }
            if (i==0) {
                // statt ganz leer: einzelner Punkt in der Mitte
                charmap[3] = 0b00100;
            }
            lcd.createChar( i, charmap);
        }
        lcd.home();

//          +----------------+
//          |TN1: Cab/Drw= 2m|
//          |90!!!!!!!!!!!!! |
//          +----------------+
    // 0123456701234567
    // TN1: Cab/Drw= 1u
    // TN1: Cab/Drw= 2m
    // TN1: Cab/Drw= 3o
    // print forever fix first line (except last 2 chars)
        lcd.print( "TN");
        lcd.print( (uint8_t)trinketNr);
        lcd.print( ": Cab/Drw= ..");     // last two chars will be updated!
        //                    !<-- posFix in first line
        LCDposFixCol = 14;
    }
    else {
        lcd.home();
        lcd.print( "TN");
        lcd.print( (uint8_t)trinketNr);
// SERIAL_MODE:
//          +----------------+
//          |TN1 1u 2umo 3umo| from setup
//          |>1u99 2m88 3o09<| sent-out serial info
//          +----------------+
            
        lcd.print( msgHdr);
        // 2nd line       
        lcd.setCursor( 1, 0);
        lcd.print( ">..............<");
    }
}

//---------------------------------------------------------
void LCDshowBar( uint8_t r01, int level)    // internal used only
{
    lcd.setCursor( 0, r01 % LCD_LINES);
    level = constrain(level, 0, 3*LCD_COLS);

    for (uint8_t i = 0; i < LCD_COLS; i++) {
        if (level >= 3) {
            level -= 3;
            lcd.write( 3);          // put out full-filled cell '!!!'
            continue;
        }
        lcd.write( (uint8_t)level);  // last cell '', '!' or '!!'
        level = 0;
    }
}
//---------------------------------------------------------
void LCDshowCabDrwBar( int cabNr, char drw, int percent)
{
    lcd.setCursor( LCDposFixCol, 0);
    lcd.print( (uint8_t)cabNr);         // put at end cabNr
    lcd.print( drw);                    // and "omu"
    // 
    int level = map( percent, 0,99, 0, 3*LCD_COLS);

    LCDshowBar( 1, level);      // 2nd line
    /////////

    // print exact percentage at end (or begin)
    int pos = (level < ((3*LCD_COLS)/10)*7) ? (LCD_COLS-2) : 0;        
    lcd.setCursor(1,pos);
    
    lcd.print( percent/10);     // 10th
    lcd.print( percent%10);     // units

    // 0123456701234567
    // Tr#1,Cab/Drw: 1u
    // !!!!!!!.......45
}

//---------------------------------------------------------
void LCDshowSerial( int nC, char *fourDigits)
{
// SERIAL_MODE:
//          +----------------+
//          |TN1 1u 2umo 3umo| from setup
//          |>1u99 2m88 3o09<| sent-out serial info
//          +----------------+
//           .1....6....1
    int pos = nC * 5 + 1;        
    lcd.setCursor( 1, pos);
    lcd.print( fourDigits);
}

//---------------------------------------------------------
