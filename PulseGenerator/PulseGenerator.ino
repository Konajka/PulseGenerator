/*
 * Pulse Generator
 * 
 * Simple flip-flop pulse generator with adjustable minimal, maximal and 
 * current working frequency. Adjusting is perfomed by rotary encoder,
 * shown on display.
 * 
 * The circuit:
 * - Arduino Nano
 * - Rotary encoder KY-040
 * - I2C OLED display 0,96" 128x64 (SSD-1306)
 * 
 * @author https://github.com/Konajka
 * @version 0.1 2019-05-02
 *      Base implementation
 * @version 0.2 2019-05-13
 *      Added rotary encoder support.
 *      Added menu structure.   
 * @version 0.3 2019-06-13     
 *      Fixed menu rendering.
 */

#include <Arduino.h>
#include "U8glib.h"
#include "lib/RotaryEncoder.h"
#include "lib/QMenu.h"
#include "lib/Env.h"

/* Enable serial link */
//#define SERIAL_LOG

/* Rotary encoder controller */
#define ENCODER_CLK 5
#define ENCODER_DT 4
#define ENCODER_SW 3
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);

/* OLED Display 128x64 */
U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);

/* Menu controller and renederer */
#define MENU_SIZE 5
QMenu menu(MENU_SETUP, "Setup");
QMenuListRenderer menuRenderer(&menu, MENU_SIZE);

/* Application status holder */
enum Status { running, navigation } status = running;

/* Current working frequency */
word frequency;

/* Application settings */
struct Settings {
    word minFreq;
    word maxFreg;
    byte freqRatio;
    byte accelerationCurve;
    byte freqFloating;
    byte freqUnits;
} settings = {
    10, // 10Hz ~ 600rpm
    200, // 200Hz ~ 12000 rpm
    5, // 5% pulse width 
    ACCELERATION_SHAPE_LINEAR, // default acceleration type
    0, // default frequency floating 0%
    FREQ_UNITS_RPM 
};

/* Initialization */
void setup() {
    #ifdef SERIAL_LOG
    Serial.begin(9600);
    #endif

    //Set menu events and create structure
    menu.setOnActiveItemChanged(activeItemChanged);
    menu.setOnItemUtilized(onItemUtilized);
    populateMenu(menu);

    // Setup menu rederer
    menuRenderer.setOnRenderItem(onRenderMenuItem);  

    // Setup encoder
    encoder.setOnChange(encoderOnChange);
    encoder.setOnClick(encoderOnClick);
    encoder.setOnLongClick(encoderOnLongClick);
    encoder.begin();
    
    // Load settings
    loadSettings();

    //Set initial frequency
    frequency = settings.minFreq;    

    // Render menu 
    renderMenu();
}

/* Main Loop */
void loop() {  
    encoder.update();
    renderScreen();
}

/** Render screen via status model */
void renderScreen() {
    // if refresh timeout ...
    // render data if not redered by menu controller...
}

/* Encoder rotation event */
void encoderOnChange(RotaryEncoderOnChangeEvent event) {  
    if (event.direction == left) {
        QMenuItem* item = menu.prev();
    } else if (event.direction == right) {
        QMenuItem* item = menu.next();
    }
}

/* Encoder click event */
void encoderOnClick() {
    QMenuItem* item = menu.enter();
}

/* Encoder long click event */
void encoderOnLongClick() {
    QMenuItem* item = menu.back();
}

/* Menu item changed */
void activeItemChanged(QMenuActiveItemChangedEvent event) {
    renderMenu();
}

/* Menu item used */
void onItemUtilized(QMenuItemUtilizedEvent event) {
    renderMenu();
}

/* Render menu item */
void onRenderMenuItem(QMenuRenderItemEvent event) {
    oled.setFont(u8g_font_6x13);
    oled.setFontRefHeightText();
    oled.setFontPosTop();

    u8g_uint_t padding = 1;
    u8g_uint_t width = oled.getWidth();
    u8g_uint_t height = oled.getFontAscent() - oled.getFontDescent() + padding;
    
    oled.setDefaultForegroundColor();
    if (event.isActive) {
        oled.drawBox(0, height * event.renderIndex, width, height + padding);
        oled.setDefaultBackgroundColor();
    }
    oled.drawStr(padding, height * event.renderIndex + padding, event.item->getCaption());
}

/* Renders menu menu in current state on oled */
void renderMenu() {
    oled.firstPage();
    oled.setFont(u8g_font_unifont);
    do {
        menuRenderer.render();
    } while (oled.nextPage());
}

/* Loads settings from EEPROM */
void loadSettings() {
    // TODO
}

/* Stores settings into EEPROM */
void saveSettings() {
    // TODO
}
