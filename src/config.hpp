#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// Configuration example for using LovyanGFX with custom settings on ESP32

/*
 Duplicate this file, rename it, and change the settings to match your environment.
 By including the created file from your user program, it becomes available for use.

 You can place the duplicated file in the lgfx_user folder of the library,
 but be aware that it might be deleted when the library is updated.

 If you want to operate safely, create a backup or place it in your user project folder.
//*/


/// Create a class that performs custom settings, derived from LGFX_Device.
class LGFX : public lgfx::LGFX_Device
{
/*
 You can change the class name from "LGFX" to another name.
 If using with AUTODETECT, change it to something other than "LGFX" as "LGFX" is already used.
 Also, if using multiple panels simultaneously, give each a different name.
 * If you change the class name, you must also change the constructor name to match.

 You can name it freely, but assuming the number of configurations increases,
 for example, if you configure ILI9341 with SPI connection on ESP32 DevKit-C,
 naming it like LGFX_DevKitC_SPI_ILI9341 and matching the file name with the class name
 makes it less confusing when using it.
//*/


// Prepare an instance that matches the type of panel you are connecting.
//lgfx::Panel_GC9A01      _panel_instance;
//lgfx::Panel_GDEW0154M09 _panel_instance;
//lgfx::Panel_HX8357B     _panel_instance;
//lgfx::Panel_HX8357D     _panel_instance;
//lgfx::Panel_ILI9163     _panel_instance;
//lgfx::Panel_ILI9341     _panel_instance;
//lgfx::Panel_ILI9342     _panel_instance;
//lgfx::Panel_ILI9481     _panel_instance;
//lgfx::Panel_ILI9486     _panel_instance;
//lgfx::Panel_ILI9488     _panel_instance;
//lgfx::Panel_IT8951      _panel_instance;
//lgfx::Panel_RA8875      _panel_instance;
//lgfx::Panel_SH110x      _panel_instance; // SH1106, SH1107
//lgfx::Panel_SSD1306     _panel_instance;
//lgfx::Panel_SSD1327     _panel_instance;
//lgfx::Panel_SSD1331     _panel_instance;
//lgfx::Panel_SSD1351     _panel_instance; // SSD1351, SSD1357
//lgfx::Panel_SSD1963     _panel_instance;
//lgfx::Panel_ST7735      _panel_instance;
//lgfx::Panel_ST7735S     _panel_instance;
lgfx::Panel_ST7789      _panel_instance;
//lgfx::Panel_ST7796      _panel_instance;


// Prepare an instance that matches the type of bus the panel is connected to.
  lgfx::Bus_SPI       _bus_instance;   // Instance for SPI bus
//lgfx::Bus_I2C       _bus_instance;   // Instance for I2C bus (ESP32 only)
//lgfx::Bus_Parallel8 _bus_instance;   // Instance for 8-bit parallel bus (ESP32 only)

// Prepare an instance if backlight control is possible. (Delete if not needed)
  lgfx::Light_PWM     _light_instance;

// Prepare an instance that matches the type of touchscreen. (Delete if not needed)
//lgfx::Touch_CST816S          _touch_instance;
//lgfx::Touch_FT5x06           _touch_instance; // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436
//lgfx::Touch_GSL1680E_800x480 _touch_instance; // GSL_1680E, 1688E, 2681B, 2682B
//lgfx::Touch_GSL1680F_800x480 _touch_instance;
//lgfx::Touch_GSL1680F_480x272 _touch_instance;
//lgfx::Touch_GSLx680_320x320  _touch_instance;
//lgfx::Touch_GT911            _touch_instance;
//lgfx::Touch_STMPE610         _touch_instance;
//lgfx::Touch_TT21xxx          _touch_instance; // TT21100
//lgfx::Touch_XPT2046          _touch_instance;

public:

  // Create a constructor and perform various settings here.
  // If you change the class name, specify the same name for the constructor.
  LGFX(void)
  {
    { // Configure bus control settings.
      auto cfg = _bus_instance.config();    // Get the structure for bus configuration.

// SPI bus configuration
      cfg.spi_host = SPI2_HOST;     // Select the SPI to use  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // * With the ESP-IDF version update, VSPI_HOST , HSPI_HOST descriptions are deprecated, so if an error occurs, use SPI2_HOST , SPI3_HOST instead.
      cfg.spi_mode = 0;             // Set SPI communication mode (0 ~ 3)
      cfg.freq_write = 40000000;    // SPI clock for transmission (Max 80MHz, rounded to 80MHz divided by an integer)
      cfg.freq_read  = 16000000;    // SPI clock for reception
      cfg.spi_3wire  = true;        // Set to true if receiving on the MOSI pin
      cfg.use_lock   = true;        // Set to true if using transaction lock
      cfg.dma_channel = SPI_DMA_CH_AUTO; // Set the DMA channel to use (0=No DMA / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=Auto setting)
      // * With the ESP-IDF version update, SPI_DMA_CH_AUTO (auto setting) is recommended for DMA channel. 1ch, 2ch specification is deprecated.
      cfg.pin_sclk = 12;            // Set SPI SCLK pin number
      cfg.pin_mosi = 11;            // Set SPI MOSI pin number
      cfg.pin_miso = 17;            // Set SPI MISO pin number (-1 = disable)
      cfg.pin_dc   = 14;            // Set SPI D/C pin number (-1 = disable)
     // If sharing the SPI bus with an SD card, be sure to set MISO without omitting it.
//*/
/*
// I2C bus configuration
      cfg.i2c_port    = 0;          // Select the I2C port to use (0 or 1)
      cfg.freq_write  = 400000;     // Clock for transmission
      cfg.freq_read   = 400000;     // Clock for reception
      cfg.pin_sda     = 21;         // Pin number where SDA is connected
      cfg.pin_scl     = 22;         // Pin number where SCL is connected
      cfg.i2c_addr    = 0x3C;       // I2C device address
//*/
/*
// 8-bit parallel bus configuration
      cfg.i2s_port = I2S_NUM_0;     // Select the I2S port to use (I2S_NUM_0 or I2S_NUM_1) (Uses ESP32's I2S LCD mode)
      cfg.freq_write = 20000000;    // Transmission clock (Max 20MHz, rounded to 80MHz divided by an integer)
      cfg.pin_wr =  4;              // Pin number where WR is connected
      cfg.pin_rd =  2;              // Pin number where RD is connected
      cfg.pin_rs = 15;              // Pin number where RS(D/C) is connected
      cfg.pin_d0 = 12;              // Pin number where D0 is connected
      cfg.pin_d1 = 13;              // Pin number where D1 is connected
      cfg.pin_d2 = 26;              // Pin number where D2 is connected
      cfg.pin_d3 = 25;              // Pin number where D3 is connected
      cfg.pin_d4 = 17;              // Pin number where D4 is connected
      cfg.pin_d5 = 16;              // Pin number where D5 is connected
      cfg.pin_d6 = 27;              // Pin number where D6 is connected
      cfg.pin_d7 = 14;              // Pin number where D7 is connected
//*/

      _bus_instance.config(cfg);    // Apply configuration settings to the bus.
      _panel_instance.setBus(&_bus_instance);      // Set the bus to the panel.
    }

    { // Configure display panel control settings.
      auto cfg = _panel_instance.config();    // Get the structure for display panel configuration.

      cfg.pin_cs           =    13;  // Pin number where CS is connected   (-1 = disable)
      cfg.pin_rst          =    15;  // Pin number where RST is connected  (-1 = disable)
      cfg.pin_busy         =    -1;  // Pin number where BUSY is connected (-1 = disable)

      // * The following values are set to general default values for each panel, so try commenting out unknown items.

      cfg.panel_width      =   240;  // Actual visible width
      cfg.panel_height     =   320;  // Actual visible height
      cfg.offset_x         =     0;  // Panel offset amount in X direction
      cfg.offset_y         =     0;  // Panel offset amount in Y direction
      cfg.offset_rotation  =     2;  // Offset value for rotation direction 0~7 (4~7 are vertically inverted)
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel read
      cfg.dummy_read_bits  =     1;  // Number of dummy read bits before non-pixel data read
      cfg.readable         =  true;  // Set to true if data read is possible
      cfg.invert           =  true;  // Set to true if panel brightness is inverted [CHANGED for your screen]
      cfg.rgb_order        =  false;  // Set to true if panel red and blue are swapped [CHANGED for your screen]
      cfg.dlen_16bit       = false;  // Set to true for panels that send data length in 16-bit units via 16-bit parallel or SPI
      cfg.bus_shared       =  true;  // Set to true if sharing bus with SD card (bus control is performed by drawJpgFile etc.)

// Set the following only if the display is shifted with drivers like ST7735 or ILI9163 where pixel count is variable.
//    cfg.memory_width     =   240;  // Maximum width supported by the driver IC
//    cfg.memory_height    =   320;  // Maximum height supported by the driver IC

      _panel_instance.config(cfg);
    }

//*

    setPanel(&_panel_instance); // Set the panel to use.
  }
};