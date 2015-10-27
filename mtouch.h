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

#ifndef MTOUCH_H
#define	MTOUCH_H

enum MTOUCH_STATE
{
    MTOUCH_STATE_initializing = 0,
    MTOUCH_STATE_released,
    MTOUCH_STATE_pressed
};

typedef struct
{
    unsigned check:1;
    unsigned error:1;
    unsigned done:1;
#if defined(CONFIGURABLE_FILTERTYPE)
    unsigned hamming_nAccumulation:1;
#endif
#if defined(CONFIGURABLE_SAMPLETYPE)
    unsigned active_nJitter:1;
#endif
#if defined(CONFIGURABLE_GUARD)
    unsigned guard:1;
#endif
#if defined(CONFIGURABLE_MOSTPRESSED)
    unsigned mostPressed:1;
#endif
} MTOUCH_FLAGS_t;

#include "mtouch_proximity.h"
#include "mtouch_slider3.h"
#include "mtouch_matrix.h"

enum MTOUCH_TIMEOUT
{
    MTOUCH_TIMEOUT_vdd = 0,
    MTOUCH_TIMEOUT_toggle,
    MTOUCH_TIMEOUT_vss
};

void                MTOUCH_Init                     (void);
void                MTOUCH_Scan                     (void);
void                MTOUCH_Decode                   (void);
void                MTOUCH_SetThreshold             (uint24_t threshold);
MTOUCH_READING_t    MTOUCH_Sensor_Reading_Get       (uint8_t sensor);
MTOUCH_READING_t    MTOUCH_Sensor_Baseline_Get      (uint8_t sensor);
void                MTOUCH_ISR_Service              (void);
void                MTOUCH_Notify_InterruptServiced (void);
void                MTOUCH_Button_State_Reset       (uint8_t sensor);
void                MTOUCH_Service_Tick             (void);

enum MTOUCH_STATE   MTOUCH_Button_State_Get         (uint8_t sensor);

MTOUCH_DELTA_t      MTOUCH_Sensor_Delta_Get         (uint8_t sensor);

void                MTOUCH_Scan_Init                (void);
void                MTOUCH_Scan_ResetCalibration    (uint8_t sensor);


/*
 *  CALLBACK FUNCTION
 *
 *  This callback function is called by the mTouch button state machine
 *  whenever a sensor is pressed.
 *
 *  @prototype      void MTOUCH_CALLBACK_PressOccurred(uint8_t sensor);
 *
 *  @parameter      sensor      the index of the sensor that is now pressed
 */
void MTOUCH_CALLBACK_PressOccurred(uint8_t sensor);

/*
 *  CALLBACK FUNCTION
 *
 *  This callback function is called by the mTouch button state machine
 *  whenever a sensor is released. This function is also called if a sensor
 *  press times-out and is forced to reinitialize.
 *
 *  @prototype      void MTOUCH_CALLBACK_ReleaseOccurred(uint8_t sensor);
 *
 *  @parameter      sensor      the index of the sensor that is now released
 */
void MTOUCH_CALLBACK_ReleaseOccurred(uint8_t sensor);

#endif	/* ADC_H */

