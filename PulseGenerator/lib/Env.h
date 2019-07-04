#ifndef ENV_H
#define ENV_H

/* Menu constants */
#define MENU_GENERATOR 1
#define MENU_MIN_FREQ 11
#define MENU_MAX_FREQ 12
#define MENU_PULSE_WIDTH 13
#define MENU_CURVE_SHAPE_SUBMENU 14
#define MENU_CURVE_SHAPE_LINEAR 141
#define MENU_CURVE_SHAPE_QUADRATIC 142
#define MENU_FREQ_FLOATING 15
#define MENU_FREQ_UNITS_SUBMENU 16
#define MENU_FREQ_UNITS_RPM 161
#define MENU_FREQ_UNITS_HZ 162
#define MENU_USE_FILTER 17
#define MENU_BACK 0

#define ACCELERATION_SHAPE_LINEAR 0
#define ACCELERATION_SHAPE_QUADRATIC 1

#define FREQ_UNITS_RPM 0
#define FREQ_UNITS_HZ 1

/* Create menu structure */
void populateMenu(QMenu& menu) {
    menu.getRoot()
        ->setMenu(QMenuItem::create(MENU_MIN_FREQ, "Minimal frequency"))
        ->setNext(QMenuItem::create(MENU_MAX_FREQ, "Maximal frequency"))
        ->setNext(QMenuItem::create(MENU_PULSE_WIDTH, "Pulse width"))
        ->setNext(QMenuItem::create(MENU_CURVE_SHAPE_SUBMENU, "Acceleration curve"))
            ->setMenu(QMenuItem::createRadio(MENU_CURVE_SHAPE_LINEAR, "Linear curve", MENU_CURVE_SHAPE_SUBMENU, true))
            ->setNext(QMenuItem::createRadio(MENU_CURVE_SHAPE_QUADRATIC, "Quadratic curve", MENU_CURVE_SHAPE_SUBMENU, false))
            ->setNext(QMenuItem::create(MENU_BACK, "Back"))
            ->getBack()
        //->setNext(QMenuItem::create(MENU_FREQ_FLOATING, "Frequency floating"))
        ->setNext(QMenuItem::create(MENU_FREQ_UNITS_SUBMENU, "Frequency units"))
            ->setMenu(QMenuItem::createRadio(MENU_FREQ_UNITS_RPM, "Rotates per minute", MENU_FREQ_UNITS_SUBMENU, true))
            ->setNext(QMenuItem::createRadio(MENU_FREQ_UNITS_HZ, "Hertz", MENU_FREQ_UNITS_SUBMENU, false))
            ->setNext(QMenuItem::create(MENU_BACK, "Back"))
            ->getBack()
        ->setNext(QMenuItem::createCheckable(MENU_USE_FILTER, "Use smooth filter", true))
        ->setNext(QMenuItem::create(MENU_BACK, "Back"));
}

/* Application settings */
#define SETTINGS_HEADER_SIZE 5
#define SETTINGS_HEADER_VERSION "SV01"
#define SETTINGS_EEPROM_ADDRESS 0
#define SETTINGS_MIN_FREQ_MIN 8
#define SETTINGS_MIN_FREQ_MAX 40
#define SETTINGS_MIN_FREQ_STEP 1
#define SETTINGS_MAX_FREQ_MIN 50
#define SETTINGS_MAX_FREQ_MAX 140
#define SETTINGS_MAX_FREQ_STEP 5
#define SETTINGS_PULSE_WIDTH_MIN 1
#define SETTINGS_PULSE_WIDTH_MAX 5
#define SETTINGS_PULSE_WIDTH_STEP 1

typedef struct Settings {
    char header[5];
    word minFreq;
    word maxFreq;
    byte pulseWidth;
    byte accelerationCurve;
    byte freqFloating;
    byte freqUnits;
    bool useFilter;
} ;

/* Returns current frequency level in requested units */
word getFreqByUnits(Settings settings, word freq) {
    return settings.freqUnits == FREQ_UNITS_RPM ? freq * 60 : freq;
}

/* Propagates settings structure to menu state */
void propagateSettingsToMenu(Settings settings, QMenu &menu) {
    menu.find(MENU_USE_FILTER, true)->setChecked(settings.useFilter);
    menu.switchRadio(menu.find(settings.accelerationCurve == ACCELERATION_SHAPE_LINEAR
            ? MENU_CURVE_SHAPE_LINEAR : MENU_CURVE_SHAPE_QUADRATIC, true));
    menu.switchRadio(menu.find(settings.freqUnits == FREQ_UNITS_RPM
            ? MENU_FREQ_UNITS_RPM : MENU_FREQ_UNITS_HZ, true));
}

/* Gets current units name */
void getFreqUnits(Settings settings, char* buffer) {
    switch (settings.freqUnits) {
        case FREQ_UNITS_RPM:
            strcpy(buffer, "rpm");
            break;
        case FREQ_UNITS_HZ:
            strcpy(buffer, "Hz");
    }
}

#endif
