/**
 * @file GaitPhaseDetector.h
 *
 * @brief Detects the swing/stance phase of the exo leg from the FSRs on BOTH feet.
 * 
 * This replaces the time-extrapolated phase estimation of the original ExoCode
 * (which predicts percent gait from an expected step duration) with a purely
 * event-driven state machine.
 * 
 * Key insight: during double support, one foot's toe enters its falling edge at
 * (almost) the same time the other foot's heel enters its rising edge. So the
 * supporting (contralateral) foot tells us exactly when the exo leg is about to
 * start, and stop, swinging. There is no need to predict the phase from time.
 * 
 * @author Your Name
 * @date 2026
 */

#ifndef GaitPhaseDetector_h
#define GaitPhaseDetector_h

#include "Arduino.h"
#include "ExoData.h"
#include "Config.h"

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

/**
 * @brief Event-driven gait phase detector for the exo leg.
 * 
 * Watches the four FSRs (exo heel/toe + contralateral heel/toe) and emits
 * one-shot swing_started/swing_ended events plus a filtered in_swing state and
 * percent progress through the swing. The swing boundaries are anchored to real
 * FSR edges, so sudden acceleration/deceleration does not make the phase wrong.
 * 
 */
class GaitPhaseDetector
{
    public:
        /**
         * @brief Construct a new Gait Phase Detector object
         * 
         * @param exo_side pointer to the side data of the leg wearing the exo (has the hip motor)
         * @param exo_data pointer to the full data instance, used to reach the contralateral side
         */
        GaitPhaseDetector(SideData* exo_side, ExoData* exo_data);

        /**
         * @brief Call once per control loop. Reads both feet's FSR contact states
         * (already updated by Side::read_data) and updates the exo side's
         * swing_started, swing_ended, in_swing, percent_swing, percent_stance,
         * and measured durations.
         */
        void update();

    private:
        /**
         * @brief A debounced edge detector for a single boolean signal.
         * An edge only fires after the new state has been held for the debounce
         * time, which rejects false edges from shuffling/dragging.
         */
        class DebouncedEdge
        {
            public:
                DebouncedEdge(unsigned int debounce_ms) : _debounce_ms(debounce_ms) {};
                
                /**
                 * @brief Feed the raw state in, returns the debounced state.
                 * 
                 * @param raw current raw state of the signal
                 * @return bool debounced state
                 */
                bool update(bool raw);
                
                /**
                 * @brief True for one call after the debounced signal rises.
                 */
                bool rising_edge();
                
                /**
                 * @brief True for one call after the debounced signal falls.
                 */
                bool falling_edge();
                
            private:
                unsigned int _debounce_ms;      /**< Hold time required before a state change is accepted. */
                unsigned long _change_start_ms; /**< Time the raw signal first differed from the debounced state. */
                bool _state = false;            /**< Current debounced state. */
                bool _rising = false;           /**< One-shot rising edge flag. */
                bool _falling = false;          /**< One-shot falling edge flag. */
                bool _initialized = false;      /**< False until the first sample has been seen. Suppresses a spurious edge at boot. */
        };

        //Exo leg FSR edges
        DebouncedEdge _exo_heel;    /**< Exo leg heel FSR contact */
        DebouncedEdge _exo_toe;     /**< Exo leg toe FSR contact */

        //Contralateral (supporting) foot FSR edges
        DebouncedEdge _contra_heel; /**< Contralateral heel FSR contact */
        DebouncedEdge _contra_toe;  /**< Contralateral toe FSR contact */

        //Phase state
        bool _in_swing = false;             /**< High while the exo leg is swinging. */
        unsigned long _swing_start_ms;      /**< Time the current swing started. */
        unsigned long _last_swing_start_ms; /**< Time the previous swing started. */
        unsigned long _last_swing_end_ms;   /**< Time the previous swing ended. */
        float _measured_swing_ms = 0;       /**< Duration of the last completed swing. */
        float _measured_stance_ms = 0;      /**< Duration of the last completed stance. */

        SideData* _exo_side;    /**< Side data of the exo leg. */
        SideData* _contra_side; /**< Side data of the contralateral (supporting) leg. */

        /**
         * @brief Emits the swing_started one-shot event and updates phase state.
         * Triggered by the contralateral heel rising edge (supporting foot just
         * landed = exo leg about to push off and swing).
         */
        void _on_swing_start();

        /**
         * @brief Emits the swing_ended one-shot event and updates phase state.
         * Triggered by the contralateral toe falling edge (supporting foot just
         * left the ground = exo leg about to heel strike).
         */
        void _on_swing_end();
};

#endif
#endif
