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
#include "hamming.h"

uint16_t MTOUCH_Hamming(uint8_t index, uint16_t value)
{
    uint8_t i = index;
    uint16_t output;
    uint16_t temp;

#if MTOUCH_SAMPLES == 32
    /* Final scaling amount: 16.50391 */

    if (i > (uint8_t)31)
    {
        output = (uint16_t)0;
    }
    else
    {
        if (i > (uint8_t)15)
        {
            i = (uint8_t)((uint8_t)(31) - i);
        }

    #if 0
        switch(i)
        {
            case 0:     output = (uint16_t)((         (value >> (uint8_t) 7)  + (value >> (uint8_t)10)) + (value >> (uint8_t)12)); /* 0.009033203 */ break;
            case 1:     output = (uint16_t)((         (value >> (uint8_t) 5)  + (value >> (uint8_t) 8)) + (value >> (uint8_t)10)); /* 0.035888672 */ break;
            case 2:     output = (uint16_t)((         (value >> (uint8_t) 4)  + (value >> (uint8_t) 6)) + (value >> (uint8_t)10)); /* 0.079345703 */ break;
            case 3:     output = (uint16_t) (         (value >> (uint8_t) 3)  + (value >> (uint8_t) 6)                          ); /* 0.138183594 */ break;
            case 4:     output = (uint16_t)((         (value >> (uint8_t) 2)  - (value >> (uint8_t) 5)) - (value >> (uint8_t) 7)); /* 0.209960938 */ break;
            case 5:     output = (uint16_t)((         (value >> (uint8_t) 2)  + (value >> (uint8_t) 5)) + (value >> (uint8_t) 6)); /* 0.29296875  */ break;
            case 6:     output = (uint16_t)((         (value >> (uint8_t) 1)  - (value >> (uint8_t) 3)) + (value >> (uint8_t) 7)); /* 0.382324219 */ break;
            case 7:     output = (uint16_t)((         (value >> (uint8_t) 1)  - (value >> (uint8_t) 5)) + (value >> (uint8_t) 7)); /* 0.476074219 */ break;
            case 8:     output = (uint16_t)((         (value >> (uint8_t) 1)  + (value >> (uint8_t) 4)) + (value >> (uint8_t) 7)); /* 0.571289063 */ break;
            case 9:     output = (uint16_t)((         (value >> (uint8_t) 1)  + (value >> (uint8_t) 3)) + (value >> (uint8_t) 5)); /* 0.6640625   */ break;
            case 10:    output = (uint16_t) ( value - (value >> (uint8_t) 2)                                                    ); /* 0.75        */ break;
            case 11:    output = (uint16_t)(((value - (value >> (uint8_t) 3)) - (value >> (uint8_t) 4)) + (value >> (uint8_t) 6)); /* 0.828125    */ break;
            case 12:    output = (uint16_t)(((value - (value >> (uint8_t) 3)) + (value >> (uint8_t) 6)) + (value >> (uint8_t) 9)); /* 0.892578125 */ break;
            case 13:    output = (uint16_t)(((value - (value >> (uint8_t) 4)) + (value >> (uint8_t) 7)) - (value >> (uint8_t)10)); /* 0.944335938 */ break;
            case 14:    output = (uint16_t)(((value - (value >> (uint8_t) 6)) - (value >> (uint8_t) 8)) - (value >> (uint8_t)11)); /* 0.979980469 */ break;
            case 15:    output = (uint16_t)(((value - (value >> (uint8_t) 9)) - (value >> (uint8_t)12))                         ); /* 0.997802734 */ break;
            default:    output = (uint16_t)0; break;
        }
    #else
        output = (uint16_t)0;
        switch(i)
        {
            case 0:     /* output = (value >> 7) + (value >> 10) + (value >> 12) */
                temp    = value >> 7;   output += temp;
                temp    = temp  >> 3;   output += temp;
                                        output += temp >> 2;
                break;
            case 1:     /* output = (value >> 5) + (value >> 8) + (value >> 10) */
                                        output  = value >> 5;
                temp    = value >> 8;   output += temp;
                                        output += temp >> 2;
                break;
            case 2:     /* output = (value >> 4) + (value >> 6) + (value >> 10) */
                temp    = value >> 4;   output += temp;
                temp    = temp  >> 2;   output += temp;
                                        output += temp >> 4;
                break;
            case 3:     /* output = (value >> 3) + (value >> 6) */
                temp    = value >> 3;   output += temp;
                                        output += temp  >> 3;
                break;
            case 4:     /* output = (value >> 2) - (value >> 5) - (value >> 7) */
                temp    = value >> 2;   output += temp;
                temp    = temp  >> 3;   output -= temp;
                                        output -= temp  >> 2;
                break;
            case 5:     /* output = (value >> 2) + (value >> 5) + (value >> 6) */
                temp    = value >> 2;   output += temp;
                temp    = temp  >> 3;   output += temp;
                                        output += temp  >> 1;
                break;
            case 6:     /* output = (value >> 1) - (value >> 3) + (value >> 7) */
                temp    = value >> 1;   output += temp;
                temp    = temp  >> 2;   output -= temp;
                                        output += temp  >> 4;
                break;
            case 7:     /* output = (value >> 1) - (value >> 5) + (value >> 7) */
                temp    = value >> 1;   output += temp;
                temp    = temp  >> 4;   output -= temp;
                                        output += temp  >> 2;
                break;
            case 8:     /* output = (value >> 1) + (value >> 4) + (value >> 7) */
                temp    = value >> 1;   output += temp;
                temp    = temp  >> 3;   output += temp;
                                        output += temp  >> 3;
                break;
            case 9:     /* output = (value >> 1) + (value >> 3) + (value >> 5) */
                temp    = value >> 1;   output += temp;
                temp    = temp  >> 2;   output += temp;
                                        output += temp  >> 2;
                break;
            case 10:    /* output = value - (value >> 2) */
                                        output += value;
                                        output -= value >> 2;
                break;
            case 11:    /* output = value - (value >> 3) - (value >> 4) + (value >> 6) */
                                        output += value;
                temp    = value >> 3;   output -= temp;
                temp    = temp  >> 1;   output -= temp;
                                        output += temp  >> 2;
                break;
            case 12:    /* output = value - (value >> 3) + (value >> 6) + (value >> 9) */
                                        output += value;
                temp    = value >> 3;   output -= temp;
                temp    = temp  >> 3;   output += temp;
                                        output += temp  >> 3;
                break;
            case 13:    /* output = value - (value >> 4) + (value >> 7) - (value >> 10) */
                                        output += value;
                temp    = value >> 4;   output -= temp;
                temp    = temp  >> 3;   output += temp;
                                        output -= temp  >> 3;
                break;
            case 14:    /* output = value - (value >> 6) - (value >> 8) - (value >> 11) */
                                        output += value;
                                        output -= value >> 6;
                temp    = value >> 8;   output -= temp;
                                        output -= temp  >> 3;
                break;
            case 15:    /* output = value - (value >> 9) - (value >> 12) */
                                        output += value;
                temp    = value >> 8;
                temp    = temp  >> 1;   output -= temp;
                                        output -= temp  >> 3;
                break;
            default: break;
        }
    #endif
    }
#elif MTOUCH_SAMPLES == 24
    /* Final scaling amount: 12.50681 */

    if (i > (uint8_t)23)
    {
        output = (uint16_t)0;
    }
    else
    {
        if (i > (uint8_t)11)
        {
            i = (uint8_t)23 - i;
        }

    #if 0
        switch(i)
        {
            case 0:     output = (uint16_t)(          (value >> (uint8_t) 6)                                                    );  /* 0.015625    */ break;
            case 1:     output = (uint16_t)(          (value >> (uint8_t) 4)  - (value >> (uint8_t)11)                          );  /* 0.062011719 */ break;
            case 2:     output = (uint16_t)((         (value >> (uint8_t) 3)  + (value >> (uint8_t) 7)) + (value >> (uint8_t) 9));  /* 0.134765625 */ break;
            case 3:     output = (uint16_t)((         (value >> (uint8_t) 2)  - (value >> (uint8_t) 6)) - (value >> (uint8_t) 9));  /* 0.232421875 */ break;
            case 4:     output = (uint16_t)((         (value >> (uint8_t) 2)  + (value >> (uint8_t) 4)) + (value >> (uint8_t) 5));  /* 0.34375     */ break;
            case 5:     output = (uint16_t)(          (value >> (uint8_t) 1)  - (value >> (uint8_t) 5)                          );  /* 0.46875     */ break;
            case 6:     output = (uint16_t)((         (value >> (uint8_t) 1)  + (value >> (uint8_t) 4)) + (value >> (uint8_t) 5));  /* 0.59375     */ break;
            case 7:     output = (uint16_t)((         (value >> (uint8_t) 1)  + (value >> (uint8_t) 2)) - (value >> (uint8_t) 5));  /* 0.71875     */ break;
            case 8:     output = (uint16_t)(((value - (value >> (uint8_t) 3)) - (value >> (uint8_t) 4)) + (value >> (uint8_t) 8));  /* 0.81640625  */ break;
            case 9:     output = (uint16_t)( (value - (value >> (uint8_t) 3)) + (value >> (uint8_t) 5)                          );  /* 0.90625     */ break;
            case 10:    output = (uint16_t)( (value - (value >> (uint8_t) 5)) - (value >> (uint8_t) 8)                          );  /* 0.96484375  */ break;
            case 11:    output = (uint16_t)(  value - (value >> (uint8_t) 8)                                                    );  /* 0.99609375  */ break;
            default:    output = (uint16_t)0; break;
        }
    #else
        output = (uint16_t)0;
        switch(i)
        {
            case 0:     /* output = (value >> 6) */
                output = value >> 6;
                break;
            case 1:     /* output = (value >> 4) - (value >> 11) */
                                        output  = value >> 4;
                temp    = value >> 8;   output -= temp >> 3;
                break;
            case 2:     /* output = (value >> 3) + (value >> 7) + (value >> 9) */
                temp    = value >> 3;   output += temp;
                temp    = temp  >> 4;   output += temp;
                                        output += temp >> 2;
                break;
            case 3:     /* output = (value >> 2) - (value >> 6) - (value >> 9) */
                temp    = value >> 2;   output += temp;
                temp    = temp  >> 4;   output -= temp;
                                        output -= temp >> 3;
                break;
            case 4:     /* output = (value >> 2) + (value >> 4) + (value >> 5) */
                temp    = value >> 2;   output += temp;
                temp    = temp  >> 2;   output += temp;
                                        output += temp  >> 1;
                break;
            case 5:     /* output = (value >> 1) - (value >> 5) */
                temp    = value >> 1;   output += temp;
                                        output -= temp  >> 4;
                break;
            case 6:     /* output = (value >> 1) - (value >> 4) + (value >> 5) */
                temp    = value >> 1;   output += temp;
                temp    = temp  >> 3;   output += temp;
                                        output += temp  >> 1;
                break;
            case 7:     /* output = (value >> 1) + (value >> 2) - (value >> 5) */
                temp    = value >> 1;   output += temp;
                temp    = temp  >> 1;   output += temp;
                                        output -= temp  >> 3;
                break;
            case 8:     /* output = value - (value >> 3) - (value >> 4) + (value >> 8) */
                                        output += value;
                temp    = value >> 3;   output -= temp;
                temp    = temp  >> 1;   output -= temp;
                                        output += value >> 8;
                break;
            case 9:     /* output = value - (value >> 3) + (value >> 5) */
                                        output += value;
                temp    = value >> 3;   output -= temp;
                                        output += temp  >> 2;
                break;
            case 10:    /* output = value - (value >> 5) + (value >> 8) */
                                        output += value;
                                        output -= value >> 5;
                                        output -= value >> 8;
                break;
            case 11:    /* output = value - (value >> 8) */
                                        output += value;
                                        output -= value >> 8;
                break;
            default: break;
        }
    #endif
    }
#else
#error HAMMING option is invalid.
#endif

    return output;
}
