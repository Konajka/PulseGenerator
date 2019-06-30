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
 *      Settings loading and saving
 * @version 0.6 2019-06-27
 *      Added valueable setting items measuring
 *      Added frequency showing
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "U8glib.h"
#include "lib/RotaryEncoder.h"
#include "lib/QMenu.h"
#include "lib/Env.h"

/* Enable serial link */
#define SERIAL_LOG

/* Freauency controlling potentiometer */
#define FREQ_PIN A0
#define FREQ_INPUT_MIN 0
#define FREQ_INPUT_MAX 1023
#define FREQ_AD_REFRESH_PERIOD 50
long adLastRefresh;

//#define BUZZER_PRESENT
#ifdef BUZZER_PRESENT
#define BUZZER_PIN 7
#endif

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
#define SETTINGS_HEADER_SIZE 5
#define SETTINGS_HEADER_VERSION "SV01"
#define SETTINGS_EEPROM_ADDRESS 0
#define SETTINGS_MIN_FREQ_MIN 8
#define SETTINGS_MIN_FREQ_MAX 20
#define SETTINGS_MIN_FREQ_STEP 1
#define SETTINGS_MAX_FREQ_MIN 180
#define SETTINGS_MAX_FREQ_MAX 240
#define SETTINGS_MAX_FREQ_STEP 10
struct Settings {
    char header[5];
    word minFreq;
    word maxFreq;
    byte pulseWidth;
    byte accelerationCurve;
    byte freqFloating;
    byte freqUnits;
    bool useSounds;
} settings = {
    SETTINGS_HEADER_VERSION, // Settings header in EEPROM
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

    #ifdef BUZZER_PRESENT
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
    #endif

    // Render menu
    oledLastRefresh = millis();
    adLastRefresh = millis();
    renderSplash();
    delay(2000);
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
    u8g_uint_t left = u8gCenter(oled.getWidth(), textWidth);
    u8g_uint_t top = u8gCenter(oled.getHeight(), fontHeight);
    oled.firstPage();
    do {
        oled.drawStr(left, top, logo);
    } while (oled.nextPage());
}

/* Main Loop */
void loop() {
    encoder.update();

    // Generator update
    if (selected->getId() == MENU_GENERATOR) {

        // Read frequency from A/D if the time comes
        if (adLastRefresh + FREQ_AD_REFRESH_PERIOD < millis()) {
            frequency = readFrequnecyValue();
            adLastRefresh = millis();
        }

        // Render displat values in the time comes
        if (oledLastRefresh + OLED_REFRESH_PERIOD < millis()) {
            renderGenerator();
            oledLastRefresh = millis();
        }

        #ifdef BUZZER_PRESENT
        tone(BUZZER_PIN, 480);
        #endif
    }
}

/* Render main screen */
void renderGenerator() {
    // Current frequency
    char freq[16] = "";
    sprintf(freq, "%d", getFreqByUnits(frequency));

    // Current frequency units
    char units[16] = "";
    getFreqUnits(units);

    // Calculate frequency text dimensions
    oled.setFont(u8g_font_fur30n);
    oled.setFontRefHeightText();
    u8g_uint_t maxValueWidth = oled.getStrWidth("00000");
    u8g_uint_t valueWidth = oled.getStrWidth(freq);
    u8g_uint_t valueHeight = oled.getFontAscent() - oled.getFontDescent();
    u8g_uint_t valueRight = u8gCenter(oled.getWidth(), maxValueWidth) + maxValueWidth;
    u8g_uint_t valueLeft = valueRight - valueWidth;
    u8g_uint_t valueTop = u8gCenter(oled.getHeight(), valueHeight);

    // Calculate units dimensions
    oled.setFont(u8g_font_6x13);
    u8g_uint_t unitsLeft = valueRight - oled.getStrWidth(units);
    u8g_uint_t unitsTop = valueTop + valueHeight + GL_BASE_PADDING;

    // Render
    oled.firstPage();
    do {
        // Current frequency
        oled.setDefaultForegroundColor();
        oled.setFont(u8g_font_fur30n);
        oled.setFontRefHeightText();
        oled.setFontPosTop();
        oled.drawStr(valueLeft, valueTop, freq);

        // Bottom line info
        oled.setFont(u8g_font_6x13);
        oled.setFontPosTop();
        oled.drawStr(unitsLeft, unitsTop, units);
    } while (oled.nextPage());
}

/* Render setup item value measuring */
void renderMeasure() {
    char value[16] = "";
    char units[16] = "";

    // Get value for measured item
    switch (selected->getId()) {
        case MENU_MIN_FREQ:
            sprintf(value, "%d", getFreqByUnits(settings.minFreq));
            getFreqUnits(units);
            break;
        case MENU_MAX_FREQ:
            sprintf(value, "%d", getFreqByUnits(settings.maxFreq));
            getFreqUnits(units);
            break;
        case MENU_PULSE_WIDTH:
            sprintf(value, "%d", settings.pulseWidth);
            strcpy(units, "ms");
            break;
        case MENU_FREQ_FLOATING:
            if (settings.freqFloating == 0) {
                strcpy(value, "Off");
            } else {
                sprintf(value, "%d", settings.freqFloating);
                strcpy(units, "%");
            }
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
        if (strlen(units) > 0) {
            oled.setFont(u8g_font_6x13);
            oled.setFontPosBottom();
            oled.drawStr(GL_BASE_PADDING, oled.getHeight() - GL_BASE_PADDING, units);
        }

        // Measured value
        if (strlen(value) > 0) {
            oled.setFont(u8g_font_fur30n);
            oled.setFontRefHeightText();
            oled.setFontPosTop();
            u8g_uint_t valueHeight = oled.getFontAscent() - oled.getFontDescent();
            oled.drawStr(GL_BASE_PADDING, u8gCenter(oled.getHeight(), valueHeight), value);
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
        // Get direction: right = increase, left = decrease
        bool up = event.direction == right;

        // Measure setup value by selected item
        switch (selected->getId()) {
            case MENU_MIN_FREQ:
                settings.minFreq = step(up, settings.minFreq, SETTINGS_MIN_FREQ_STEP,
                        up ? SETTINGS_MIN_FREQ_MAX : SETTINGS_MIN_FREQ_MIN);
                break;

            case MENU_MAX_FREQ:
                settings.maxFreq = step(up, settings.maxFreq, SETTINGS_MAX_FREQ_STEP,
                        up ? SETTINGS_MAX_FREQ_MAX : SETTINGS_MAX_FREQ_MIN);
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

/* Change value by step in given direction */
word step(bool up, word value, word step, word limit) {
    if (up) {
        return value + step <= limit ? value + step : limit;
    } else {
        return value - step >= limit ? value - step : limit;
    }
}

/* Encoder click event */
void encoderOnClick() {
    if (measureSettingsValue) {
        // Update measured value and escape measuring
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

        //Save settings when leaving menu or draw menu
        if (event.newActiveItem->getId() == MENU_GENERATOR) {
            saveSettings();
        } else {
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
        if (event.utilizedItem->getId() == MENU_SOUNDS) {
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

/* Gets current units name */
void getFreqUnits(char* buffer) {
    switch (settings.freqUnits) {
        case FREQ_UNITS_RPM:
            strcpy(buffer, "rpm");
            break;
        case FREQ_UNITS_HZ:
            strcpy(buffer, "Hz");
    }
}

/* Center object to range */
u8g_uint_t u8gCenter(u8g_uint_t range, u8g_uint_t size) {
    return (range - size) / 2;
}

/* Loads settings from EEPROM if stored */
void loadSettings() {
    Serial.println("Loading settings");
    
    // Find settings header in EEPROM
    bool headerFound = true;
    for (unsigned int index = 0; index < SETTINGS_HEADER_SIZE; index++) {
        if (EEPROM.read(SETTINGS_EEPROM_ADDRESS + index) != SETTINGS_HEADER_VERSION[index]) {
            headerFound = false;
            break;
        }
    }

    // If header found, read whole settings structure
    if (headerFound) {
        for (unsigned int index = 0; index < sizeof(settings); index++) {
            *((char *)&settings + index) = EEPROM.read(SETTINGS_EEPROM_ADDRESS + index);
        }

        // TODO Update menu checkables and radios from loaded settings
        Serial.println("Settings loaded");
        printSettings();
    } else {
        Serial.println("Header not found");
        printSettings();
    }
}

/* Stores settings into EEPROM */
void saveSettings() {
    Serial.println("Saving settings");
    printSettings();
    
    for (unsigned int index = 0; index < sizeof(settings); index++) {
        EEPROM.update(SETTINGS_EEPROM_ADDRESS + index, *((char *)&settings + index));
    }
}

void printSettings() {
    p("header", settings.header);
    p("nimFreq", settings.minFreq);
    p("maxFreq", settings.maxFreq);
    p("pulseWidth", settings.pulseWidth);
    p("accelCurve", settings.accelerationCurve);
    p("freqFloating", settings.freqFloating);
    p("freqUnits", settings.freqUnits);
    p("useSounds", settings.useSounds ? "true" : "false");

}

void p(char* name, const char* value) {
    Serial.print(name);
    Serial.print(" = ");
    Serial.print(value);
    Serial.println();
}

void p(char* name, word value) {
    Serial.print(name);
    Serial.print(value);
    Serial.println();
}

/* Calucates frequency from min and max value and A/D current value */
word readFrequnecyValue() {
    int value = analogRead(FREQ_PIN);
    // TODO apply curve shape
    return map(value, FREQ_INPUT_MIN, FREQ_INPUT_MAX, settings.minFreq, settings.maxFreq);
}

word getFreqByUnits(word freq) {
    return settings.freqUnits == FREQ_UNITS_RPM ? freq * 60 : freq;
}
