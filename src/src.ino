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
 * 5. Replace this value with scale_factor:
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
 * - HX_DOUT_PIN : HX711 DOUT pin (A1 based on updated PinLayout.md)
 * - HX_SCK_PIN  : HX711 SCK pin (A2 based on updated PinLayout.md)
 */
const int HX_DOUT_PIN = A1;
const int HX_SCK_PIN  = A2;
const float TARGET_WEIGHT_GRAMS = 16.0;

// Buttons & Mill Relay (Pins updated based on PinLayout.md)
const int MILL_BUTTON_PIN = 2; // Supports hardware interrupt INT0
const int MILL_RELAY_PIN  = 12;

// Stepper Motor Definitions & Grouping
const int STEPS_PER_REV = 2048;   // full-step mode (provides higher torque)
const int MOTOR_RPM     = 12;      // slightly reduced RPM for significantly more torque
// 12 RPM * 2048 steps/rev / 60 s = ~410 steps/s
const float MOTOR_SPEED_SPS = (float)MOTOR_RPM * STEPS_PER_REV / 60.0;

// Motor 1: 28BYJ-48 stepper via ULN2003 on D4-D7
// Wiring: D4=IN4, D5=IN3, D6=IN2, D7=IN1 (Reversed order in PinLayout.md!)
// AccelStepper FULL4WIRE pin order: IN1,IN3,IN2,IN4 → D7,D5,D6,D4
AccelStepper stepper(AccelStepper::FULL4WIRE, 7, 5, 6, 4);

// Motor 2: 28BYJ-48 stepper via ULN2003 on D8-D11
// Wiring: D8=IN1, D9=IN2, D10=IN3, D11=IN4
// AccelStepper FULL4WIRE pin order: IN1,IN3,IN2,IN4 → D8,D10,D9,D11
AccelStepper stepper2(AccelStepper::FULL4WIRE, 8, 10, 9, 11);

/**
 * Calibration factor
 * - Value is raw units per gram
 * - Adjust after calibration for your load cell
 */

// Adjusted based on actual vs reported measurements:
// Formula: 1104 * (16.67 / 17.7) = ~1039.8
float CALIBRATION_FACTOR = 1039.8;

#include "WeightRegressor.h"

WeightRegressor regressor;

/**
 * HX711 instance
 * - Uses data and clock pins
 */
HX711_7semi scale(HX_DOUT_PIN, HX_SCK_PIN);

// Power off stepper coils to prevent heating when idle
void disableStepper() {
  stepper.disableOutputs();
  stepper2.disableOutputs();
}

void setup() {
  Serial.begin(115200); // Increased from 9600 to 115200 to prevent Serial buffer blocking
  
  // Set up both motor drivers
  stepper.setMaxSpeed(MOTOR_SPEED_SPS);
  stepper.setSpeed(MOTOR_SPEED_SPS);
  stepper2.setMaxSpeed(MOTOR_SPEED_SPS);
  stepper2.setSpeed(MOTOR_SPEED_SPS);

  // Set up Button and Relay Pins
  pinMode(MILL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MILL_RELAY_PIN, OUTPUT);
  digitalWrite(MILL_RELAY_PIN, LOW); // Start with Relay disabled (Safe!)

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
  Wire.setClock(400000); // Speed up I2C to 400kHz to minimize stepper stepping interruptions
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);  

  display.setCursor(0, 0);
  display.println(F("Place reservoir,"));
  display.println(F("then wait..."));
  display.display();

  delay(4000);
  scale.tare(10);   /* tare with full reservoir */
  regressor.reset();

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
}


enum class Mode{
  fast_dispense,
  done
};

Mode mode = Mode::fast_dispense;
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 250; // Update display every 250ms during dispense

void loop() {

  switch(mode){
    case Mode::fast_dispense:{
      stepper.runSpeed();   // keep motor 1 stepping as fast as possible
      stepper2.runSpeed();  // keep motor 2 stepping as fast as possible

      // only read scale when HX711 has new data ready (DOUT LOW)
      if (digitalRead(HX_DOUT_PIN) == LOW) {
        float dispensedRaw = -scale.getWeight(1);
        unsigned long currentMillis = millis();

        // Feed measurement into linear regressor
        regressor.addSample(currentMillis, dispensedRaw);

        // Get predicted weight filtering out random high frequency vibrations
        float dispensedPredicted = regressor.predict(currentMillis);
        
        if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
          drawWeights(TARGET_WEIGHT_GRAMS, dispensedPredicted);
          lastDisplayUpdate = currentMillis;
        }

        // Print raw and predicted values for plotting / monitoring
        // This is safe even at high frequencies now thanks to 115200 Baud!
        Serial.print("raw:");
        Serial.print(dispensedRaw, 2);
        Serial.print(",predicted:");
        Serial.println(dispensedPredicted, 2);

        if (dispensedPredicted >= TARGET_WEIGHT_GRAMS) {
          disableStepper();
          mode = Mode::done;
          drawWeights(TARGET_WEIGHT_GRAMS, dispensedPredicted); // Ensure final exact weight is displayed
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
