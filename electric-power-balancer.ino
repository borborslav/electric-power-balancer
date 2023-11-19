/*
Copyright (c) 2021 Jakub Mandula

Example of using one PZEM module with Software Serial interface.
================================================================

If only RX and TX pins are passed to the constructor, software 
serial interface will be used for communication with the module.

*/

/*
 * перевірити струм кожну секунду і якщо струм менше (i+відключена нагрузка) протягом ton секунд то включити нагрузку
 * якщо струм більше imax то виключити нагрузку
 * якщо струм більше inominal протягом toff секунд то виключити нагрузку
 * 
 * 
 *  
 */

# define o1 4             // out#1 pin  boller    2000
# define o2 5             // out#2 pin  bedroom   1531+752+571 = 2854
# define o3 6             // out#3 pin  bedroom2  1929
# define o4 7             // out#4 pin  kitchen   1531
# define o5 8             // out#5 pin  holl, wash, WC 1068 + 444 + 129 = 1641
# define o6 A0             // out#6 pin
# define inominal 25      // nominal current, A 
# define imax 32          // max current, A
# define ton 60           // time to on load, S
# define toff 60          // time to off load, S
# define numberofouts 6   // numbers of load

int tofftimer = toff;
int tontimer = ton;
byte outs[numberofouts] = {0};
float outscurrent[numberofouts] = {9, 12.9, 8.77, 7, 7.45};
byte tout = 0;
byte loads = 0; // current number of ON loads


#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

#if defined(ESP32)
    #error "Software Serial is not supported on the ESP32"
#endif

/* Use software serial for the PZEM
 * Pin 12 Rx (Connects to the Tx pin on the PZEM)
 * Pin 13 Tx (Connects to the Rx pin on the PZEM)
*/
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 12
#define PZEM_TX_PIN 13
#endif


SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);


void setpins()
{
  digitalWrite (o1, outs[1]);
  digitalWrite (o2, outs[2]);
  digitalWrite (o3, outs[3]);
  digitalWrite (o4, outs[4]);
  digitalWrite (o5, outs[5]);
  digitalWrite (o6, outs[6]);
}

void offload()
{
  outs[loads] = 0; // last load OFF
  loads--;
  setpins();
}

void onload()
{
  if (loads < numberofouts)
  {
    loads++;
    outs[loads] = 1;
  }
 
  setpins();
}

void setup() {
    /* Debugging serial */
    Serial.begin(9600);
    pinMode(o1, OUTPUT);
    pinMode(o2, OUTPUT);
    pinMode(o3, OUTPUT);
    pinMode(o4, OUTPUT);
    pinMode(o5, OUTPUT);
    pinMode(o6, OUTPUT);
    digitalWrite(o1, LOW);
    digitalWrite(o2, LOW);
    digitalWrite(o3, LOW);
    digitalWrite(o4, LOW);
    digitalWrite(o5, LOW);
    digitalWrite(o6, LOW);
}

void loop() {
         
    Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    // Check if the data is valid
    if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
    } else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);
    }

    Serial.println();

    
    if (current > imax) offload();
    if (current > inominal)
    {
      if (tofftimer > 0) tofftimer --;
      else offload();
      tontimer = ton;
    }
    if (current <= inominal)
    {
      tofftimer = toff;
      if (tontimer > 0) tofftimer --;
      else
      {
        if (loads < numberofouts)
        if (current + outscurrent[loads+1] < inominal) onload();
      }
    }
    
    delay(2000);
}
