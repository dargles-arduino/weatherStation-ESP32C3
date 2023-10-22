/**
 * Class definition for OLEDscreen
 * 
 * This OLEDscreen class provides methods to send text to a small OLED 
 * screen, in this case, a:
 * - monochrome OLED with a 
 * - SSD1306 driver,
 * - 128x64 pixel display, and 
 * - using I2C to communicate with 2 pins (SDA & SCL).
 * Reset isn't available on my screen, nor does it appear to be required.
 *
 * Adafruit invests time and resources providing this open source code, 
 * please support Adafruit and open-source hardware by purchasing products 
 * from Adafruit!
 *
 * The original code was written by Limor Fried/Ladyada for Adafruit 
 * Industries, with contributions from the open source community.
 * Modified by David Argles, 18oct2023
 * BSD license, check license.txt for more information.
 ************************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH   128  // OLED display width, in pixels
#define SCREEN_HEIGHT  64   // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Can find the I2C address from the (OLED)test code
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

class OLEDscreen
{
  private:

  public:

  // The constructor
  void begin()
  {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) Serial.println(F("SSD1306 allocation failed"));

    // Clear the buffer
    display.clearDisplay();
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    // Welcome message
    display.println("OLED screen active...");
    display.display();
  }

  void print(String sometext)
  {
    display.print(sometext);
    display.display();
    return;
  }

  void clear()
  {
    display.clearDisplay();
    display.setCursor(0, 0);     // Start at top-left corner
    display.display();
    return;
  }

void banner(String prog, String ver, String build)
  {
    display.clearDisplay();
    display.println();
    display.println(prog);
    display.print("Version: ");
    display.println(ver);
    display.println(build);
    display.display();
    return;
  }
};