#include <Arduino.h>
#include "Controller.h"
#include "ButtonGroup.h"
#include <LovyanGFX.h>
#include <config.hpp>
#include <Adafruit_NeoPixel.h>

// Define specific board
#define LGFX_FEATHER_ESP32_S3_TFT
static LGFX lcd;
// Create the Sprite instances
static LGFX_Sprite _sprites[2] = { LGFX_Sprite(&lcd), LGFX_Sprite(&lcd) };

// Pin definitions
#define LEFT_BUTTON_PIN 21
#define RIGHT_BUTTON_PIN 26
#define Bottom_BUTTON_PIN 33

#define LED_PIN 46
#define LED_COUNT 3

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ButtonGroup buttonGroup;
Controller controller(lcd,
                      &_sprites[0], 
                      &_sprites[1],
                      buttonGroup,
                      pixels
                    );

void setup() {
    buttonGroup.setLeftPin(LEFT_BUTTON_PIN);
    buttonGroup.setRightPin(RIGHT_BUTTON_PIN);
    buttonGroup.setBottomPin(Bottom_BUTTON_PIN);

    controller.begin();
}

void loop() {
  controller.service();
}