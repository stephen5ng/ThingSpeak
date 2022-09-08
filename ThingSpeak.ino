#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

// ==========================================================================
//  Data Logging Options
// ==========================================================================
/** Start [logging_options] */

// Set the input and output pins for the logger
// NOTE:  Use -1 for pins that do not apply
const int32_t serialBaud = 115200; // Baud rate for debugging
const int8_t greenLED = 8;         // Pin for the green LED
const int8_t redLED = 9;           // Pin for the red LED
const int8_t buttonPin = 21;       // Pin for debugging mode (ie, button pin)
const int8_t wakePin = 31;         // MCU interrupt/alarm pin to wake from sleep
// Mayfly 0.x D31 = A7
// Set the wake pin to -1 if you do not want the main processor to sleep.
// In a SAMD system where you are using the built-in rtc, set wakePin to 1
const int8_t sdCardPwrPin = -1;   // MCU SD card power pin
const int8_t sdCardSSPin = 12;    // SD card chip select/slave select pin
const int8_t sensorPowerPin = 22; // MCU pin controlling main sensor power
/** End [logging_options] */

// ==========================================================================
//  Wifi/Cellular Modem Options
// ==========================================================================
/** Start [sim7080] */
// For almost anything based on the SIMCom SIM7080G
#include <modems/SIMComSIM7080.h>

#define modemSerial Serial1

// NOTE: Extra hardware and software serial ports are created in the "Settings
// for Additional Serial Ports" section
const int32_t modemBaud = 9600; //  SIM7080 does auto-bauding by default, but for simplicity we set to 9600

// Modem Pins - Describe the physical pin connection of your modem to your board
// NOTE:  Use -1 for pins that do not apply

const int8_t modemVccPin = 18;     // MCU pin controlling modem power --- Pin 18 is the power enable pin for the bee socket on Mayfly v1.0,
                                   //  use -1 if using Mayfly 0.5b or if the bee socket is constantly powered (ie you changed SJ18 on Mayfly1.0 to 3.3v)
const int8_t modemStatusPin = 19;  // MCU pin used to read modem status
const int8_t modemSleepRqPin = 23; // MCU pin for modem sleep/wake request
const int8_t modemLEDPin = redLED; // MCU pin connected an LED to show modem
                                   // status

// Network connection information
const char *apn = "hologram"; // APN connection name, typically Hologram unless you have a different provider's SIM card. Change as needed

// Create the modem object
SIMComSIM7080 modem7080(&modemSerial, modemVccPin, modemStatusPin,
                        modemSleepRqPin, apn);
// Create an extra reference to the modem by a generic name
SIMComSIM7080 modem = modem7080;
/** End [sim7080] */

// Weather station channel details
unsigned long weatherStationChannelNumber = SECRET_CH_ID_WEATHER_STATION;
unsigned int temperatureFieldNumber = 4;

// Counting channel details
unsigned long counterChannelNumber = SECRET_CH_ID_COUNTER;
const char *myCounterReadAPIKey = SECRET_READ_APIKEY_COUNTER;
unsigned int counterFieldNumber = 1;

void setup()
{
    Serial.begin(115200); // Initialize serial
    Serial.println("setup");

    modemSerial.begin(modemBaud);
    modem.setModemLED(modemLEDPin);
    modem.setModemWakeLevel(HIGH);  // ModuleFun Bee inverts the signal
    modem.setModemResetLevel(HIGH); // ModuleFun Bee inverts the signal
    Serial.println(F("Waking modem and setting Cellular Carrier Options..."));
    modem.modemWake();                  // NOTE:  This will also set up the modem
    modem.gsmModem.setBaud(modemBaud);  // Make sure we're *NOT* auto-bauding!
    modem.gsmModem.setNetworkMode(38);  // set to LTE only
    modem.gsmModem.setPreferredMode(1); // set to CAT-M

    ThingSpeak.begin(modem.gsmClient); // Initialize ThingSpeak
    Serial.println("setup done");
}

void loop()
{
    Serial.println("looping");

    if (!modem.modemWake())
    {
        Serial.println(F("Could not wake modem."));
        return;
    }
    if (!modem.connectInternet())
    {
        Serial.println(F("Could not connect to internet."));
        return;
    }

    // Read in field 4 of the public channel recording the temperature
    float temperatureInF = ThingSpeak.readFloatField(weatherStationChannelNumber, temperatureFieldNumber);

    // Check the status of the read operation to see if it was successful
    int statusCode = 0;
    statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.println("Temperature at MathWorks HQ: " + String(temperatureInF) + " deg F");
    }
    else
    {
        Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
    }

    delay(15000); // No need to read the temperature too often.

    // Read in field 1 of the private channel which is a counter
    long count = ThingSpeak.readLongField(counterChannelNumber, counterFieldNumber, myCounterReadAPIKey);

    // Check the status of the read operation to see if it was successful
    statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200)
    {
        Serial.println("Counter: " + String(count));
    }
    else
    {
        Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
    }

    delay(15000); // No need to read the counter too often.
}