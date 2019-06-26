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
 * - Potentiometer 10k/N
 * - Buzzer
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
 * @version 0.5 2019-06-26
 *      Updated QMenu library.
 *      Fixed menu rendering.
 *      Fixed display rendering.
 *      Fixed menu controlling.
 *      Added menu icons drawing.
 *      Fixed menu constant names.
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

/* Drawing values */
#define GL_BASE_PADDING 1
#define GL_MENU_PADDING 1

/* Display render period in ms */
#define OLED_REFRESH_PERIOD 200

/* Application settings */
struct Settings {
    word minFreq;
    word maxFreg;
    byte pulseWidth;
    byte accelerationCurve;
    byte freqFloating;
    byte freqUnits;
    bool useSounds;
} settings = {
    10, // 10Hz ~ 600rpm
    200, // 200Hz ~ 12000 rpm
    5, // 5 ms
    ACCELERATION_SHAPE_LINEAR, // default acceleration type
    0, // default frequency floating 0%
    FREQ_UNITS_RPM,
    false
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
    delay(1000);
}

/* Render splash screen */
void renderSplash() {
    oled.setFont(u8g_font_6x13);
    oled.setFontRefHeightText();
    oled.setFontPosTop();
    oled.setDefaultForegroundColor();

    char* logo = "Pulse generator";
    u8g_uint_t textWidth = oled.getStrWidth(logo);
    u8g_uint_t fontHeight = oled.getFontAscent() - oled.getFontDescent();
    u8g_uint_t left = (oled.getWidth() - textWidth) / 2;
    u8g_uint_t top = (oled.getHeight() - fontHeight) / 2;
    oled.firstPage();
    do {
        oled.drawStr(left, top, logo);
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
    oled.firstPage();
    do {
        // Current frequency
        char freq[16];
        sprintf(freq, "%d", frequency);

        oled.setDefaultForegroundColor();
        oled.setFont(u8g_font_fur30n);
        oled.setFontRefHeightText();
        oled.setFontPosTop();
        oled.drawStr(0, 0, freq);

        // Bottom line info
        oled.setFont(u8g_font_6x13);
        oled.setFontPosBottom();
        oled.drawStr(0, 60, settings.freqUnits == FREQ_UNITS_RPM ? "rpm" : "Hz");
    } while (oled.nextPage());
}

/* Render setup item value measuring */
void renderMeasure() {
    // TODO Use static strings
    char* value = NULL;
    char* units = NULL;

    switch (selected->getId()) {
        case MENU_PULSE_WIDTH:
            value = settings.pulseWidth;
            break;
    }

    // Draw settings item value measure
    oled.setDefaultForegroundColor();

    oled.firstPage();
    do {
        // Measured item caption
        oled.setFont(u8g_font_6x13);
        oled.setFontPosTop();
        oled.drawStr(GL_BASE_PADDING, GL_BASE_PADDING, selected->getCaption());

        // Measured item units
        if (units != NULL) {
            oled.setFont(u8g_font_6x13);
            oled.setFontPosBottom();
            oled.drawStr(GL_BASE_PADDING, oled.getHeight() - GL_BASE_PADDING, units);
        }

        // Measured value
        if (value != NULL) {
            oled.setFont(u8g_font_fur30n);
            oled.setFontPosTop();
            oled.drawStr(GL_BASE_PADDING, oled.getHeight() - GL_BASE_PADDING, value);
        }

    } while (oled.nextPage());
}


/* Renders menu menu in current state on oled */
void renderMenu() {
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

            case MENU_PULSE_WIDTH:
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
            menu.prev();
        } else if (event.direction == right) {
            menu.next();
        }
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
        menu.enter();
    }
}

/* Encoder long click event */
void encoderOnLongClick() {
    // TODO Long click held one moment longer causes short click in menu

    if (measureSettingsValue) {
        // Discard measured value and escape measuring
        measureSettingsValue = false;
        renderMenu();
    } else if (selected->getId() != MENU_GENERATOR) {
        // Get up in the menu
        menu.back();
    }
}

/* Menu item changed */
void activeItemChanged(QMenuActiveItemChangedEvent event) {
    // If selected item really changed, redraw
    if (selected != event.newActiveItem) {
        selected = event.newActiveItem;
        if (event.newActiveItem->getId() != MENU_GENERATOR) {
            renderMenu();
        }
    }
}

/* Menu item used */
void onItemUtilized(QMenuItemUtilizedEvent event) {
    if (event.utilizedItem->getId() == MENU_BACK) {
        menu.back();
    } else if (event.utilizedItem->isCheckable()) {
        menu.toggleCheckable(event.utilizedItem);
        if (event.utilizedItem->getId() == MENU_SOUND) {
            settings.useSounds = event.utilizedItem->isChecked(); 
        }
        renderMenu();
    } else if (event.utilizedItem->isRadio()) {
        menu.switchRadio(event.utilizedItem);
        switch (event.utilizedItem->getId()) {
            case MENU_CURVE_SHAPE_LINEAR:
                settings.accelerationCurve = ACCELERATION_SHAPE_LINEAR;
                break;
            case MENU_CURVE_SHAPE_QUADRATIC:
                settings.accelerationCurve = ACCELERATION_SHAPE_LINEAR;
                break;
            case MENU_FREQ_UNITS_RPM:
                settings.freqUnits = FREQ_UNITS_RPM;
                break;
            case MENU_FREQ_UNITS_HZ:
                settings.freqUnits = FREQ_UNITS_HZ;
                break;
        }
        renderMenu();
    } else {

        // Switch action via currently selected menu item
        switch (event.utilizedItem->getId()) {

            // Setup item value measuring
            case MENU_MIN_FREQ:
            case MENU_MAX_FREQ:
            case MENU_PULSE_WIDTH:
            case MENU_FREQ_FLOATING:
                measureSettingsValue = true;
                renderMeasure();
                break;
        }
    }
}

/* Render menu item */
void onRenderMenuItem(QMenuRenderItemEvent event) {
    // Item icon
    char icon[2] = "";
    if (event.item->getMenu() != NULL) {
        strcpy(icon, "\x36");
    } else if (event.item->isRadio()) {
        strcpy(icon, event.item->isChecked() ? "\x49" : "\x4b");
    } else if (event.item->isCheckable()) {
        strcpy(icon, event.item->isChecked() ? "\x23" : "\x21");   
    }

    // Setup font for menu caption
    oled.setFont(u8g_font_6x13);
    oled.setFontRefHeightText();
    oled.setFontPosTop();
    oled.setDefaultForegroundColor();
    u8g_uint_t lineHeight = oled.getFontAscent() - oled.getFontDescent() + GL_MENU_PADDING;

    // If drawing selected item, draw bar and set bg color
    if (event.isActive) {
        oled.drawBox(0, lineHeight * event.renderIndex, oled.getWidth(), lineHeight + GL_MENU_PADDING);
        oled.setDefaultBackgroundColor();
    }
    oled.drawStr(GL_MENU_PADDING, lineHeight * event.renderIndex + GL_MENU_PADDING, event.item->getCaption());

    // Draw item's icon
    if (strlen(icon) > 0) {
        oled.setFont(u8g_font_8x13_75r);
        oled.setFontPosTop();
        u8g_uint_t iconWidth = oled.getStrWidth(icon);
        if (event.isActive) {
            oled.setDefaultBackgroundColor();
        } else {
            oled.setDefaultForegroundColor();
        }
        oled.drawStr(oled.getWidth() - iconWidth - GL_MENU_PADDING, lineHeight * event.renderIndex, icon);
    }
}

/* Loads settings from EEPROM */
void loadSettings() {
    // TODO set settings values
    // TODO update menu checkables and radios
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
