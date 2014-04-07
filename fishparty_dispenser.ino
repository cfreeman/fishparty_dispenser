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

int NbTopsFan;                                          // The number of revolutions measured by the hall effect sensor.
unsigned long t;                                        // The last time that volume was measured.
volatile float volume;                                  // The total volume measured by the flow sensor.
                                                        // ** WARNING: ******************************************************************
                                                        // Do not access directly, use getVolume() to fetch the current dispensed volume.

char url_buffer[64] = "http://192.168.0.1:8080/0/?l=";  // The URL of the detector/hub. -- This is uniquely identfying for each dispenser.
                                                        // ** WARNING: ******************************************************************
                                                        // If you make any tweaks to the arduino code, each dispenser needs to be updated
                                                        // with a different URL:
                                                        // Dispenser 192.168.0.2 uses url_buffer[64] = "http://192.168.0.1:8080/0/?l=";
                                                        // Dispenser 192.168.0.3 uses url_buffer[64] = "http://192.168.0.1:8080/1/?l=";
                                                        // Dispenser 192.168.0.4 uses url_buffer[64] = "http://192.168.0.1:8080/2/?l=";
                                                        // Dispenser 192.168.0.5 uses url_buffer[64] = "http://192.168.0.1:8080/3/?l=";

static const int SERVING_DELAY = 3000;                  // The number of milliseconds to wait when the valve is opening, and then closing.
                                                        // I.e. The valve will be open for 3000 milliseconds, before being told to shut.
                                                        // dispense beverage will then wait another 3000 milliseconds to ensure that the valve
                                                        // is fully closed.

static const float PARTY_DURATION = 10800000.0;         // The minimum duration of the party in milliseconds.
static const float TOTAL_VOLUME = 13000.00;             // The total volume of the drink dispenser in ml.
static const int BUTTON_PIN = 3;                        // The pin that the serving button sits on.
static const int LED_PIN = 4;                           // The pin that the serve ready LED indicator sits on.
static const int VALVE_PIN = 5;                         // The pin that the valve for dispensing drinks sits on.

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

  // If we have not recieved any pulses for the last 500ms, restart the flow calculation.
  // The valve for the sensor must have been closed.
  if ((ct - t) > 500) {
    NbTopsFan = 0;
    t = ct;
  }

  // Every ten spins of the flow sensor, calculate frequency and flow rate.
  if (NbTopsFan > 20) {
    // Old calibration value: 0.073
    float dV = (NbTopsFan / (0.01825f * 60.0f)) * ((ct - t) / 1000.0f); // 73Q = 1L / Minute.
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
  // Open the valve and start dispensing the drink.
  float startVolume = getVolume();
  Serial.print("Start: ");
  Serial.print(startVolume);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(VALVE_PIN, HIGH);
  digitalWrite(13, HIGH);

  // Wait for serve to dispense before closing valve.
  delay(SERVING_DELAY);

  // Close the valve and stop dispensing the drink.
  digitalWrite(13, LOW);
  digitalWrite(VALVE_PIN, LOW);

  // Wait for the valve to close.
  delay(SERVING_DELAY);

  // Wait for the valve to close;
  Serial.print(" closed: ");
  Serial.print(getVolume());
  Serial.print(" Total: ");
  Serial.println(1.0 - (getVolume() / TOTAL_VOLUME));

  updateTankLevel(1.0 - (getVolume() / TOTAL_VOLUME));
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

  attachInterrupt(1, updatevolume, RISING);  // Sets pin 2 on the Arduino Yun as the interrupt. (from flow meter).

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
  // If we are ready to serve, and someone has pushed the drink button - dispense a beverage.
  if (digitalRead(BUTTON_PIN) == HIGH && digitalRead(LED_PIN) == HIGH) {
    dispenseBeverage();
  }

  // If the ready to serve indicator is off - work out if we are ready to serve again.
  if (digitalRead(LED_PIN) == LOW) {
    float duration = sqrt(millis() / PARTY_DURATION);    // Quadratic beverage dispense rate.

    if (duration > (getVolume() / TOTAL_VOLUME)) {
      digitalWrite(LED_PIN, HIGH);
    }
  }
}
