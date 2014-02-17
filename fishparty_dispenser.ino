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
int NbTopsFan; //measuring the rising edges of the signal
unsigned long t;  // The last time that volume was measured.

volatile float Vol;

void updateVol () {
  unsigned long ct = millis();
  NbTopsFan++;  //Accumulate the pulses from the hall effect sesnors (rising edge).

  // Every second measure the frequency of the hall effect sensors and calculate change
  // in volume.
  
  // TODO: Change volume calcs to be based off the number of ticks, rather than the time interval that has passed.
  // This will make algo update faster when flow is faster, and slower when flow is slower. Rather than the fixed
  // 1second updates at the moment.
  
  if (ct > (t + 1000)) {
    float dV = (NbTopsFan / (0.073f * 60.0f)); // 73Q, = flow rate in L/min
    Vol += dV;

    NbTopsFan = 0;
    t = ct;
  }
}

float getVol() {
  float result = 0.0;  
  cli();
  result = Vol;
  sei();  

  return result;
}

/**
 * Arduino initalisation.
 */
void setup() { 
  pinMode(2, INPUT); //initializes digital pin 2 as an input
  attachInterrupt(1, updateVol, RISING); // Sets pin 2 on the Arduino Yun as the interrupt.

  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  Vol = 0.0f;
  t = millis();
}

/**
 * Main Arduino loop.
 */
void loop () {
  
  delay(7);

  Serial.print(getVol());
  Serial.print (" ml\r\n");

  delay(100);
}
