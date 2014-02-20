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
int NbTopsFan;       // The number of revolutions measured by the hall effect sensor.
unsigned long t;     // The last time that volume was measured.
volatile float Vol;  // The total volume measured by the flow sensor.

void updateVol () {
  unsigned long ct = millis();
  NbTopsFan++;  //Accumulate the pulses from the hall effect sesnors (rising edge).

  // Every ten spins of the flow sensor, calculate frequency and flow rate.
  if (NbTopsFan > 10) {
    float dV = (NbTopsFan / (0.073f * 60.0f)) * ((ct - t) / 1000.0f); // 73Q = 1L / Minute.
    Vol += dV;

    NbTopsFan = 0;
    t = ct;
  }
}

float getVol() {
  cli();
  float result = Vol;
  sei();  

  return result;
}

/**
 * Arduino initalisation.
 */
void setup() { 
  pinMode(2, INPUT);                     // Initializes digital pin 2 as an input
  attachInterrupt(1, updateVol, RISING); // Sets pin 2 on the Arduino Yun as the interrupt.

  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  Vol = 0.0f;
  t = millis();
}

/**
 * Main Arduino loop.
 */
void loop () {  
  Serial.print(getVol());
  Serial.print (" ml\r\n");

  delay(100);
}
