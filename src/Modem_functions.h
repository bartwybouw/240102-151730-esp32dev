/**************************************************************
 * Bart Wybouw 31/12/2023
 * 
 * This file contains the Modem related functions and definitions
 *  - 
 *  - 
 *  - 
 *
 **************************************************************/
#ifndef MODEM_FUNCTIONS_H
#define MODEM_FUNCTIONS_H


// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet
// These defines are only for this example; they are not needed in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false


// set GSM PIN, if any
#define GSM_PIN ""

// Define Pins for modem
#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4
#define ADC_PIN 35
#define LOG_SWITCH_PIN 5 // Update with your logging switch pin

void modemPowerOn();
void modemPowerOff();
void modemRestart();


 // Modem functions
void modemPowerOn()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(1000);    //Datasheet Ton mintues = 1S
    digitalWrite(PWR_PIN, LOW);
}

void modemPowerOff()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(1500);    //Datasheet Ton mintues = 1.2S
    digitalWrite(PWR_PIN, LOW);
}

void modemRestart()
{
    modemPowerOff();
    delay(1000);
    modemPowerOn();
}
#endif // MODEM_FUNCTIONS_H