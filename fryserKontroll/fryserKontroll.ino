#include <LiquidCrystal.h>
#include <avr/wdt.h>

// Constants
//@ TODO: update with correct pins
const int AMBIENT_TEMP_SERNSOR    =    0;
const int COMPRESSOR_TEMP_SENSOR  =    0;
const int FREEZER                 =    9;
const int HEATER                  =   10;
const int REGULATOR_INTERVAL      = 3600;

// Variables
int ambientTemp                   =    0;
int compressorTemp                =    0;
int timing                        =    0;
// initialize the LiquidCrystal library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup() {
  // LCD setup
  lcd.begin(16, 2);
  lcd.print("Ute:");
  lcd.setCursor(0, 1);
  lcd.print("Kompressor:");

  // Setup serial for status dump
  Serial.begin(9600);

  // Pin setup
  pinMode( 9, OUTPUT);
  pinMode(10, OUTPUT);
  
  // Setup for ~8S WD reset
  noInterrupts();
  wdt_reset();
  MCUSR &= ~bit(WDRF);                            //clear WDRF
  WDTCSR |= bit(WDCE) | bit(WDE);                 //enable WDTCSR change
  WDTCSR =  bit(WDIE) | bit(WDP3) | bit(WDP0);    //~8 sec
  interrupts();

}

void loop() 
{
  // Keep WD from barking!
  wdt_reset();

  // Read temp
  //@ TODO: get temp sensors
  ambientTemp = 0;
  compressorTemp = 0;

  // 
  if (ambientTemp <= -15)
  {
    // Freezer relay is N.C
    digitalWrite(FREEZER, HIGH);
  }
  else 
  {
    heaterControl();
  }

  // Update Display
  lcd.setCursor(12, 0);
  lcd.print(ambientTemp);
  lcd.setCursor(12, 1);
  lcd.print(compressorTemp);

  // Hang a bit
  delay(1000);
}

void heaterControl()
{
  ++timing;
  if (timing >= REGULATOR_INTERVAL)
  {
    
  }
}

