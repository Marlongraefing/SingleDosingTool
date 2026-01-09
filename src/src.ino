/*
 * HX711 Load Cell Scale Factor Calibration Note
 *
 * The scale factor is used to convert raw ADC values from the HX711
 * into grams.
 *
 * Calibration Steps:
 * 1. Tare the scale (set it to zero with no weight).
 * 2. Place a known weight on the load cell (e.g., 1000g).
 * 3. Observe the raw reading from the HX711.
 * 4. Calculate the scale factor using:
 *      scale_factor = raw_reading / known_weight
 * 5. Replace this value eith scale_factor:
 *      scale.set_scale(scale_factor);
 *
 * Tips:
 * - Use accurate, known weights for calibration.
 * - Make sure the load cell is mounted securely during calibration.
 * - Recalibrate if the physical setup changes.
 */
#include <Arduino.h>
#include "7semi_HX711.h"

/**
 * Pin configuration
 * - HX_DOUT_PIN: HX711 DOUT pin
 * - HX_SCK_PIN : HX711 SCK pin
 */
const int HX_DOUT_PIN = GPIO_NUM_19;
const int HX_SCK_PIN  = GPIO_NUM_18;

/**
 * Calibration factor
 * - Value is raw units per gram
 * - Adjust after calibration for your load cell
 */
//86.38
float CALIBRATION_FACTOR = 552.6;

/**
 * HX711 instance
 * - Uses data and clock pins
 */
HX711_7semi scale(HX_DOUT_PIN, HX_SCK_PIN);

/**
 * Setup
 * - Start serial
 * - Initialize HX711
 * - Apply calibration and tare
 */
void setup() {
  Serial.begin(9600);
  pinMode(GPIO_NUM_17, OUTPUT);

  scale.begin();
  scale.setGain(GAIN_64);
  scale.setTimeout(1000);
  scale.setScale(CALIBRATION_FACTOR);

  Serial.println("HX711 simple example");
  Serial.println("Taring... remove any load");
  delay(2000);

  scale.tare();   /* uses default sample count */

  Serial.println("Tare done.");
  Serial.println("Place weight on the load cell.");
}

/**
 * Main loop
 * - Read weight in grams
 * - Print to serial monitor
 */
bool motor = false;

void loop() {
  float weight = scale.getWeight(1);  /* uses default averaging */
  if(motor){
    digitalWrite(GPIO_NUM_17, HIGH);
    motor = false;
  } else {
    digitalWrite(GPIO_NUM_17, LOW);
    motor = true;
  }

  //Serial.print("Weight: ");
  Serial.println(weight, 2);
  //Serial.println(" g");

}
