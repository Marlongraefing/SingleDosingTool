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
const int MOTOR_PIN   = GPIO_NUM_17;
const float TARGET_WEIGHT_GRAMS = 16.0;

/**
 * Calibration factor
 * - Value is raw units per gram
 * - Adjust after calibration for your load cell
 */
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
  pinMode(MOTOR_PIN, OUTPUT);

  scale.begin();
  scale.setGain(GAIN_128);
  scale.setTimeout(1000);
  scale.setScale(CALIBRATION_FACTOR);
  Serial.println("Taring... remove any load");
  delay(2000);
  scale.tare(10);   /* uses default sample count */
  Serial.println("Tare done.");
  Serial.println("Starting dispense.");
  digitalWrite(MOTOR_PIN, HIGH);
}


void runMotorForMiliseceonds(int miliseconds){
  digitalWrite(MOTOR_PIN, HIGH);
  delay(miliseconds);
  digitalWrite(MOTOR_PIN, LOW);
}

void runMotorUntilWeight(float targetWeight){
  digitalWrite(MOTOR_PIN, HIGH);
  while(true){
    float weight = scale.getWeight(1);
    Serial.println(weight);
    if(weight >= targetWeight){
      digitalWrite(MOTOR_PIN, LOW);
      return;
    }
  }
}

enum class Mode{
  fast_dispense,
  precise_dispense,
  done
};

Mode mode = Mode::fast_dispense;

void loop() {
  switch(mode){
    case Mode::fast_dispense:{
      runMotorUntilWeight(TARGET_WEIGHT_GRAMS*0.85);
      mode = Mode::precise_dispense;
      break;
    }
    case Mode::precise_dispense:{
      runMotorForMiliseceonds(200);
      delay(150);
      float weight = scale.getWeight(5);
      Serial.println(weight);
      if(weight >= TARGET_WEIGHT_GRAMS){
        mode = Mode::done;
        Serial.println("Dispense complete.");
      }
      break;
    }
    case Mode::done:{
      float weight = scale.getWeight(10);
      Serial.println(weight);
      break;
    }
  }
  

}
