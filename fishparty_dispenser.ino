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
volatile int NbTopsFan; //measuring the rising edges of the signal
float Calc;    
float Vol;
int hallsensor = 2;    //The pin location of the sensor
 
//This is the function that the interupt calls 
void rpm () { 
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 

// The setup() method runs once, when the sketch starts
void setup() { 
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  attachInterrupt(1, rpm, RISING); //and the interrupt is attached
  Vol = 0.0f;
}

// the loop() method runs over and over again,
// as long as the Arduino has power
void loop () {
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
  cli();      //Disable interrupts
  Calc = (NbTopsFan / (73.0f * 60.0f)); //(Pulse frequency x 60) / 73Q, = flow rate in L/hour 
  Vol += (Calc * 1000.0);
  Serial.print(NbTopsFan, DEC);
  Serial.print(" ");
  Serial.print (Calc); //Prints the number calculated above
  Serial.print(" ");
  Serial.print(Vol);
  Serial.print (" ml\r\n"); //Prints "L/hour" and returns a  new line
}

// 18 - 19 L/Hour

// 250ml - 30secs
