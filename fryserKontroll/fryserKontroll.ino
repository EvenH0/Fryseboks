#include <LiquidCrystal.h>
#include <avr/wdt.h>

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
  
  // Setup for ~8S WDT reset
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

}
