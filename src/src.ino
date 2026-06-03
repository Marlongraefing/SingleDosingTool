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
#include "7semi_HX711.h"
#include <AccelStepper.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/**
 * Pin configuration
 * - HX_DOUT_PIN : HX711 DOUT pin
 * - HX_SCK_PIN  : HX711 SCK pin
 * - STEPPER_IN1-4: ULN2003 stepper driver inputs (D2-D5)
 */
const int HX_DOUT_PIN = 10;
const int HX_SCK_PIN  = 11;
const float TARGET_WEIGHT_GRAMS = 16.0;

// 28BYJ-48 stepper via ULN2003 on D2-D5
// Wiring: D2=IN4, D3=IN3, D4=IN2, D5=IN1
// AccelStepper HALF4WIRE pin order: IN1,IN3,IN2,IN4 → D5,D3,D4,D2
const int STEPS_PER_REV = 4096;   // half-step mode (smoother)
const int MOTOR_RPM     = 15;
// 15 RPM * 4096 steps/rev / 60 s = ~1024 steps/s
const float MOTOR_SPEED_SPS = (float)MOTOR_RPM * STEPS_PER_REV / 60.0;
AccelStepper stepper(AccelStepper::HALF4WIRE, 5, 3, 4, 2);

/**
 * Calibration factor
 * - Value is raw units per gram
 * - Adjust after calibration for your load cell
 */

 //TODO: needs to be fixed
float CALIBRATION_FACTOR = 1104;

/**
 * HX711 instance
 * - Uses data and clock pins
 */
HX711_7semi scale(HX_DOUT_PIN, HX_SCK_PIN);

// Power off stepper coils to prevent heating when idle
void disableStepper() {
  stepper.disableOutputs();
}

void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(MOTOR_SPEED_SPS);
  stepper.setSpeed(MOTOR_SPEED_SPS);

  scale.begin();
  scale.setGain(GAIN_128);
  scale.setTimeout(1000);
  scale.setScale(CALIBRATION_FACTOR);
  // Wait for display
  delay(500);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);  

  display.setCursor(0, 0);
  display.println(F("Place reservoir,"));
  display.println(F("then wait..."));
  display.display();

  delay(4000);
  scale.tare(10);   /* tare with full reservoir */

  display.clearDisplay();
  display.setCursor(0, 0);  
  display.println("Tare done.");
  display.println("Starting dispense.");
  display.display();

}

void drawWeights(float target, float dispensed) {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);   
  display.println(F("Target:"));
  display.setCursor(0, 16);
  display.setTextSize(2);             // Draw 2X-scale text
  display.println(target);

  display.setCursor(64, 0);
  display.setTextSize(1);
  display.println(F("Dispensed:"));
  display.setCursor(64, 16);
  display.setTextSize(2);             // Draw 2X-scale text
  if (dispensed < 10){
    display.print(' ');
  }
  display.println(dispensed);

  display.display();

  Serial.print("dispensed:");
  Serial.print(dispensed);
  Serial.print("g target:");
  Serial.print(target);
  Serial.println("g");
}


enum class Mode{
  fast_dispense,
  done
};

Mode mode = Mode::fast_dispense;

void loop() {

  switch(mode){
    case Mode::fast_dispense:{
      stepper.runSpeed();  // keep motor stepping as fast as possible

      // only read scale when HX711 has new data ready (DOUT LOW)
      if (digitalRead(HX_DOUT_PIN) == LOW) {
        float dispensed = -scale.getWeight(1);
        drawWeights(TARGET_WEIGHT_GRAMS, dispensed);
        if (dispensed >= TARGET_WEIGHT_GRAMS) {
          disableStepper();
          mode = Mode::done;
          Serial.println("Dispense complete.");
        }
      }
      break;
    }
    case Mode::done:{
      if (digitalRead(HX_DOUT_PIN) == LOW) {
        float dispensed = -scale.getWeight(10);
        drawWeights(TARGET_WEIGHT_GRAMS, dispensed);
      }
      break;
    }
  }

}
