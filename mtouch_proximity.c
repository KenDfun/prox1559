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

#include <stdint.h>

#include "configuration.h"
#include "mtouch.h"
#include "mtouch_proximity.h"

#if defined(MTOUCH_PROXIMITY_ENABLED)
/* Local Variables */
static uint32_t             MTOUCH_prox_reading;
static uint32_t             MTOUCH_prox_baseline;
static uint32_t             MTOUCH_prox_filter;
static enum MTOUCH_STATE    MTOUCH_prox_state;
static MTOUCH_DELTA_t      MTOUCH_Proximity_Delta;
static uint8_t              tap_index[MTOUCH_MEDIAN_FILTER_TAP];
static uint32_t             filter_data[MTOUCH_MEDIAN_FILTER_TAP];


extern void MTOUCH_CALLBACK_ProxActivated   (void);
extern void MTOUCH_CALLBACK_ProxDeactivated (void);

MTOUCH_DELTA_t       MTOUCH_proximity_threshold;
uint8_t              MTOUCH_proximity_scaling;    

/* Local Prototypes */
static void MTOUCH_Proximity_StateMachine   (void);
static void MTOUCH_Proximity_Update         (void);
static uint32_t median_filter(uint32_t );

/*
 *  PUBLIC FUNCTION
 *
 *  Initializes all proximity variables.
 *
 *  @prototype      void MTOUCH_Proximity_Init(void)
 */
void MTOUCH_Proximity_Init(void)
{
    MTOUCH_prox_reading     = (uint32_t)0;
    MTOUCH_prox_baseline    = (uint32_t)0;
    MTOUCH_prox_filter      = (uint32_t)0;
    MTOUCH_prox_state       = MTOUCH_STATE_released;
    MTOUCH_proximity_threshold = (MTOUCH_DELTA_t)MTOUCH_PROX_ACTIVATE_THRESHOLD;
    MTOUCH_proximity_scaling = (uint8_t)MTOUCH_PROX_SCALING;
    
    /*Initialize Median filter*/
    for (uint8_t i=0;i<MTOUCH_MEDIAN_FILTER_TAP;i++)
    {
        tap_index[i]=i;
        filter_data[i] = 0;
    }
}

/*
 *  PUBLIC FUNCTION
 *
 *  This is the API function call for decoding the proximity sensor.
 *
 *  @prototype      void MTOUCH_Proximity_Decode(void)
 */
void MTOUCH_Proximity_Decode(void)
{
    MTOUCH_Proximity_Update();
    MTOUCH_Proximity_StateMachine();
}

/*
 *  PUBLIC FUNCTION
 *
 *  Calculate the current proximity sensor delta value.
 *
 *  @prototype      MTOUCH_DELTA_t MTOUCH_Proximity_Delta_Calculate(void)
 */
void MTOUCH_Proximity_Delta_Calculate(void)
{
    uint32_t            reading;

    reading = MTOUCH_prox_filter >> MTOUCH_proximity_scaling;

    if (reading > (uint32_t)INT8_MAX)
    {
        MTOUCH_Proximity_Delta = (MTOUCH_DELTA_t)INT8_MAX;
    }
    else
    {
        MTOUCH_Proximity_Delta = (MTOUCH_DELTA_t)reading;
    }
}

MTOUCH_DELTA_t      MTOUCH_Proximity_Delta_Get  (void)
{
    return MTOUCH_Proximity_Delta;
}

/*
 *  PUBLIC FUNCTION
 *
 *  Provides the current proximity sensor state.
 *
 *  @prototype      enum MTOUCH_STATE MTOUCH_Proximity_State_Get(void)
 */
enum MTOUCH_STATE MTOUCH_Proximity_State_Get(void)
{
    return MTOUCH_prox_state;
}

/*
 *  PRIVATE FUNCTION
 *
 *  The state machine for the proximity sensor. Very similar to the button state
 *  machine, but with fewer features.
 *
 *  @prototype      static void MTOUCH_Proximity_StateMachine(void)
 */
static void MTOUCH_Proximity_StateMachine(void)
{
    MTOUCH_Proximity_Delta_Calculate();
    switch(MTOUCH_prox_state)
    {
        /*
         * 
         */
        case MTOUCH_STATE_initializing:
            MTOUCH_prox_state = MTOUCH_STATE_released;
            break;

        /*
         * 
         */
        case MTOUCH_STATE_released:
            if (MTOUCH_Proximity_Delta_Get() > MTOUCH_proximity_threshold)
            {
                MTOUCH_prox_state = MTOUCH_STATE_pressed;
                
                /*lint -e522*/
                MTOUCH_CALLBACK_ProxActivated();  /* Do not care if this has no side effects. */
                /*lint +e522*/
            }
            break;

        /*
         * 
         */
        case MTOUCH_STATE_pressed:
            if (MTOUCH_Proximity_Delta_Get() < (MTOUCH_proximity_threshold>>1))
            {
                MTOUCH_prox_state = MTOUCH_STATE_released;
                MTOUCH_prox_baseline = MTOUCH_prox_reading;

                /*lint -e522*/
                MTOUCH_CALLBACK_ProxDeactivated();    /* Do not care if this has no side effects. */
                /*lint +e522*/
            }
            break;

        /*
         * We should never reach this point.
         */
        default:    MTOUCH_prox_state = MTOUCH_STATE_initializing;
    }
}

/*
 *  PRIVATE FUNCTION
 *
 *  Updates the current proximity sensor readings based on the current sensor's
 *  values.
 *
 *  @prototype      static void MTOUCH_Proximity_Update(void)
 */
static void MTOUCH_Proximity_Update(void)
{
    int32_t delta;

    MTOUCH_prox_reading  -= MTOUCH_prox_reading >> MTOUCH_PROX_GAIN;
    MTOUCH_prox_reading  +=median_filter(MTOUCH_Sensor_Reading_Get((uint8_t)MTOUCH_PROXIMITY));
    
    MTOUCH_prox_baseline = MTOUCH_Sensor_Baseline_Get((uint8_t)MTOUCH_PROXIMITY)<<MTOUCH_PROX_GAIN;

    delta = (int32_t)(MTOUCH_prox_reading - MTOUCH_prox_baseline);

    /* Clipping the negative side of the delta values. */
    if (delta < (int32_t)0)
    { 
        delta = (int32_t)0;
    }

    MTOUCH_prox_filter -= MTOUCH_prox_filter >> (uint8_t)MTOUCH_PROX_GAIN;
    MTOUCH_prox_filter += (uint32_t)delta;

}


static uint32_t median_filter(uint32_t new_data)
{
    uint8_t i = (uint8_t)0;
    static uint8_t deleted;
    uint32_t temp_data;
    uint8_t new_index,temp_index;

    new_index=deleted;
    while (tap_index[i]!=deleted)
    {
        if (filter_data[i]>new_data)
        {
            temp_data=filter_data[i];
            temp_index=tap_index[i];
            filter_data[i]=new_data;
            tap_index[i]=new_index;
            new_data=temp_data;
            new_index=temp_index;
        }
        i++;
    }
    filter_data[i]=new_data;
    tap_index[i]=new_index;
    for (;i<MTOUCH_MEDIAN_FILTER_TAP-1;i++)
    {
        if(new_data>filter_data[i+1])
        {
            filter_data[i]=filter_data[i+1];
            tap_index[i]=tap_index[i+1];
            filter_data[i+1]=new_data;
            tap_index[i+1]=new_index;
        }
        else
            break;
    }
    if (++deleted>=MTOUCH_MEDIAN_FILTER_TAP)
        deleted = (uint8_t)0;

    return filter_data[MTOUCH_MEDIAN_FILTER_TAP/2];
}

#endif
