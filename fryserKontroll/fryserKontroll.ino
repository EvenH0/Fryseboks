#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <avr/wdt.h>

// Constants
#define FREEZER_RELAY                  9
#define HEATER_RELAY                  10
#define REGULATOR_INTERVAL          3600
#define ONE_WIRE_BUS                  13
#define TEMPERATURE_PRECISION          9

// Global Variables
int ambientTemp                   =    0;
int compressorTemp                =    0;
int freezerTemp                   =    0;
int timer                         =    0;

// initialize the LiquidCrystal library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress AMBIENT_TEMP_SERNSOR, COMPRESSOR_TEMP_SENSOR, FREEZER_TEMP_SENSOR;

void setup() {
  // LCD setup
  lcd.begin(16, 2);
  lcd.print("Ute:");
  lcd.setCursor(7, 0);
  lcd.print("Boks:");
  lcd.setCursor(0, 1);
  lcd.print("Kompressor:");

  // Setup serial for status dump
  Serial.begin(9600);

  // One Wire temp sensors
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin();
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (!sensors.getAddress(AMBIENT_TEMP_SERNSOR,   0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(COMPRESSOR_TEMP_SENSOR, 1)) Serial.println("Unable to find address for Device 1"); 
  if (!sensors.getAddress(FREEZER_TEMP_SENSOR,    2)) Serial.println("Unable to find address for Device 2"); 
  // set the resolution to 9 bit
  sensors.setResolution(AMBIENT_TEMP_SERNSOR,     TEMPERATURE_PRECISION);
  sensors.setResolution(COMPRESSOR_TEMP_SENSOR,   TEMPERATURE_PRECISION);
  sensors.setResolution(FREEZER_TEMP_SENSOR,      TEMPERATURE_PRECISION);
  
  // Pin setup
  pinMode(FREEZER_RELAY, OUTPUT);
  pinMode(HEATER_RELAY , OUTPUT);
  
  // Freezer ON
  // N.C relay
  digitalWrite(FREEZER_RELAY, HIGH);
  
  // Heater OFF
  // N.O relay
  digitalWrite(HEATER_RELAY, LOW);
  
  // Setup for ~8S WD reset
  noInterrupts();
  wdt_reset();
  MCUSR &= ~bit(WDRF);                            //clear WDRF
  WDTCSR |= bit(WDCE) | bit(WDE);                 //enable WDTCSR change
  WDTCSR =  bit(WDIE) | bit(WDP3) | bit(WDP0);    //~8 sec
  interrupts();

  Serial.println("Setup complete!");

}

void loop() 
{
  // Keep WD from barking!
  wdt_reset();

  // Read temp
  sensors.requestTemperatures();   
  ambientTemp     = sensors.getTempC(AMBIENT_TEMP_SERNSOR);
  compressorTemp  = sensors.getTempC(COMPRESSOR_TEMP_SENSOR);
  freezerTemp     = sensors.getTempC(FREEZER_TEMP_SENSOR);

  // Regulate
  if (timer >= REGULATOR_INTERVAL)
  {
    timer = 0;
    freezerControl();
  }
  
  // Update Display
  //@ TODO: Limit decimals when printing
  lcd.setCursor(4, 0);
  lcd.print(ambientTemp);
  lcd.setCursor(12, 0);
  lcd.print(freezerTemp);
  lcd.setCursor(12, 1);
  lcd.print(compressorTemp);

  // Hang a bit, get loop time ~1sec
  delay(900);
  ++timer;
}

void freezerControl()
{
  if (ambientTemp <= -15)
  {
    // It's cold outside turn off the freezer
    // N.C relay
    digitalWrite(FREEZER_RELAY, HIGH);
  }
  else if (ambientTemp <= -4 && freezerTemp <= -14)
  {
    // It's cold inside turn off the freezer
    // N.C relay
    digitalWrite(FREEZER_RELAY, HIGH);
  }
  else 
  {
    // Need Heater?
    // If heating wait 10min and try again
    if (!heaterControl())
    {
      // It's time to FREEZE!
      // N.C relay
      digitalWrite(FREEZER_RELAY, LOW);
    }
  }
}

bool heaterControl()
{
  bool heating = false;
  if (compressorTemp <= -4)
  {
    digitalWrite(HEATER_RELAY, HIGH);
    heating = true;
    // Heat for 10 min and check again.
    timer = (REGULATOR_INTERVAL - 600);
  }
  else
  {
    digitalWrite(HEATER_RELAY, LOW);
  }
  return heating;
}

