/**
 * @brief Library for easy use of rotary encoder with rotation velocity support.
 *
 * @author https://github.com/Konajka
 * @version 0.1 2019-05-13
 *      Base implementation
 * @version 0.2 2019-05-13
 *      Base rotation support.
 *      Base switch support.
 * @version 0.3 2019-05-13
 *      Added click and long click support.
 * @version 0.4 2019-05-14
 *      Added velocity support.
 * @version 0.5 2019-06-21
 *      Removed ROTARY_ENCODER_CLICKS_SUPPORT direcitve.
 */

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <Arduino.h>

// Long click minimal time
#define ROTARY_ENCODER_LONG_CLICK_MILLIS 450

// Direction definition
enum RotaryEncoderDirection { left, right };
struct RotaryEncoderOnChangeEvent {
    const RotaryEncoderDirection direction;
    const word velocity;
};
// Direction change event
typedef void (*RotaryEncoderOnChange) (RotaryEncoderOnChangeEvent);

// Switch state definition
enum RotaryEncoderSwitchAction { press, release };
struct RotaryEncoderOnSwitchEvent {
    const RotaryEncoderSwitchAction action;
};
// Switch state change event
typedef void (*RotaryEncoderOnSwitch) (RotaryEncoderSwitchAction);

// Click and long-click event
typedef void (*RotaryEncoderOnClick) ();
typedef void (*RotaryEncoderOnLongClick) ();

/**
 * Rotary encoder controller class.
 */
class RotaryEncoder 
{
    private:
        // Initialization flag
        bool initialized = false;
    
        // PIN definitions
        uint8_t _PIN_CLOCK;
        uint8_t _PIN_DATA;
        uint8_t _PIN_SWITCH;
        
        // Last values memory
        int _pinClockRetain;
        int _pinDataRetain;
        int _pinSwitchRetain;
        
        // Events
        RotaryEncoderOnChange _onChange = NULL;
        RotaryEncoderOnSwitch _onSwitch = NULL;
        RotaryEncoderOnClick _onClick = NULL;
        RotaryEncoderOnLongClick _onLongClick = NULL;
        
        // Measuring velocity
        unsigned long _velocityTime = 0;
                
        // Long click time measuring
        unsigned long _switchPressTime = 0;

    protected:
    
        /**
         * Calls onChange event if assigned.
         * 
         * @param direction Rotation direction info.
         * @param velocity Time spend from last event in millis.
         */ 
        void doOnChange(RotaryEncoderDirection direction, word velocity) {
            if (_onChange != NULL) {
                RotaryEncoderOnChangeEvent event = {
                    direction,
                    velocity
                };
                _onChange(event);
            }
        }

        /**
         * Calls onSwitch event if assigned.
         * 
         * @param action Encoder switch info.
         */ 
        void doOnSwitch(RotaryEncoderSwitchAction action) {
            if (_onSwitch != NULL) {
                RotaryEncoderSwitchAction event = {
                    action
                };
                _onSwitch(event);
            }
        }
        
        /**
         * Calls onClick event if assigned.
         */ 
        void doOnClick() {
            if (_onClick != NULL) {
                _onClick();
            }
        }    
        
        /**
         * Calls onLongClick event if assigned.
         */ 
        void doOnLongClick() {
            if (_onLongClick != NULL) {
                _onLongClick();
            }
        }              

    public:
    
        /**
         * Controller constructor, assigns encoder pins. 
         */
        RotaryEncoder(uint8_t PIN_CLOCK, uint8_t PIN_DATA, uint8_t PIN_SWITCH) {
            _PIN_CLOCK = PIN_CLOCK;
            _PIN_DATA = PIN_DATA;
            _PIN_SWITCH = PIN_SWITCH;
        }

        /**
         * Gets onChange callback.
         * 
         * @return Returns assigned callback.
         */
        RotaryEncoderOnChange getOnChange()  {
            return _onChange;
        }

        /**
         * Assigns onChange event callback.
         * 
         * @param onChange callback to assign.
         */
        void setOnChange(RotaryEncoderOnChange onChange)  {
            _onChange = onChange;
        }

        /**
         * Gets onSwitch callback.
         * 
         * @return Returns assigned callback.
         */
        RotaryEncoderOnSwitch getOnSwitch() {
            return _onSwitch;
        }

        /**
         * Assigns onSwitch event callback.
         * 
         * @param onSwitch callback to assign.
         */
        void setOnSwitch(RotaryEncoderOnSwitch onSwitch) {
            _onSwitch = onSwitch;
        }
        
        /**
         * Gets onClick callback.
         * 
         * @return Returns assigned callback.
         */
        RotaryEncoderOnClick getOnClick() {
            return _onClick;
        }

        /**
         * Assigns onClick event callback.
         * 
         * @param onClick callback to assign.
         */
        void setOnClick(RotaryEncoderOnClick onClick) {
            _onClick = onClick;
        } 
        
        /**
         * Gets onLongClick callback.
         * 
         * @return Returns assigned callback.
         */
        RotaryEncoderOnClick getOnLongClick() {
            return _onLongClick;
        }

        /**
         * Assigns onLongClick event callback.
         * 
         * @param onLongClick callback to assign.
         */
        void setOnLongClick(RotaryEncoderOnLongClick onLongClick) {
            _onLongClick = onLongClick;
        }    

        /**
         * Initializes controller. Call this once before encoder use.
         */
        void begin() {
            initialized = true;
        
            // Clock pin in pull-up mode
            pinMode(_PIN_CLOCK, INPUT);
            digitalWrite(_PIN_CLOCK, HIGH);
            
            // Data pin in pull-up mode
            pinMode(_PIN_DATA, INPUT);
            digitalWrite(_PIN_DATA, HIGH);
            
            // Switch pin in pull-up mode
            pinMode(_PIN_SWITCH, INPUT_PULLUP);

            // First read
            _pinClockRetain = digitalRead(_PIN_CLOCK);
            _pinDataRetain = digitalRead(_PIN_DATA);
            _pinSwitchRetain = digitalRead(_PIN_SWITCH);
            
            _velocityTime = millis();
        }

        /**
         * Updates encoder state and calls callbacks if any assigned. Call this
         * repeatly in `loop()` method.
         * 
         * Note: Call `begin()` before this call or nothing s happens.
         */
        void update() {
            if (!initialized) {
                return;
            }
        
            // Detect rotation
            int pinClk = digitalRead(_PIN_CLOCK);
            if (_pinClockRetain != pinClk) {
                _pinClockRetain = pinClk;
                if (pinClk == HIGH) {
                    unsigned long velocityMeasure = millis();
                    unsigned long velocity = velocityMeasure - _velocityTime;
                    int pinDt = digitalRead(_PIN_DATA);
                    if (pinClk == pinDt) {                        
                        doOnChange(right, velocity);
                    } else {
                        doOnChange(left, velocity);
                    }
                    _velocityTime = velocityMeasure;
                }
            }
            
            // Detect long click
            if (_switchPressTime > 0) {
                if ((millis() - _switchPressTime) > ROTARY_ENCODER_LONG_CLICK_MILLIS) {
                    _switchPressTime = 0;
                    doOnLongClick();
                }                
            }

            // Detect switch press or release
            int pinSw = digitalRead(_PIN_SWITCH);
            if (_pinSwitchRetain != pinSw) {
                _pinSwitchRetain = pinSw;
                if (pinSw == HIGH) {                    
                    doOnSwitch(release);                    
                    if (_switchPressTime > 0) {
                        _switchPressTime = 0;
                        doOnClick();
                    }                     
                } else {
                    _switchPressTime = millis();
                    doOnSwitch(press);
                }
            }
        }
};

#endif
