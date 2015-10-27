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

#include <xc.h>
#include <stdint.h>
#include "configuration.h"
#include "mtouch.h"

#define TRIS_INPUT  ((uint8_t)1)
#define TRIS_OUTPUT ((uint8_t)0)

void MTOUCH_CALLBACK_PressOccurred(uint8_t sensor)
{
    #if (MTOUCH_SENSORS != 6)
    #warning("Output logic should be updated!")
    #endif

    switch(sensor)
    {
        //case 0:     LED0_TRIS = TRIS_OUTPUT; break;
        case 1:     LED1_TRIS = TRIS_OUTPUT; break;
        case 2:     LED2_TRIS = TRIS_OUTPUT; break;
        case 3:     LED3_TRIS = TRIS_OUTPUT; break;
        case 4:     LED4_TRIS = TRIS_OUTPUT; break;
        case 5:     LED5_TRIS = TRIS_OUTPUT; break;
        default:                             break;
    }
}

void MTOUCH_CALLBACK_ReleaseOccurred(uint8_t sensor)
{
    #if (MTOUCH_SENSORS != 6)
    #warning("Output logic should be updated!")
    #endif

    switch(sensor)
    {
        //case 0:     LED0_TRIS = TRIS_INPUT; break;
        case 1:     LED1_TRIS = TRIS_INPUT; break;
        case 2:     LED2_TRIS = TRIS_INPUT; break;
        case 3:     LED3_TRIS = TRIS_INPUT; break;
        case 4:     LED4_TRIS = TRIS_INPUT; break;
        case 5:     LED5_TRIS = TRIS_INPUT; break;
        default:                            break;
    }
}

#if defined(MTOUCH_SLIDER_ENABLED)
void MTOUCH_CALLBACK_SliderUpdate       (void)
{
    MTOUCH_SLIDER_t slider = MTOUCH_Slider_GetFiltered();
}
#endif

#if defined(MTOUCH_PROXIMITY_ENABLED)
void MTOUCH_CALLBACK_ProxActivated      (void)          {LED0_TRIS = TRIS_OUTPUT;}
void MTOUCH_CALLBACK_ProxDeactivated    (void)          {LED0_TRIS = TRIS_INPUT;}
#endif

#if defined(MTOUCH_MATRIX_ENABLED)
void MTOUCH_CALLBACK_Matrix_NodePressed (uint8_t node)  {}
void MTOUCH_CALLBACK_Matrix_NodeReleased(uint8_t node)  {}
#endif
