#ifndef ENV_H
#define ENV_H

/* Menu constants */
#define MENU_GENERATOR 1
#define MENU_MIN_FREQ 11
#define MENU_MAX_FREQ 12
#define MENU_PULSE_RATIO 13
#define MENU_CURVE_SHAPE_SUBMENU 14
#define MENU_CURVE_SHAPE_LINEAR 141
#define MENU_CURVE_SHAPE_QUADRATIC 142
#define MENU_FREQ_FLOATING 15
#define MENU_FREQ_UNITS_SUBMENU 16
#define MENU_FREQ_UNITS_RPM 161
#define MENU_FREQ_UNITS_HZ 162
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
        ->setNext(QMenuItem::create(MENU_PULSE_RATIO, "Pulse ratio"))
        ->setNext(QMenuItem::create(MENU_CURVE_SHAPE_SUBMENU, "Acceleration curve"))
            ->setMenu(QMenuItem::createRadio(MENU_CURVE_SHAPE_LINEAR, "Linear curve", MENU_CURVE_SHAPE_SUBMENU, true))
            ->setNext(QMenuItem::createRadio(MENU_CURVE_SHAPE_QUADRATIC, "Quadratic curve", MENU_CURVE_SHAPE_SUBMENU, false))
            ->setNext(QMenuItem::create(MENU_BACK, "Back"))
            ->getBack()
        ->setNext(QMenuItem::create(MENU_FREQ_FLOATING, "Frequency floating"))
        ->setNext(QMenuItem::create(MENU_FREQ_UNITS_SUBMENU, "Frequency units"))
            ->setMenu(QMenuItem::createRadio(MENU_FREQ_UNITS_RPM, "Rotates per minute", MENU_FREQ_UNITS_SUBMENU, true))
            ->setNext(QMenuItem::createRadio(MENU_FREQ_UNITS_HZ, "Hertz", MENU_FREQ_UNITS_SUBMENU, false))
            ->setNext(QMenuItem::create(MENU_BACK, "Back"))
            ->getBack()
        ->setNext(QMenuItem::create(MENU_BACK, "Back"));
}

#endif
