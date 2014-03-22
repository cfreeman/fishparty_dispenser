/*
 * Copyright (c) Clinton Freeman 2014
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
#define __ASSERT_USE_STDERR
#include <Process.h>
#include <assert.h>

int NbTopsFan;                                       // The number of revolumeutions measured by the hall effect sensor.
unsigned long t;                                     // The last time that volumeume was measured.
volatile float volume;                               // The total volume measured by the flow sensor. Do not access directly,
                                                     // use getVolume to fetch the current dispensed volume.
char url_buffer[64] = "http://10.1.1.3:8080/0/?l=";  // The URL of the detector/hub.

static const float PARTY_DURATION = 10800000.0;      // The minimum duration of the party in milliseconds.
static const float TOTAL_VOLUME = 12000.00;          // The total volume of the drink dispenser in ml.
static const float SERVING_SIZE = 50.0;              // The size of a server in millilitres.
static const int BUTTON_PIN = 3;                     // The pin that the serving button sits on.
static const int LED_PIN = 4;                        // The pin that the serve ready LED indicator sits on.
static const int VALVE_PIN = 5;                      // The pin that the valve for dispensing drinks sits on.

/**
 * updateTankLevel transmits the current level of the dispenser to teh central detector/hub.
 *
 * current_dispenser_level is The current level of this beverage dispenser. 0.0f is empty while
 * 1.0f is full.
 */
void updateTankLevel(float current_dispenser_level) {
  assert(current_dispenser_level > 0.0f && current_dispenser_level < 1.0f);

  // Spin up a curl process on the atheros processor running linino.
  Process p;
  p.begin("curl");

  // Write the current level of the drink dispenser to the end of the URL.
  dtostrf(current_dispenser_level, 5, 5, &url_buffer[26]);
  p.addParameter(url_buffer); // Add the URL parameter to "curl"
  p.run();

  // Ignore the output from curl - we don't care about the server response.
}

/**
 * Callback method for pin2 interrupt. Updates the total volume that has been drained from
 * the drink dispenser. This method is called on each 'pulse' of the hall effect sensor in
 * the flow meter.
 */
void updatevolume () {
  unsigned long ct = millis();
  NbTopsFan++;  //Accumulate the pulses from the hall effect sesnors (rising edge).

  // Every ten spins of the flow sensor, calculate frequency and flow rate.
  if (NbTopsFan > 10) {
    float dV = (NbTopsFan / (0.073f * 60.0f)) * ((ct - t) / 1000.0f); // 73Q = 1L / Minute.
    volume += dV;

    NbTopsFan = 0;
    t = ct;
  }
}

/**
 * Returns the current volume (in ml) that has been poured by this beverage dispenser.
 */
float getVolume() {
  cli();
  float result = volume;
  sei();

  return result;
}

void dispenseBeverage() {
  // open the valve and start dispensing the drink.
  digitalWrite(LED_PIN, LOW);
  digitalWrite(VALVE_PIN, HIGH);
  digitalWrite(13, HIGH);

  float startVolume = getVolume();
  while ((getVolume() - startVolume) < SERVING_SIZE) {
    delay(10);
  }

  // close the valve and stop dispensing the drink.
  digitalWrite(13, LOW);
  digitalWrite(VALVE_PIN, LOW);

  // updateTankLevel(1.0 - (getVolume() / TOTAL_VOLUME));
}

/**
 * Arduino initalisation.
 */
void setup() {
  digitalWrite(13, HIGH);
  Bridge.begin();                            // Begin the bridge between the two processors on the Yun.
  pinMode(2, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);

  attachInterrupt(1, updatevolume, RISING);  // Sets pin 2 on the Arduino Yun as the interrupt.

  Serial.begin(9600);
  volume = 0.0f;
  t = millis();
  digitalWrite(13, LOW);
  digitalWrite(LED_PIN, HIGH);               // All powered up - show that we are ready to dispense a drink.
}

/**
 * Main Arduino loop.
 */
void loop () {
  if (digitalRead(BUTTON_PIN) == HIGH && digitalRead(LED_PIN) == HIGH) {
    dispenseBeverage();
  }

  if (digitalRead(LED_PIN) == LOW) {
      float duration = millis() / PARTY_DURATION;

      if (duration > (getVolume() / TOTAL_VOLUME)) {
        digitalWrite(LED_PIN, HIGH);
      }
  }
}
