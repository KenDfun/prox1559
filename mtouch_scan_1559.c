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
#include "hamming.h"
#include "main.h"

enum SCAN_HANDLER_STATUS
{
    SCAN_HANDLER_good = 0,
    SCAN_HANDLER_error
};

// Global Variables
extern  enum MTOUCH_STATE       MTOUCH_button_state             [MTOUCH_SENSORS];
extern  uint24_t                MTOUCH_reading                  [MTOUCH_SENSORS];
extern  MTOUCH_DELTA_t          MTOUCH_button_threshold         [MTOUCH_SENSORS];
extern  uint8_t                 MTOUCH_delta_scaling            [MTOUCH_SENSORS];

#if defined(CONFIGURABLE_NOISETHRESH)
        int24_t                 MTOUCH_noise_threshold;
#endif
        uint8_t                 MTOUCH_frequency;
        uint8_t                 MTOUCH_frequency_min;
#if defined(CONFIGURABLE_NUMBERSAMPLES)
         int8_t                 MTOUCH_numberSamples;
#endif
        uint8_t                 MTOUCH_addcap                   [MTOUCH_SENSORS];
#if defined(CONFIGURABLE_READINGGAIN)
        uint8_t                 MTOUCH_readingGain;
#endif


// Local Variables
static uint16_t                 MTOUCH_capturedSamples          [MTOUCH_ADCS][MTOUCH_SAMPLES+1];

static MTOUCH_SAMPLE_t          MTOUCH_sample                   [MTOUCH_ADCS];
static uint24_t                 MTOUCH_noise                    [MTOUCH_ADCS];

static uint8_t                  MTOUCH_dataReady;
static  int8_t                  MTOUCH_sampleCounter;
static uint8_t                  MTOUCH_readingCounter           [MTOUCH_SENSORS];
static uint8_t                  MTOUCH_calibrated               [MTOUCH_SENSORS];
       MTOUCH_SAMPLE_t          MTOUCH_lastSample               [MTOUCH_SENSORS];

#if defined(CONFIGURABLE_SINGLERESULT)
       uint16_t                 MTOUCH_singleResult             [MTOUCH_SENSORS];
#endif
#if defined(CONFIGURABLE_DIFFRESULT)
       uint16_t                 MTOUCH_diffResult               [MTOUCH_SENSORS];
#endif

       MTOUCH_FLAGS_t           MTOUCH_flags;

       void interrupt           ISR                             (void);
static void                     Sample                          (uint8_t sensor1, uint8_t sensor2);
static enum SCAN_HANDLER_STATUS Scan_Handler                    (uint8_t sensor1, uint8_t sensor2);
static void                     Hamming_Handler                 (uint8_t adc,     uint8_t i);
static uint8_t                  Sample_Handler                  (uint8_t adc,     uint8_t sensor);
static uint8_t                  Noise_Check                     (uint8_t adc,     uint8_t sensor);
static void                     Noise_Calculation               (uint8_t adc);
static void                     FREQ_incrMin(void);

#if defined(CONFIGURABLE_SAMPLETYPE)
static uint8_t MTOUCH_Random(void);
#endif

void MTOUCH_Scan(void)
{
    #if (MTOUCH_SENSORS != 6)
    #warning("Sample function calls should be updated!")
    #endif

    /*                             ADC1, ADC2                            */
    Sample(    0,    2   );

#if !defined(DEBUG_I2C)
    Sample(    3,    1   );
#endif

#if !defined(DEBUG_UART)
    Sample(    4,    5   );
#endif
}

void MTOUCH_Scan_Init(void)
{
    for (uint8_t i = 0; i < MTOUCH_SENSORS; i++)
    {
        MTOUCH_readingCounter   [i] = (uint8_t)0;
        MTOUCH_addcap           [i] = (uint8_t)0;
        MTOUCH_calibrated       [i] = (uint8_t)0;
        MTOUCH_lastSample       [i] = (MTOUCH_SAMPLE_t)0;
    }
    
    MTOUCH_dataReady                = (uint8_t)0;
    MTOUCH_frequency                = (uint8_t)TMR_FREQ_MIN;
    MTOUCH_frequency_min            = (uint8_t)TMR_FREQ_MIN;

#if defined(CONFIGURABLE_NOISETHRESH)
    MTOUCH_noise_threshold          = MTOUCH_NOISE_THRESHOLD;
#endif
#if defined(CONFIGURABLE_NUMBERSAMPLES)
    MTOUCH_numberSamples            = MTOUCH_SAMPLES;
#endif
#if defined(CONFIGURABLE_READINGGAIN)
    MTOUCH_readingGain              = MTOUCH_READING_GAIN;
#endif

#if defined(CONFIGURABLE_SAMPLETYPE)
    MTOUCH_flags.active_nJitter = 1;
#endif
#if defined(CONFIGURABLE_GUARD)
    MTOUCH_flags.guard = 1;
#endif
#if defined(CONFIGURABLE_FITLERTYPE)
    MTOUCH_flags.hamming_nAccumulation = 1;
#endif

}

void MTOUCH_Scan_ResetCalibration(uint8_t sensor)
{
    MTOUCH_addcap    [sensor] = 0;
    MTOUCH_calibrated[sensor] = 0;
}

static void Sample(uint8_t sensor1, uint8_t sensor2)
{
    static const uint8_t    TMR_FREQUENCIES[4] =
    {
        (uint8_t)TMR_FREQ_INCR0,
        (uint8_t)TMR_FREQ_INCR1,
        (uint8_t)TMR_FREQ_INCR2,
        (uint8_t)TMR_FREQ_INCR3
    };

            uint8_t         freq;
            uint8_t         i;        
            uint8_t         best_frequency                    = (uint8_t)MTOUCH_frequency;
            uint24_t        best_noise                        = (uint24_t)0;
            MTOUCH_SAMPLE_t best_sample      [MTOUCH_ADCS];


    do
    {
        MTOUCH_dataReady = 1;
        
        /* Perform scan */
        PR2 = (uint8_t)MTOUCH_frequency;
        while(Scan_Handler(sensor1, sensor2) == SCAN_HANDLER_error);

#if defined(CONFIGURABLE_SAMPLETYPE)
        if (MTOUCH_flags.active_nJitter == 1)
        {
#endif
            if (   (MTOUCH_calibrated[sensor1] == 0)
                || (MTOUCH_calibrated[sensor2] == 0)
                || (Noise_Check(0, sensor1)    != 0)
                || (Noise_Check(1, sensor2)    != 0)
               )
            {
                best_frequency                    = (uint8_t)MTOUCH_frequency;
                best_noise                        = (uint24_t)0;
                
                /* Now find the best scanning frequency in case there's been a change in noise.*/
                for (i = (uint8_t)0; i < (uint8_t)4; i++)
                {
                    freq = MTOUCH_frequency + TMR_FREQUENCIES[i];
                    if (freq < MTOUCH_frequency_min) { freq += MTOUCH_frequency_min; }
                    PR2 = freq;
                    while(Scan_Handler(sensor1, sensor2) == SCAN_HANDLER_error);

                    if (MTOUCH_noise[0] > best_noise)
                    {
                        best_sample[0]  = MTOUCH_sample[0];
                        best_sample[1]  = MTOUCH_sample[1];
                        best_noise      = MTOUCH_noise[0];
                        best_frequency  = freq;
                    }
                    if (MTOUCH_noise[1] > best_noise)
                    {
                        best_sample[0]  = MTOUCH_sample[0];
                        best_sample[1]  = MTOUCH_sample[1];
                        best_noise      = MTOUCH_noise[1];
                        best_frequency  = freq;
                    }
                }
                MTOUCH_frequency = best_frequency;
                MTOUCH_sample[0] = best_sample[0];
                MTOUCH_sample[1] = best_sample[1];
            }
            MTOUCH_lastSample[sensor1] = MTOUCH_sample[0]; /* Set for next time. */
            MTOUCH_lastSample[sensor2] = MTOUCH_sample[1];
#if defined(CONFIGURABLE_SAMPLETYPE)
        }
#endif

        MTOUCH_dataReady &= Sample_Handler(0, sensor1);
        MTOUCH_dataReady &= Sample_Handler(1, sensor2);

    /* If the reading is not yet initialized, repeat this scan a few times
     * until it has had an opportunity to settle. */
    } while (MTOUCH_dataReady == (uint8_t)0);
}

static uint8_t Noise_Check(uint8_t adc, uint8_t sensor)
{
    int24_t delta;
    int24_t threshold   = MTOUCH_button_threshold[sensor];
            threshold <<= MTOUCH_delta_scaling[sensor];
            threshold >>= MTOUCH_READING_GAIN;
    #if defined(CONFIGURABLE_NOISETHRESH)
            threshold >>= MTOUCH_noise_threshold;
    #else
            threshold >>= MTOUCH_NOISE_THRESHOLD;
    #endif

    delta = MTOUCH_sample[adc] - MTOUCH_lastSample[sensor];
    if (delta < 0) { delta = -delta; }

    #if defined(CONFIGURABLE_NOISETHRESH)
    return (delta > threshold) ? (uint8_t)1 : (uint8_t)0;
    #else
    return (delta > threshold) ? (uint8_t)1 : (uint8_t)0;
    #endif
}

static uint8_t Sample_Handler(uint8_t adc, uint8_t sensor)
{
    int24_t delta;
    uint8_t dataReady = 0;

    if (MTOUCH_calibrated[sensor] == 0)
    {
        MTOUCH_calibrated[sensor] = 1;
//        if (MTOUCH_calibrated[sensor] == 0)
//        {
//            if (    (MTOUCH_sample[adc] < MTOUCH_ADDCAP_LIMIT_DONOTEDIT)
//                 || (MTOUCH_addcap[sensor] == ADC_ADDCAP_MAX)
//               )
//            {
//                MTOUCH_calibrated[sensor] = 1;
//            }
//            else
//            {
//                MTOUCH_addcap[sensor]++;
//            }
//        }
    }
    else
    {
        /* Integrate the latest sample with the reading through a simple IIR low
         * pass filter. This smoothes the reading used to decode the sensor. */
        MTOUCH_readingCounter[sensor]++;
        if ((MTOUCH_button_state[sensor] == MTOUCH_STATE_initializing) && (MTOUCH_readingCounter[sensor] == (uint8_t)1))
        {  
            #if defined(CONFIGURABLE_READINGGAIN)
            MTOUCH_reading[sensor] = (MTOUCH_READING_t)((MTOUCH_READING_t)((uint24_t)MTOUCH_sample[adc] << (uint8_t)MTOUCH_readingGain));
            #else
            MTOUCH_reading[sensor] = (MTOUCH_READING_t)((MTOUCH_READING_t)((uint24_t)MTOUCH_sample[adc] << (uint8_t)MTOUCH_READING_GAIN));
            #endif
        }
        else
        {
            delta = MTOUCH_sample[adc];
            #if defined(CONFIGURABLE_READINGGAIN)
            delta -= (MTOUCH_Sensor_Reading_Get(sensor) >> (uint8_t)MTOUCH_readingGain);
            #else
            delta -= (MTOUCH_Sensor_Reading_Get(sensor) >> (uint8_t)MTOUCH_READING_GAIN);
            #endif
            MTOUCH_reading[sensor] += delta;

            if (delta < 0) delta = -delta;

            if (MTOUCH_readingCounter[sensor] > (uint8_t)MTOUCH_READING_RISETIME)
            {
                MTOUCH_readingCounter[sensor]--;
                dataReady = (uint8_t)1;
            }
        }
    }
    
    return dataReady;
}

static enum SCAN_HANDLER_STATUS Scan_Handler(uint8_t sensor1, uint8_t sensor2)
{
    static const uint8_t MTOUCH_ADCON0[MTOUCH_SENSORS] =
    {
         ADC_SENSOR0_ADCON0
        #if (MTOUCH_SENSORS > 1)
        ,ADC_SENSOR1_ADCON0
        #endif
        #if (MTOUCH_SENSORS > 2)
        ,ADC_SENSOR2_ADCON0
        #endif
        #if (MTOUCH_SENSORS > 3)
        ,ADC_SENSOR3_ADCON0
        #endif
        #if (MTOUCH_SENSORS > 4)
        ,ADC_SENSOR4_ADCON0
        #endif
        #if (MTOUCH_SENSORS > 5)
        ,ADC_SENSOR5_ADCON0
        #endif
        #if (MTOUCH_SENSORS > 6)
        #error("Edit required in MTOUCH_ADCON0 array initialization to support additional sensors.")
        #endif
    };

    int8_t i;

    /* ADC initialization */
    AAD1CON0    = MTOUCH_ADCON0[sensor1];
    AAD1CAP     = MTOUCH_addcap[sensor1];
    AAD1CON2    = 0b01010000; /* TMR2 Trigger */
    AAD1CON3    = 0b01000000;
    AD1PRECON   = (uint8_t)20;
    AD1ACQCON   = (uint8_t)6;

    AAD2CON0    = MTOUCH_ADCON0[sensor2];
    AAD2CAP     = MTOUCH_addcap[sensor2];
    AAD2CON2    = 0b01010000; /* TMR2 Trigger */
    AAD2CON3    = 0b01000000;
    AD2PRECON   = (uint8_t)20;
    AD2ACQCON   = (uint8_t)6;

    AAD2GRD = 0;
#if defined(CONFIGURABLE_GUARD)
    if (MTOUCH_flags.guard == 1)    { AAD1GRD= 0b01000000; }
    else                            { AAD1GRD= 0b00000000; }
#else
    AAD1GRD = 0b01100000;
#endif

    /* Interrupt initialization */
    ADCOMCON                = (uint8_t)0b01010000;
    T2CON                   = (uint8_t)0;
#if defined(CONFIGURABLE_NUMBERSAMPLES)
    MTOUCH_sampleCounter    = MTOUCH_numberSamples;
#else
    MTOUCH_sampleCounter    = (int8_t)(MTOUCH_SAMPLES);
#endif

    MTOUCH_flags.check = (uint8_t)0;
    MTOUCH_flags.error = (uint8_t)0;
    MTOUCH_flags.done  = (uint8_t)0;
    T2CONbits.TMR2ON   = (uint8_t)1;
    do
    {
        /* If the next ADC sample is not yet complete, then our sampling rate
         * is sufficiently slow to fit our processing between samples. This is
         * good. If the conversion is already complete, we will error out of
         * this packet because we may be missing data. */
        if      (PIR1bits.AD1IF == 0)   { MTOUCH_flags.check = 0; }
        while   (PIR1bits.AD1IF == 0)   { }
        PIR1bits.AD1IF  = 0;
        

        AAD1CON3 ^= 0b11000000; /* Toggle precharge polarities  */
        AAD2CON3 ^= 0b11000000;
        AAD1GRD  ^= 0b00100000; /* Toggle guard polarity        */

        MTOUCH_capturedSamples[0][MTOUCH_sampleCounter] = AAD1RES0;
        MTOUCH_capturedSamples[1][MTOUCH_sampleCounter] = AAD2RES0;

        MTOUCH_sampleCounter--;
        if (MTOUCH_sampleCounter < (int8_t)0)
        {
            /* Complete packet. Perform final storage steps. */
            MTOUCH_flags.done   = (uint8_t)1;
        }

        #if defined(CONFIGURABLE_SAMPLETYPE)
        if (MTOUCH_flags.active_nJitter == 0)
        {
            AD1CON0bits.ADON = 0;
            AD2CON0bits.ADON = 0;       /* Stop possible conversion. */
            T2CONbits.TMR2ON = 0;
            PR2 = MTOUCH_Random();      /* Randomize next TMR2 trigger. */
            if (PR2 == 0) { PR2++; }    /* Do not allow PR2 == 0. */
            PIR1bits.AD1IF   = 0;       /* Clear ADC flag */
            AD1CON0bits.ADON = 1;
            AD2CON0bits.ADON = 1;
            T2CONbits.TMR2ON = 1;       /* Begin next scanning phase. */
        }
        #endif

        if (MTOUCH_flags.check != 0)
        {
            #if defined(CONFIGURABLE_SAMPLETYPE)
            if (MTOUCH_flags.active_nJitter != 0)
            {
            #endif
                MTOUCH_flags.error = 1;
                MTOUCH_flags.done  = 1;
            #if defined(CONFIGURABLE_SAMPLETYPE)
            }
            #endif
        }
        MTOUCH_flags.check = 1;

    } while(MTOUCH_flags.done == 0);
    T2CONbits.TMR2ON        = 0;
    AAD1CON0bits.ADON       = 0;
    AAD2CON0bits.ADON       = 0;

    if (MTOUCH_flags.error == 1)
    {
        return SCAN_HANDLER_error;
    }

    MTOUCH_sample[0] = 0;
    MTOUCH_sample[1] = 0;
    #if defined(CONFIGURABLE_NUMBERSAMPLES)
    for (i = (int8_t)((MTOUCH_numberSamples/2)-1); i >= 0; i--)
    #else
    for (i = (int8_t)((MTOUCH_SAMPLES/2)-1); i >= 0; i--)
    #endif
    {
        Hamming_Handler(0, i);
        Hamming_Handler(1, i);
    }

    /* Calculate high pass filter output */
    Noise_Calculation(0);
    Noise_Calculation(1);

    MTOUCH_singleResult[sensor1] = MTOUCH_capturedSamples[0][0];
    MTOUCH_singleResult[sensor2] = MTOUCH_capturedSamples[1][0];

#if defined(CONFIGURABLE_DIFFRESULT)
    MTOUCH_diffResult[sensor1]  = 0x8000;
    MTOUCH_diffResult[sensor1] += MTOUCH_capturedSamples[0][0] >> 1;
    MTOUCH_diffResult[sensor1] -= MTOUCH_capturedSamples[0][1] >> 1;
    MTOUCH_diffResult[sensor2]  = 0x8000;
    MTOUCH_diffResult[sensor2] += MTOUCH_capturedSamples[1][0] >> 1;
    MTOUCH_diffResult[sensor2] -= MTOUCH_capturedSamples[1][1] >> 1;
#endif

    return SCAN_HANDLER_good;
}

static void Hamming_Handler(uint8_t adc, uint8_t i)
{
    MTOUCH_SAMPLE_t sample = 0x20000;

    if ((i & 0x01) == 0)
    {
        sample += MTOUCH_capturedSamples[adc][i+1];
        sample -= MTOUCH_capturedSamples[adc][i];
        sample += MTOUCH_capturedSamples[adc][MTOUCH_SAMPLES-(i+1)];
        sample -= MTOUCH_capturedSamples[adc][MTOUCH_SAMPLES-i];
    }
    else
    {
        sample += MTOUCH_capturedSamples[adc][i];
        sample -= MTOUCH_capturedSamples[adc][i+1];
        sample += MTOUCH_capturedSamples[adc][MTOUCH_SAMPLES-i];
        sample -= MTOUCH_capturedSamples[adc][MTOUCH_SAMPLES-(i+1)];
    }
    sample >>= 2;

    #if defined(CONFIGURABLE_FILTERTYPE)
    if (MTOUCH_flags.hamming_nAccumulation == 1)
    {
        MTOUCH_sample[adc] += (MTOUCH_SAMPLE_t)MTOUCH_Hamming(i, sample);
    }
    else
    {
        MTOUCH_sample[adc] += sample;
    }
    #else
    MTOUCH_sample[adc] += (MTOUCH_SAMPLE_t)MTOUCH_Hamming(i, sample);
    #endif
}

static void Noise_Calculation(uint8_t adc)
{
    int24_t delta;
    uint8_t i;
    
    MTOUCH_noise[adc] = 0;
    for (i = (uint8_t)1; i < (uint8_t)MTOUCH_SAMPLES; i++)
    {
        delta  = MTOUCH_capturedSamples[adc][i-(uint8_t)1];
        delta -= MTOUCH_capturedSamples[adc][i+(uint8_t)1];
        if (delta < 0) { delta = -delta; }
        MTOUCH_noise[adc] += (uint24_t)delta;
    }
}

void MTOUCH_Notify_InterruptServiced(void)
{
    MTOUCH_flags.error = 1;
}

#if defined(CONFIGURABLE_SAMPLETYPE)
/*
 *  PRIVATE FUNCTION
 *
 *  Generates a pseudo-random value based on the Galois LFSR algorithm.
 *
 *  If MTOUCH_RANDOM_BIT is 8, then an 8-bit value is used for the seed and
 *  the output of the function will repeat after 2^8 function calls.
 *
 *  If MTOUCH_RANDOM_BIT is 16, then a 16-bit value is used for the seed and
 *  the output of the function will repeat after 2^16 function calls.
 *
 *  If MTOUCH_RANDOM_BIT is 24, then a 24-bit value is used for the seed and
 *  the output of the function will repeat after 2^24 function calls.
 *
 *  If MTOUCH_RANDOM_BIT is 32, then a 32-bit value is used for the seed and
 *  the output of the function will repeat after 2^32 function calls.
 *
 *
 *  NOTE: This function is completely self-contained. You can copy and paste
 *  this into another project without needing to include any other code.
 *
 *
 *  @prototype      uint8_t MTOUCH_Random(void)
 *
 *  @return         a pseudo-random value. Not sufficient for any algorithm
 *                  needing true randomization such as cryptography. Plenty
 *                  of randomness to implement mTouch sample rate jittering.
 */

#define MTOUCH_RANDOM_BIT   8

#if     MTOUCH_RANDOM_BIT == 8
    #define MTOUCH_RANDOM_XOR   0xB4
    typedef uint8_t MTOUCH_RANDOM_t;
#elif   MTOUCH_RANDOM_BIT == 16
    #define MTOUCH_RANDOM_XOR   0xA1A1
    typedef uint16_t MTOUCH_RANDOM_t;
#elif   MTOUCH_RANDOM_BIT == 24
    #define MTOUCH_RANDOM_XOR   0xD7D7D7
    typedef uint_least24_t MTOUCH_RANDOM_t;
#elif   MTOUCH_RANDOM_BIT == 32
    #define MTOUCH_RANDOM_XOR   0xA6A6A6A6
    typedef uint32_t MTOUCH_RANDOM_t;
#endif

static uint8_t MTOUCH_Random(void)
{
    static MTOUCH_RANDOM_t seed = (MTOUCH_RANDOM_t)0;

    if ((seed & (MTOUCH_RANDOM_t)1) != (MTOUCH_RANDOM_t)0)
    {
        /* If the LSb of "seed" is a '1' then divide seed by 2. */
        seed = seed >> (uint8_t)1;
    }
    else
    {
        /*
         * If the LSb of "seed" is a '0' then divide seed by 2, and XOR it with
         * the LFSR polynom.
         */
        seed = seed >> (uint8_t)1;
        seed = seed ^ (MTOUCH_RANDOM_t)MTOUCH_RANDOM_XOR;
    }

    return (uint8_t)seed;
}
#endif
