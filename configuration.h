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

#ifndef CONFIGURATION_H
#define	CONFIGURATION_H

#if defined(_16LF1559)
#include "configuration_1559.h"
#endif

//#define DEBUG_I2C
//#define DEBUG_UART

#define UART_ENABLED
#define I2C_ENABLED

#define MTOUCH_BUTTON_THRESHOLD0    100
#define MTOUCH_BUTTON_THRESHOLD1    100
#define MTOUCH_BUTTON_THRESHOLD2    100
#define MTOUCH_BUTTON_THRESHOLD3    100
#define MTOUCH_BUTTON_THRESHOLD4    100
#define MTOUCH_BUTTON_THRESHOLD5    100

#define MTOUCH_NOISE_THRESHOLD      3

#define MTOUCH_SAMPLES                  32          /* Number of samples to take in one burst */
#define MTOUCH_READING_GAIN             1           /* Additional filter gain applied to readings */
#define MTOUCH_BASELINE_RATE            10          /*  */
#define MTOUCH_BASELINE_GAIN            4           /* Additional filter gain applied to baselines */
#define MTOUCH_DELTA_SCALING            9
#define MTOUCH_NEGATIVEDELTA_COUNTS     32          /*  */
#define MTOUCH_PRESSTIMEOUT_COUNTS      10          /* 0.5sec per count */
#define MTOUCH_MOSTPRESSED_THRESHOLD    10
#define MTOUCH_DEBOUNCE                 0

#define COUNT_0p5sec                    61

#define TXCON_MAX                   ((uint8_t)0x03)
#define TMR_FREQ_MIN_MAX            ((uint8_t)240)
#define TMR_FREQ_MIN                ((uint8_t)170)  /* The most important value to get right. This must be set so the minimum timer value is longer than the ISR service time. */
#define TMR_FREQ_INCR0              ((uint8_t)1)
#define TMR_FREQ_INCR1              ((uint8_t)3)
#define TMR_FREQ_INCR2              ((uint8_t)4)
#define TMR_FREQ_INCR3              ((uint8_t)7)
#define TMR_FREQ_MAX                ((uint8_t)UINT8_MAX)

#define ID_PRODUCT                  0xFF
#define ID_MANUFACTURER             0xBD
#define ID_REVISION                 0x00

#define ADC_BITS                    16          /* Left-aligned ADC result  */
#define MTOUCH_SAMPLE_GAIN          4           /* Based off 32-sample Hamming filter */



#define CONFIGURABLE_SAMPLETYPE
#define CONFIGURABLE_FILTERTYPE
#define CONFIGURABLE_NOISETHRESH
#define CONFIGURABLE_SCANRATE_MIN
//#define CONFIGURABLE_SCANRATE_TxCON
#define CONFIGURABLE_NUMBERSAMPLES
#define CONFIGURABLE_GUARD
#define CONFIGURABLE_READINGGAIN
#define CONFIGURABLE_BASELINEGAIN
#define CONFIGURABLE_BASELINERATE
#define CONFIGURABLE_NEGDELTACOUNTS
#define CONFIGURABLE_PRESSTIMEOUT
#define CONFIGURABLE_MOSTPRESSED
#define CONFIGURABLE_DEBOUNCE
#define CONFIGURABLE_SINGLERESULT
#define CONFIGURABLE_DIFFRESULT





#define ADC_MAX_OUTPUT                  ((uint24_t)65535)
#define HAMMING_SCALER                  ((float)16.50391)
/* The below value is equal to ADC_MAX_OUTPUT * HAMMING_SCALER. Do not edit. */
#define MTOUCH_ADDCAP_LIMIT_DONOTEDIT   ((int24_t)1081584)

/*
 * MTOUCH_SAMPLE_t Sizing
 *
 * Determine how large the sensor sample variable size needs to be based on:
 *
 *  - The number of bits in an ADC result
 *  - The gain caused by accumulating MTOUCH_SAMPLES samples
 *
 * Equation:
 *      ADC Bits + 1 + MTOUCH_SAMPLE_GAIN <= MTOUCH_SAMPLE_t
 */
#define MTOUCH_SAMPLE_BITS          (ADC_BITS + 1 + MTOUCH_SAMPLE_GAIN)
#if     MTOUCH_SAMPLE_BITS <= 16
typedef int16_t        MTOUCH_SAMPLE_t;
#elif   MTOUCH_SAMPLE_BITS <= 24
typedef int_least24_t  MTOUCH_SAMPLE_t;
#else
typedef int32_t        MTOUCH_SAMPLE_t;
#endif

/*
 * MTOUCH_READING_t Sizing
 *
 * Determine how large the sensor reading variable size needs to be based on:
 *
 *  - The number of bits in a sensor sample
 *  - The gain of the reading filter applied to the samples as they are read.
 *
 * Equation:
 *      MTOUCH_SAMPLE_BITS + MTOUCH_READING_GAIN <= MTOUCH_READING_t
 */
#define MTOUCH_READING_BITS         (MTOUCH_SAMPLE_BITS + MTOUCH_READING_GAIN)
#if     MTOUCH_READING_BITS <= 16
typedef uint16_t        MTOUCH_READING_t;
#elif   MTOUCH_READING_BITS <= 24
typedef uint_least24_t  MTOUCH_READING_t;
#else
typedef uint32_t        MTOUCH_READING_t;
#endif

typedef uint32_t        MTOUCH_BASELINE_t;

typedef int8_t              MTOUCH_DELTA_t;
#define MTOUCH_DELTA_MAX    ((MTOUCH_DELTA_t)INT8_MAX)
#define MTOUCH_DELTA_MIN    ((MTOUCH_DELTA_t)INT8_MIN)

#if     MTOUCH_READING_GAIN == 0
#define MTOUCH_READING_RISETIME 0
#elif   MTOUCH_READING_GAIN == 1
#define MTOUCH_READING_RISETIME 3
#elif   MTOUCH_READING_GAIN == 2
#define MTOUCH_READING_RISETIME 8
#elif   MTOUCH_READING_GAIN == 3
#define MTOUCH_READING_RISETIME 17
#endif

#endif	/* CONFIGURATION_H */

