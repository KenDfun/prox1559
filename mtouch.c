/*
    MICROCHIP SOFTWARE NOTICE AND DISCLAIMER:

    You may use this software, and any derivatives created by any person or
    entity by or on your behalf, exclusively with Microchip's products.
    Microchip and its subsidiaries ("Microchip"), and its licensors, retain all
    ownership and intellectual property rights in the accompanying software and
    in all derivatives hereto.

    This software and any accompanying information is for suggestion only. It
    does not modify Microchip's standard warranty for its products.  You agree
    that you are solely responsible for testing the software and determining
    its suitability.  Microchip has no obligation to modify, test, certify, or
    support the software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE APPLY TO THIS SOFTWARE, ITS INTERACTION WITH MICROCHIP'S
    PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT, WILL MICROCHIP BE LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT
    (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), STRICT LIABILITY,
    INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR EXPENSE OF
    ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWSOEVER CAUSED, EVEN IF
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
    FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW, MICROCHIP'S TOTAL
    LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
    THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
    THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF
    THESE TERMS.
*/

#pragma warning disable 1395 /* 'Notable code sequence' message */

#include <stdint.h>

#include "configuration.h"
#include "mtouch.h"

        extern MTOUCH_FLAGS_t      MTOUCH_flags;

/* Global variables */
        enum MTOUCH_STATE   MTOUCH_button_state             [MTOUCH_SENSORS];
        MTOUCH_READING_t    MTOUCH_reading                  [MTOUCH_SENSORS];
        uint8_t             MTOUCH_delta_scaling            [MTOUCH_SENSORS];
        uint8_t             MTOUCH_button_debounce          [MTOUCH_SENSORS];
        MTOUCH_DELTA_t      MTOUCH_delta                    [MTOUCH_SENSORS];

        MTOUCH_DELTA_t      MTOUCH_button_threshold         [MTOUCH_SENSORS] =
        {
             (MTOUCH_DELTA_t)MTOUCH_BUTTON_THRESHOLD0
            #if (MTOUCH_SENSORS > 1)
            ,(MTOUCH_DELTA_t)MTOUCH_BUTTON_THRESHOLD1
            #endif
            #if (MTOUCH_SENSORS > 2)
            ,(MTOUCH_DELTA_t)MTOUCH_BUTTON_THRESHOLD2
            #endif
            #if (MTOUCH_SENSORS > 3)
            ,(MTOUCH_DELTA_t)MTOUCH_BUTTON_THRESHOLD3
            #endif
            #if (MTOUCH_SENSORS > 4)
            ,(MTOUCH_DELTA_t)MTOUCH_BUTTON_THRESHOLD4
            #endif
            #if (MTOUCH_SENSORS > 5)
            ,(MTOUCH_DELTA_t)MTOUCH_BUTTON_THRESHOLD5
            #endif
            #if (MTOUCH_SENSORS > 6)
            #error("Edit required in MTOUCH_Button_StateMachine threshold array initialization to support additional sensors.")
            #endif
        };

#if defined(CONFIGURABLE_BASELINEGAIN)
        uint8_t             MTOUCH_baselineGain;
#endif
#if defined(CONFIGURABLE_BASELINERATE)
        uint8_t             MTOUCH_baselineRate;
#endif
#if defined(CONFIGURABLE_NEGDELTACOUNTS)
        uint8_t             MTOUCH_negDeltaCount;
#endif
#if defined(CONFIGURABLE_PRESSTIMEOUT)
        uint8_t             MTOUCH_pressTimeout;
#endif
#if defined(CONFIGURABLE_DEBOUNCE)
        uint8_t             MTOUCH_debounce;
#endif

/* Local variables */
static  uint8_t             MTOUCH_mostPressed_index;
static  MTOUCH_DELTA_t      MTOUCH_mostPressed_delta;
        MTOUCH_DELTA_t      MTOUCH_mostPressed_threshold;


static  MTOUCH_BASELINE_t   MTOUCH_baseline                 [MTOUCH_SENSORS];
static  uint8_t             MTOUCH_baseline_count           [MTOUCH_SENSORS];

typedef uint16_t            RESET_COUNT_t;
        RESET_COUNT_t       MTOUCH_baseline_reset_count     [MTOUCH_SENSORS];

/* Local Functions */
static void             MTOUCH_Button_StateMachine      (uint8_t sensor);
static void             MTOUCH_Sensor_Baseline_Update   (uint8_t sensor);
static void             MTOUCH_Sensor_Delta_Calculate   (uint8_t sensor);


/*
 *  PUBLIC FUNCTION
 *
 *  Initializes all mTouch scanning variables, the ADC, and Timer0 which is used
 *  as the scan interrupt.
 */
void MTOUCH_Init(void)
{
    uint8_t i;
    
    for (i = (uint8_t)0; i < (uint8_t)MTOUCH_SENSORS; i++)
    {
        MTOUCH_button_state [i] = MTOUCH_STATE_initializing;
        MTOUCH_delta_scaling[i] = MTOUCH_DELTA_SCALING;
    }
    MTOUCH_mostPressed_threshold = MTOUCH_MOSTPRESSED_THRESHOLD;

    #if defined(CONFIGURABLE_BASELINEGAIN)
    MTOUCH_baselineGain = MTOUCH_BASELINE_GAIN;
    #endif
    #if defined(CONFIGURABLE_BASELINERATE)
    MTOUCH_baselineRate = MTOUCH_BASELINE_RATE;
    #endif
    #if defined(CONFIGURABLE_NEGDELTACOUNTS)
    MTOUCH_negDeltaCount = MTOUCH_NEGATIVEDELTA_COUNTS;
    #endif
    #if defined(CONFIGURABLE_PRESSTIMEOUT)
    MTOUCH_pressTimeout = MTOUCH_PRESSTIMEOUT_COUNTS;
    #endif
    #if defined(CONFIGURABLE_DEBOUNCE)
    MTOUCH_debounce = MTOUCH_DEBOUNCE;
    #endif
    
    MTOUCH_Scan_Init();

    #if defined(MTOUCH_PROXIMITY_ENABLED)
    MTOUCH_Proximity_Init();
    #endif

    #if defined(MTOUCH_SLIDER_ENABLED)
    MTOUCH_Slider_Init();
    #endif

    #if defined(MTOUCH_MATRIX_ENABLED)
    MTOUCH_Matrix_Init();
    #endif
}

/*
 *  PUBLIC FUNCTION
 *
 *  An mTouch service function designed to be called at a regular 10ms rate out
 *  of the main loop. Calls the sensor state machine on any sensor with new data
 *  to decode, and updates the baseline at a fixed rate.
 */
void MTOUCH_Decode(void)
{
    uint8_t i;
    MTOUCH_DELTA_t delta;
    MTOUCH_DELTA_t biggestDelta = 0;
    MTOUCH_DELTA_t secondDelta  = 0;

    for (i = (uint8_t)0; i < (uint8_t)MTOUCH_SENSORS; i++)
    {
        MTOUCH_Sensor_Delta_Calculate(i);
    }

#if defined(CONFIGURABLE_MOSTPRESSED)
    if (MTOUCH_flags.mostPressed == 1)
    {
#endif
        for (i = (uint8_t)0; i < (uint8_t)MTOUCH_SENSORS; i++)
        {
            delta = MTOUCH_Sensor_Delta_Get(i);
            if (delta > biggestDelta)
            {
                secondDelta = biggestDelta;
                biggestDelta = delta;
                MTOUCH_mostPressed_index = i;
                MTOUCH_mostPressed_delta = biggestDelta - secondDelta;
            }
            else if (delta > secondDelta)
            {
                secondDelta = delta;
                MTOUCH_mostPressed_delta = biggestDelta - secondDelta;
            }
        }
#if defined(CONFIGURABLE_MOSTPRESSED)
    }
#endif

    for (i = (uint8_t)0; i < (uint8_t)MTOUCH_SENSORS; i++)
    {
        MTOUCH_Button_StateMachine(i);
    }

    #if defined(MTOUCH_PROXIMITY_ENABLED)
    MTOUCH_Proximity_Decode();
    #endif

    #if defined(MTOUCH_SLIDER_ENABLED)
    MTOUCH_Slider_Decode();
    #endif

    #if defined(MTOUCH_MATRIX_ENABLED)
    MTOUCH_Matrix_Decode();
    #endif

    for (i = (uint8_t)0; i < (uint8_t)MTOUCH_SENSORS; i++)
    {
        MTOUCH_baseline_count[i]++;
        #if defined(CONFIGURABLE_BASELINERATE)
        if (MTOUCH_baseline_count[i] >= (uint8_t)MTOUCH_baselineRate)
        #else
        if (MTOUCH_baseline_count[i] >= (uint8_t)MTOUCH_BASELINE_RATE)
        #endif
        {
            MTOUCH_baseline_count[i] = (uint8_t)0;
            MTOUCH_Sensor_Baseline_Update(i);
        }
    }
}

/*
 *  PUBLIC FUNCTION
 *
 *  Safely returns the current reading value for the requested sensor.
 */
MTOUCH_READING_t MTOUCH_Sensor_Reading_Get(uint8_t sensor)
{
    return MTOUCH_reading[sensor];
}


/*
 *  PRIVATE FUNCTION
 *
 *  Updates the requested sensor's baseline, based on the current state of the
 *  sensor and the current reading value.
 */
static void MTOUCH_Sensor_Baseline_Update(uint8_t sensor)
{
    if (MTOUCH_button_state[sensor] == MTOUCH_STATE_initializing)
    {
        #if defined(CONFIGURABLE_BASELINEGAIN)
        MTOUCH_baseline[sensor] = (MTOUCH_BASELINE_t)((MTOUCH_BASELINE_t)(MTOUCH_Sensor_Reading_Get(sensor)) << (uint8_t)MTOUCH_baselineGain);
        #else
        MTOUCH_baseline[sensor] = (MTOUCH_BASELINE_t)((MTOUCH_BASELINE_t)(MTOUCH_Sensor_Reading_Get(sensor)) << (uint8_t)MTOUCH_BASELINE_GAIN);
        #endif
    }
    else if (MTOUCH_button_state[sensor] == MTOUCH_STATE_released)
    {
        MTOUCH_baseline[sensor] -= MTOUCH_Sensor_Baseline_Get(sensor);
        MTOUCH_baseline[sensor] += MTOUCH_Sensor_Reading_Get(sensor);
    }
    else
    {
        /* Do nothing when the sensor is pressed. */
    }
}

/*
 *  PUBLIC FUNCTION
 *
 *  Safely returns the current baseline value for the requested sensor. The
 *  value is scaled to have the same gain as the reading, so that it is directly
 *  comparable without any additional logic.
 */
MTOUCH_READING_t MTOUCH_Sensor_Baseline_Get(uint8_t sensor)
{
    #if defined(CONFIGURABLE_BASELINEGAIN)
    return (MTOUCH_READING_t)(MTOUCH_baseline[sensor] >> (uint8_t)MTOUCH_baselineGain);
    #else
    return (MTOUCH_READING_t)(MTOUCH_baseline[sensor] >> (uint8_t)MTOUCH_BASELINE_GAIN);
    #endif
}

/*
 *  PUBLIC FUNCTION
 *
 *  Safely returns the difference between the baseline and the current sensor
 *  reading. Protects against overflow and provides thread safety.
 */
MTOUCH_DELTA_t MTOUCH_Sensor_Delta_Get(uint8_t sensor)
{
    return MTOUCH_delta[sensor];
}

static void MTOUCH_Sensor_Delta_Calculate(uint8_t sensor)
{
    int32_t delta;
    uint8_t i;

    delta = (int32_t)((int32_t)(MTOUCH_Sensor_Reading_Get(sensor)) - (int32_t)(MTOUCH_Sensor_Baseline_Get(sensor)));

    for (i = MTOUCH_delta_scaling[sensor]; i > 0; i--)
    {
        delta = delta / 2;
    }

    if (delta > MTOUCH_DELTA_MAX)
    {
        delta = MTOUCH_DELTA_MAX;
    }
    if (delta < MTOUCH_DELTA_MIN)
    {
        delta = MTOUCH_DELTA_MIN;
    }

    MTOUCH_delta[sensor] = (MTOUCH_DELTA_t)delta;
}

/*
 *  PUBLIC FUNCTION
 *
 *  Implements the mTouch sensor decoding state machine. There are three states:
 *
 *    - MTOUCH_STATE_initializing establishes a new baseline and sets the sensor
 *      to 'released'.
 *    - MTOUCH_STATE_released checks for the delta to increase above the press
 *      threshold, and recalibrates if too many consecutive negative deltas.
 *    - MTOUCH_STATE_pressed checks for the delta to decrease below the release
 *      threshold, and recalibrates if pressed for too long.
 *    - The 'default' option resets the sensor to initializing.
 */
static void MTOUCH_Button_StateMachine(uint8_t sensor)
{
    MTOUCH_DELTA_t delta = MTOUCH_Sensor_Delta_Get(sensor);

    switch(MTOUCH_button_state[sensor])
    {
        /*
         * The initialization state sets the baseline to the current value and
         * then enters the released state. This state assumes that calibration
         * is complete.
         */
        case MTOUCH_STATE_initializing:
            MTOUCH_Sensor_Baseline_Update(sensor);
            MTOUCH_button_state[sensor] = MTOUCH_STATE_released;
            MTOUCH_baseline_reset_count[sensor] = (RESET_COUNT_t)0;
            break;

        /*
         * The released state checks for the delta to increase above the press
         * threshold, and recalibrates if too many consecutive negative deltas.
         */
        case MTOUCH_STATE_released:
            if (delta > MTOUCH_button_threshold[sensor])
            {
                #if defined(CONFIGURABLE_MOSTPRESSED)
                if ((MTOUCH_flags.mostPressed == 0) ||
                   ((MTOUCH_flags.mostPressed == 1) && (MTOUCH_mostPressed_index == sensor) && (MTOUCH_mostPressed_delta > MTOUCH_mostPressed_threshold)))
                {
                #else
                if ((MTOUCH_mostPressed_index == sensor) && (MTOUCH_mostPressed_delta > MTOUCH_mostPressed_threshold))
                {
                #endif

                    #if defined(CONFIGURABLE_DEBOUNCE)
                    if (MTOUCH_button_debounce[sensor] == MTOUCH_debounce)
                    #else
                    if (MTOUCH_button_debounce[sensor] == MTOUCH_DEBOUNCE)
                    #endif
                    {
                        MTOUCH_button_state[sensor] = MTOUCH_STATE_pressed;
                        MTOUCH_button_debounce[sensor] = 0;
                        #if defined(CONFIGURABLE_PRESSTIMEOUT)
                        MTOUCH_baseline_reset_count[sensor] = MTOUCH_pressTimeout;
                        #else
                        MTOUCH_baseline_reset_count[sensor] = MTOUCH_PRESSTIMEOUT_COUNTS;
                        #endif
                        MTOUCH_CALLBACK_PressOccurred(sensor);
                    }
                    else
                    {
                        MTOUCH_button_debounce[sensor]++;
                    }
                }
                else
                {
                    MTOUCH_button_debounce[sensor] = 0;
                }
            }
            else if (delta < 0)
            {
                MTOUCH_button_debounce[sensor] = 0;
                MTOUCH_baseline_reset_count[sensor]++;
                #if defined(CONFIGURABLE_NEGDELTACOUNTS)
                if (MTOUCH_baseline_reset_count[sensor] >= (uint16_t)MTOUCH_negDeltaCount)
                #else
                if (MTOUCH_baseline_reset_count[sensor] >= (uint16_t)MTOUCH_NEGATIVEDELTA_COUNTS)
                #endif
                {
                    /*
                     * If negative capacitance values are maintained for an
                     * extended period, recalibrate the ADDCAP register for this
                     * sensor and reset the baseline.
                     */
                    MTOUCH_Button_State_Reset(sensor);
                }
            }
            else
            {
                /* If the delta count is positive, reset the negative counter. */
                MTOUCH_baseline_reset_count[sensor] = (RESET_COUNT_t)0;
                MTOUCH_button_debounce[sensor] = 0;
            }
            break;

        /*
         * The pressed state checks for the delta to decrease below the release
         * threshold, and recalibrates if pressed for too long.
         */
        case MTOUCH_STATE_pressed:
            if (delta < (MTOUCH_button_threshold[sensor] / (uint8_t)2))
            {
                #if defined(CONFIGURABLE_DEBOUNCE)
                if (MTOUCH_button_debounce[sensor] == MTOUCH_debounce)
                #else
                if (MTOUCH_button_debounce[sensor] == MTOUCH_DEBOUNCE)
                #endif         
                {
                    MTOUCH_button_state[sensor] = MTOUCH_STATE_released;
                    MTOUCH_button_debounce[sensor] = 0;
                    MTOUCH_baseline_reset_count[sensor] = (RESET_COUNT_t)0;
                    MTOUCH_CALLBACK_ReleaseOccurred(sensor);
                }
                else
                {
                    MTOUCH_button_debounce[sensor]++;
                }
            }
            else
            {
                MTOUCH_button_debounce[sensor] = 0;
                if (MTOUCH_baseline_reset_count[sensor] == (RESET_COUNT_t)0)
                {
                    MTOUCH_Button_State_Reset(sensor);
                    MTOUCH_CALLBACK_ReleaseOccurred(sensor);
                }
            }
            break;

        /*
         * We should never reach this point. If so, recover by resetting the
         * sensor through a recalibration and reinitialization.
         */
        default:    MTOUCH_Button_State_Reset(sensor);
    }
}

/*
 *  PUBLIC FUNCTION
 */
enum MTOUCH_STATE MTOUCH_Button_State_Get(uint8_t sensor)
{
    return MTOUCH_button_state[sensor];
}

/*
 *  PUBLIC FUNCTION
 */
void MTOUCH_Button_State_Reset(uint8_t sensor)
{
    MTOUCH_button_state[sensor] = MTOUCH_STATE_initializing;
    MTOUCH_Scan_ResetCalibration(sensor);
}

void MTOUCH_Service_Tick(void)
{
    uint8_t i;

    for (i = 0; i < MTOUCH_SENSORS; i++)
    {
        if (MTOUCH_button_state[i] == MTOUCH_STATE_pressed)
        {
            if (MTOUCH_baseline_reset_count[i] != 0)
                MTOUCH_baseline_reset_count[i]--;
        }
    }
}