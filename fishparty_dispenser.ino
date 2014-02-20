/*
 * Copyright (c) Clinton Freeman 2013
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */ 
#include <Process.h> 

int NbTopsFan;          // The number of revolumeutions measured by the hall effect sensor.
unsigned long t;        // The last time that volumeume was measured.
volatile float volume;  // The total volume measured by the flow sensor.
float total_volume;     // The total volume of the drink dispenser in ml.


void updateTankLevel(float current_dispenser_level) {
  Process p;
  p.begin("curl");
  p.addParameter("http://arduino.cc/asciilogo.txt"); // Add the URL parameter to "curl"
  p.run();

  // TODO: Sort out URL structure of server running on tank (R-Pi).

  // Print arduino logo over the Serial
  // A process output can be read with the stream methods
  //while (p.available() > 0) {
  //  char c = p.read();
  //  Serial.print(c);
  //}

  // Ensure the last bit of data is sent.
  //Serial.flush();
}

void updatevolume () {
  unsigned long ct = millis();
  NbTopsFan++;  //Accumulate the pulses from the hall effect sesnors (rising edge).

  // Every ten spins of the flow sensor, calculate frequency and flow rate.
  if (NbTopsFan > 10) {
    float dV = (NbTopsFan / (0.073f * 60.0f)) * ((ct - t) / 1000.0f); // 73Q = 1L / Minute.
    volume += dV;
    
    // TODO: Update fishparty_tank of total volume dispensed.
    // updateTankLevel

    NbTopsFan = 0;
    t = ct;
  }
}

float getvolume() {
  cli();
  float result = volume;
  sei();  

  return result;
}

void dispenseBeverage() {
  // TODO: open valve.

  // TODO: While volume less than beverage allocation - keep dispensing.
  
  // TODO: close valve.    
}

/**
 * Arduino initalisation.
 */
void setup() { 
  Bridge.begin();                        // Begin the bridge between the two processors on the Yun.
  pinMode(2, INPUT);                     // Initializes digital pin 2 as an input
  attachInterrupt(1, updatevolume, RISING); // Sets pin 2 on the Arduino Yun as the interrupt.

  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  volume = 0.0f;
  t = millis();
}

/**
 * Main Arduino loop.
 */
void loop () {  
//  Serial.print(getvolume());
//  Serial.print (" ml\r\n");

  // TODO: If button press detected, dispense beverage.

  delay(100);
}
