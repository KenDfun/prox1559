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

#ifndef CONFIGURATION_1559_H
#define	CONFIGURATION_1559_H

#ifndef _16LF1559
#error ("Wrong device.")
#endif

/* PIC16LF1559 Evaluation Board
 *                                 _________
 *                          VDD --|*        |-- VSS
 *                   CS5 / AN20 --|RA5   RA0|-- AN0  / LED5 / ICSPDAT
 *                ADTRIG / AN10 --|RA4   RA1|-- AN1  / LED4 / ICSPCLK
 *                   SDA / MCLR --|RA3   RA2|-- AN2  / CS4
 *                  LED3 / AN21 --|RC5   RC0|-- AN13 / CS3
 *               AD1GRDA / AN11 --|RC4   RC1|-- AN23 / CS2
 *           PWM2 / LED1 / AN22 --|RC3   RC2|-- AN12 / LED0 / PWM1
 *                   CS0 / AN14 --|RC6   RB4|-- AN26 / CS1
 *                  LED2 / AN24 --|RC7   RB5|-- AN16 / RX
 *                    TX / AN15 --|RB7   RB6|-- AN25 / SCL
 *                                 ---------
 */
#define MTOUCH_SENSORS              6           /* Number of sensors    */
#define MTOUCH_ADCS                 2           /* Number of ADCs       */

#define ADC_SENSOR0_ADCON0          0b00111001  /* AN14 */
#define ADC_SENSOR1_ADCON0          0b01101001  /* AN26 */
#define ADC_SENSOR2_ADCON0          0b01011101  /* AN23 */
#define ADC_SENSOR3_ADCON0          0b00110101  /* AN13 */
#define ADC_SENSOR4_ADCON0          0b00001001  /* AN2  */
#define ADC_SENSOR5_ADCON0          0b01010001  /* AN20 */

#define LED0_TRIS                   (TRISCbits.TRISC2)
#define LED1_TRIS                   (TRISCbits.TRISC3)
#define LED2_TRIS                   (TRISCbits.TRISC7)
#define LED3_TRIS                   (TRISCbits.TRISC5)
#define LED4_TRIS                   (TRISAbits.TRISA1)
#define LED5_TRIS                   (TRISAbits.TRISA0)

#define ADC_ADDCAP_MAX              7

#endif

