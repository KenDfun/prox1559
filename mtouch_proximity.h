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

#ifndef __MTOUCH_PROXIMITY_H
#define __MTOUCH_PROXIMITY_H

#define MTOUCH_PROXIMITY_ENABLED

#define MTOUCH_PROXIMITY                    0       /* Index of the proximity sensor */

#define MTOUCH_PROX_GAIN                    2
#define MTOUCH_PROX_SCALING                 6

#define MTOUCH_PROX_ACTIVATE_THRESHOLD      100

#define MTOUCH_MEDIAN_FILTER_TAP            7


#if     (MTOUCH_PROX_GAIN + 7) <= 8
typedef uint8_t        MTOUCH_PROXIMITY_t;
#elif   (MTOUCH_PROX_GAIN + 7) <= 16
typedef uint16_t       MTOUCH_PROXIMITY_t;
#elif   (MTOUCH_PROX_GAIN + 7) <= 24
typedef uint_least24_t MTOUCH_PROXIMITY_t;
#elif   (MTOUCH_PROX_GAIN + 7) <= 32
typedef uint32_t       MTOUCH_PROXIMITY_t;
#endif    

void                MTOUCH_Proximity_Init       (void);
void                MTOUCH_Proximity_Decode     (void);
MTOUCH_DELTA_t      MTOUCH_Proximity_Delta_Get  (void);
void                MTOUCH_Proximity_Delta_Calculate(void);
enum MTOUCH_STATE   MTOUCH_Proximity_State_Get  (void);

#endif  // __MTOUCH_PROXIMITY_H