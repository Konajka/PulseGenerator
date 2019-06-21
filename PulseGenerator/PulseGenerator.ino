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
 *      Base implementation.
 * @version 0.2 2019-05-13
 *      Added rotary encoder support.
 *      Added menu structure.   
 * @version 0.3 2019-06-13     
 *      Fixed menu rendering.
 * @version 0.4 2019-06-18     
 *      Set state model.
 *      Fixed font sizes.
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
QMenu menu(MENU_GENERATOR, "Generator");
QMenuListRenderer menuRenderer(&menu, MENU_SIZE);

/* Display render period in ms */
#define OLED_REFRESH_PERIOD 200

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

/* Last renreding millis */
long oledLastRefresh;

/* Current menu item selected */
QMenuItem* selected = NULL;

/* Current working frequency */
word frequency = settings.minFreq;

/* Settings value measuring flag */
bool measureSettingsValue = false;

/* Initialization */
void setup() {
    #ifdef SERIAL_LOG
    Serial.begin(9600);
    #endif

    //Set menu events and create structure
    menu.setOnActiveItemChanged(activeItemChanged);
    menu.setOnItemUtilized(onItemUtilized);
    populateMenu(menu);
    selected = menu.getActive();

    // Setup menu rederer
    menuRenderer.setOnRenderItem(onRenderMenuItem);  

    // Setup encoder
    encoder.setOnChange(encoderOnChange);
    encoder.setOnClick(encoderOnClick);
    encoder.setOnLongClick(encoderOnLongClick);
    encoder.begin();
    
    // Load settings
    loadSettings();

    // Read frequency from A/D
    frequency = readFrequnecyValue();    

    // Render menu 
    oledLastRefresh = millis();
    renderSplash();
    delay(10000);
}

/* Render splash screen */
void renderSplash() {
    oled.setFont(u8g_font_6x13);
    oled.setFontRefHeightText();
    oled.setFontPosTop();
    oled.setDefaultForegroundColor();

    char* text = "Pulse generator";
    u8g_uint_t textWidth = oled.getStrWidth(text);
    u8g_uint_t fontHeight = oled.getFontAscent() - oled.getFontDescent();
    u8g_uint_t left = (oled.getWidth() - textWidth) / 2;
    u8g_uint_t top = (oled.getHeight() - fontHeight) / 2;
    oled.firstPage();    
    do {
        oled.drawStr(left, top, text);
        
        oled.setFont(u8g_font_6x13_75r);
        oled.drawStr(0, 40, "\x50 \x51");
    } while (oled.nextPage());    
}

/* Main Loop */
void loop() {  
    encoder.update();

    // Render generator screen if actaual
    if (selected->getId() == MENU_GENERATOR) {
        if (oledLastRefresh + OLED_REFRESH_PERIOD < millis()) {
            renderGenerator();
            oledLastRefresh = millis();
        }
    }
}

/* Render main screen */
void renderGenerator() {
    #ifdef SERIAL_LOG
    Serial.println("renderGenerator()");
    #endif
    
    oled.firstPage();
    do {
        // Current frequency
        String text = String(frequency);
        oled.setDefaultForegroundColor();
        oled.setFont(u8g_font_fur30n);
        oled.setFontRefHeightText();
        oled.setFontPosTop();
        oled.drawStr(0, 0, text.c_str()); 
        
        // Bottom line info
        oled.setFont(u8g_font_6x13);
        oled.setFontPosBottom();
        text = settings.freqUnits == FREQ_UNITS_RPM ? "rpm" : "Hz";
        oled.drawStr(0, 60, text.c_str());
    } while (oled.nextPage());
}

/* Render setup item value measuring */
void renderMeasure() {
    #ifdef SERIAL_LOG
    Serial.println("renderMeasure()");
    #endif
    
    // TODO Draw settings item value measure
    oled.firstPage();
    oled.setDefaultForegroundColor();
    oled.setFont(u8g_font_6x13);
    oled.setFontPosTop();    
    do {
        oled.drawStr(0, 30, "Measure item");
    } while (oled.nextPage()); 
}


/* Renders menu menu in current state on oled */
void renderMenu() {
    #ifdef SERIAL_LOG
    Serial.println("renderMenu()");
    #endif
    
    oled.firstPage();
    do {
        menuRenderer.render();
    } while (oled.nextPage());
}

/* Encoder rotation event */
void encoderOnChange(RotaryEncoderOnChangeEvent event) { 
    if (measureSettingsValue) {
        // Measure setup value by selected item 
        switch (selected->getId()) {
            case MENU_MIN_FREQ:
                // TODO change value and request render
                break;
              
            case MENU_MAX_FREQ:
                // TODO change value and request render
                break;
                
            case MENU_PULSE_RATIO:
                // TODO change value and request render
                break;
            case MENU_FREQ_FLOATING:
                // TODO change value and request render
                break;
        }
        renderMeasure();
    } else if (selected->getId() != MENU_GENERATOR) {
        // Move in menu
        if (event.direction == left) {
            selected = menu.prev();
        } else if (event.direction == right) {
            selected = menu.next();
        }
        renderMenu();    
    }
}

/* Encoder click event */
void encoderOnClick() {
    if (measureSettingsValue) { 
        // Update measured value and escape measuring
        // TODO Save measure item
        measureSettingsValue = false;
        renderMenu();
    } else { 
        // Menu click
        selected = menu.enter(); 
        // No need to request render, menu controller does it
    }
}

/* Encoder long click event */
void encoderOnLongClick() {
    if (measureSettingsValue) {
        // Discard measured value and escape measuring
        measureSettingsValue = false;
        renderMenu();
    } else if (selected->getId() != MENU_GENERATOR) {
        // Get up in the menu
        selected = menu.back(); 
        if (selected->getId() == MENU_GENERATOR) {
            renderGenerator();
        }
    }
}

/* Menu item changed */
void activeItemChanged(QMenuActiveItemChangedEvent event) {
    renderMenu();
}

/* Menu item used */
void onItemUtilized(QMenuItemUtilizedEvent event) {
    // Switch action via currently selected menu item
    switch (event.utilizedItem->getId()) {

        // Setup item value measuring
        case MENU_MIN_FREQ:
        case MENU_MAX_FREQ:
        case MENU_PULSE_RATIO:
        case MENU_FREQ_FLOATING:
            measureSettingsValue = true;
            renderMeasure();
            break;

        // Common menu item click (radiogroup or checkbox item)
        default:
            renderMenu();
    }
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

/* Loads settings from EEPROM */
void loadSettings() {
    // TODO
}

/* Stores settings into EEPROM */
void saveSettings() {
    // TODO
}

/* Calucates frequency from min and max value and A/D current value */
word readFrequnecyValue() {
    // TODO read A/D value
    // TODO apply curve shape
    // TODO map value between min and max
}
