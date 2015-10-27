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
#include "mtouch_slider3.h"

#if defined(MTOUCH_SLIDER_ENABLED)
extern void MTOUCH_CALLBACK_SliderUpdate(void);

static MTOUCH_SLIDER_t MTOUCH_slider;
static uint16_t        MTOUCH_slider_filter;
static uint8_t         MTOUCH_slider_sensorDelta   [MTOUCH_SLIDER_SENSORS];
        
/* Local Functions */
static void     MTOUCH_Slider_StateMachine      (void);
static uint16_t MTOUCH_Slider_Decode_FarLeft    (void);
static uint16_t MTOUCH_Slider_Decode_FarRight   (void);
static uint16_t MTOUCH_Slider_Decode_Right      (void);
static uint16_t MTOUCH_Slider_Decode_Left       (void);
static void     MTOUCH_Slider_Update            (MTOUCH_SLIDER_t newValue);

/* Validate configuration */
#if     (MTOUCH_SLIDER_SENSORS != 3)
    #error("This slider firmware is valid only for three sensor sliders.")
#elif   !defined(MTOUCH_SLIDER_SENSOR0) || \
        !defined(MTOUCH_SLIDER_SENSOR1) || \
        !defined(MTOUCH_SLIDER_SENSOR2)
    #error("Slider firmware requires definitions for Sensors 0-2 of the slider.")
#elif   !defined(MTOUCH_SLIDER_SENSOR0_THRESHOLD) || \
        !defined(MTOUCH_SLIDER_SENSOR1_THRESHOLD) || \
        !defined(MTOUCH_SLIDER_SENSOR2_THRESHOLD)
    #error("Slider firmware requires definitions for Sensors' 0-2 thresholds.")
#endif

void MTOUCH_Slider_Init(void)
{
    MTOUCH_slider                   = (MTOUCH_SLIDER_t)0;
    MTOUCH_slider_filter            = (uint16_t)0;
    MTOUCH_slider_sensorDelta[0]    = (uint8_t)0;
    MTOUCH_slider_sensorDelta[1]    = (uint8_t)0;
    MTOUCH_slider_sensorDelta[2]    = (uint8_t)0;
}

MTOUCH_SLIDER_t MTOUCH_Slider_GetFiltered(void)
{
    return (MTOUCH_SLIDER_t)(MTOUCH_slider_filter >> (uint8_t)MTOUCH_SLIDER_FILTER_GAIN);
}
MTOUCH_SLIDER_t MTOUCH_Slider_GetRaw(void)
{
    return MTOUCH_slider;
}
 
/*************************************************************************
 * mTouch_SliderDecode() - Slider Decoding Module
 ************************************************************************/
void MTOUCH_Slider_Decode(void)
{
    /*
     *  STORE ALL DELTA VALUES
     * ---------------------------------
     *  Regardless of the decision, store the absoulte value of the current
     *  deltas for each sensor.
     */
    MTOUCH_slider_sensorDelta[0] = (uint8_t)MTOUCH_Sensor_Delta_Get((uint8_t)MTOUCH_SLIDER_SENSOR0);
    MTOUCH_slider_sensorDelta[1] = (uint8_t)MTOUCH_Sensor_Delta_Get((uint8_t)MTOUCH_SLIDER_SENSOR1);
    MTOUCH_slider_sensorDelta[2] = (uint8_t)MTOUCH_Sensor_Delta_Get((uint8_t)MTOUCH_SLIDER_SENSOR2);

    if (MTOUCH_slider_sensorDelta[0] & (uint8_t)0x80) { MTOUCH_slider_sensorDelta[0] = (uint8_t)0; }
    if (MTOUCH_slider_sensorDelta[1] & (uint8_t)0x80) { MTOUCH_slider_sensorDelta[1] = (uint8_t)0; }
    if (MTOUCH_slider_sensorDelta[2] & (uint8_t)0x80) { MTOUCH_slider_sensorDelta[2] = (uint8_t)0; }

    /*
     *  DECODE THE SLIDER
     * ---------------------------------
     *  If any of the slider's sensors are being pressed,
     *  then decode the slider and get an updated position.
     */
    if (    (MTOUCH_slider_sensorDelta[0] > (uint8_t)MTOUCH_SLIDER_SENSOR0_THRESHOLD)
        ||  (MTOUCH_slider_sensorDelta[1] > (uint8_t)MTOUCH_SLIDER_SENSOR1_THRESHOLD)
        ||  (MTOUCH_slider_sensorDelta[2] > (uint8_t)MTOUCH_SLIDER_SENSOR2_THRESHOLD)
       )
    {
        MTOUCH_Slider_StateMachine();
        MTOUCH_CALLBACK_SliderUpdate();
    }
    else
    {
        if (MTOUCH_slider != (MTOUCH_SLIDER_t)0)
        {
            MTOUCH_slider = (MTOUCH_SLIDER_t)0;
            MTOUCH_CALLBACK_SliderUpdate();
        }
    }
}

static void MTOUCH_Slider_StateMachine(void)
{
    uint16_t newSlider;
    uint8_t  maxIndex;
    
    /*
     *  FIND THE MAX-PRESSED SENSOR
     * ---------------------------------
     *  The slider decode is based off which sensor is pressed the most. Start by finding
     *  which sensor that is.
     */
    if (MTOUCH_slider_sensorDelta[(uint8_t)0] > MTOUCH_slider_sensorDelta[(uint8_t)1])
    {
        if (MTOUCH_slider_sensorDelta[(uint8_t)0] > MTOUCH_slider_sensorDelta[(uint8_t)2])
        {
            maxIndex = (uint8_t)0;
        }
        else
        {
            maxIndex = (uint8_t)2;
        }
    } 
    else 
    {
        if (MTOUCH_slider_sensorDelta[(uint8_t)1] > MTOUCH_slider_sensorDelta[(uint8_t)2])
        {
            maxIndex = (uint8_t)1;
        }
        else
        {
            maxIndex = (uint8_t)2;
        }
    }
    
    
    /*
     *  POSITION-BASED DECODE
     * ---------------------------------
     *  Decode the slider based on which sensor is the most pressed and, in the case of
     *  the middle sensor, which neighbor is the most pressed.
     */
    if (maxIndex == (uint8_t)0)                                      /* Most pressed = Far Left  */
    {     
        newSlider = MTOUCH_Slider_Decode_FarLeft();
    }
    else if (maxIndex == (uint8_t)2)                                 /* Most pressed = Far Right */
    {   
        newSlider = MTOUCH_Slider_Decode_FarRight();
    }
    else                                                    /* Most pressed = Middle */
    {
        if (MTOUCH_slider_sensorDelta[(uint8_t)2] > MTOUCH_slider_sensorDelta[(uint8_t)0])
        {
            newSlider = MTOUCH_Slider_Decode_Right();       /* Biggest Neighbor = Right */
        }
        else
        {
            newSlider = MTOUCH_Slider_Decode_Left();        /* Biggest Neighbor = Left */
        }
    }

    
    /*
     *  SCALING
     * ---------------------------------
     *  Scale the new slider value to ensure a final result fits in MTOUCH_SLIDER_t.
     */
    newSlider = (uint16_t)(newSlider >> (uint8_t)1);
    if (newSlider > (uint16_t)UINT8_MAX)
    {
        newSlider = (uint16_t)UINT8_MAX;
    }
    
    /*
     *  SPEED LIMITATION
     * ---------------------------------
     *  Only allow the slider value to change by a certain amount each decode.
     */
    MTOUCH_Slider_Update((MTOUCH_SLIDER_t)newSlider);
}



static uint16_t MTOUCH_Slider_Decode_FarLeft(void)
{
    uint16_t newSlider;

    /*
     *      |           delta[0]                |
     *      | (  -------------------  ) - 0x100 | = Slider Value
     *      |    delta[0] + delta[1]            |
     */

    newSlider   = (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[0] << (uint8_t)8);
    newSlider  /= (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[0] + (uint16_t)MTOUCH_slider_sensorDelta[1]);

    if (newSlider > (uint16_t)0x100)
    {
        newSlider = newSlider - (uint16_t)0x100;
    }
    else
    {
        newSlider = (uint16_t)0x100 - newSlider;
    }

    return newSlider;
}

static uint16_t MTOUCH_Slider_Decode_FarRight(void)
{
    uint16_t newSlider;
    
    /*
     *                 delta[last]                                    
     *      (  ---------------------------  ) + 0x100 * buttonPosition  = Slider Value
     *         delta[last] + delta[last-1]
     */
     
    newSlider   = (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[1] << (uint8_t)8);
    newSlider  /= (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[2] + (uint16_t)MTOUCH_slider_sensorDelta[1]);
    newSlider   = (uint16_t)((uint16_t)2 << (uint8_t)8) - newSlider;
    
    return newSlider;
}

static uint16_t MTOUCH_Slider_Decode_Right(void)
{
    uint16_t newSlider;
    
    /*  Right neighbor of max index is larger than left neighbor
     *
     *
     *               delta[i+1]                                     
     *      (  ---------------------  )  + (0x100 * i) = Slider Value
     *         delta[i] + delta[i+1]
     */

    newSlider   = (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[2] << (uint8_t)8);
    newSlider  /= (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[1] + (uint16_t)MTOUCH_slider_sensorDelta[2]);
    newSlider  += (uint16_t)((uint16_t)1 << (uint8_t)8);

    return newSlider;
}

static uint16_t MTOUCH_Slider_Decode_Left(void)
{
    uint16_t newSlider;
    
    /*  Left neighbor of max index is larger than right neighbor
     *
     *
     *                             delta[i-1]    
     *      (0x100 * i) - (  ---------------------  )  = Slider Value
     *                       delta[i] + delta[i-1]                  
     */

    newSlider   = (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[0] << (uint8_t)8);
    newSlider  /= (uint16_t)((uint16_t)MTOUCH_slider_sensorDelta[1] + (uint16_t)MTOUCH_slider_sensorDelta[0]);
    newSlider   = (uint16_t)((uint16_t)1 << (uint8_t)8) - newSlider;
    
    return newSlider;
}

static void MTOUCH_Slider_Update(MTOUCH_SLIDER_t newValue)
{
    MTOUCH_slider_filter -= MTOUCH_slider_filter >> MTOUCH_SLIDER_FILTER_GAIN;
    MTOUCH_slider_filter += newValue;
    
    MTOUCH_slider         = newValue;
    if (MTOUCH_slider == (MTOUCH_SLIDER_t)0)
    {
        MTOUCH_slider = (MTOUCH_SLIDER_t)1; /* Minimum active slider value is '1'. */
    }
}
#endif
