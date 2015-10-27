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

#if __XC8_VERSION != 1340
#warning("This code was developed using XC8 v1.34")
#endif

#include "main.h"
#include "configuration.h"
#include "mtouch.h"
#include "uart.h"
#include "i2c.h"

#ifdef _16LF1559
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = SWDTEN    // Watchdog Timer Enable (WDT controlled by the SWDTEN bit in the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config WRT = ALL        // Flash Memory Self-Write Protection (000h to FFFh write protected, no addresses may be modified by PMCON control)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), 1.9V trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)
#else
#error("This code was developed for the PIC16LF1559.")
#endif

extern MTOUCH_SAMPLE_t MTOUCH_lastSample[MTOUCH_SENSORS];

void interrupt  ISR         (void);
void            SYSTEM_Init (void);

void main(void)
{
    SYSTEM_Init();

    #if defined(I2C_ENABLED)
    I2C_Init();
    PIR1bits.SSP1IF = 0;
    PIE1bits.SSP1IE = 1;
    #endif

    #if defined(UART_ENABLED)
    UART_Init();
    PIR1bits.RCIF = 0;
    PIE1bits.RCIE = 1;
    #endif

    MTOUCH_Init();

    INTCONbits.PEIE = (uint8_t)1;
    INTCONbits.GIE  = (uint8_t)1;

    /**********************/
    /* START OF MAIN LOOP */
    /**********************/
    for(;;)
    {
        INTCONbits.TMR0IE = 0;
        LATAbits.LATA4 = 1;
        MTOUCH_Scan();                  /* Scan the sensors */
        LATAbits.LATA4 = 0;
        INTCONbits.TMR0IE = 1;
        
        MTOUCH_Decode();                /* Process the new data */

        #if defined(DEBUG_UART)
        UART_SendShortLong(MTOUCH_lastSample[0] );
        UART_SendShortLong(MTOUCH_Sensor_Reading_Get(0) );
        UART_SendShortLong(MTOUCH_Sensor_Baseline_Get(0));
        UART_SendSignedChar(MTOUCH_Sensor_Delta_Get(0));
        UART_SendShortLong(MTOUCH_lastSample[2] );
        UART_SendShortLong(MTOUCH_Sensor_Reading_Get(2) );
        UART_SendShortLong(MTOUCH_Sensor_Baseline_Get(2));
        UART_SendSignedChar(MTOUCH_Sensor_Delta_Get(2));
        UART_SendNewLine();
        #endif
    }
    /**********************/
    /* END OF MAIN LOOP   */
    /**********************/
}

void interrupt ISR(void)
{
    static uint8_t tick = 0;

    #if defined(UART_ENABLED)
    if (PIE1bits.RCIE == 1)
    {
        if (PIR1bits.RCIF == 1)
        {
            PIR1bits.RCIF = 0;
            UART_Receive_Service();
            MTOUCH_Notify_InterruptServiced();  /* Required. */
        }
    }
    #endif

    #if defined(I2C_ENABLED)
    if (PIE1bits.SSP1IE == 1)
    {
        if (PIR1bits.SSP1IF == 1)
        {
            PIR1bits.SSP1IF = 0;
            I2C_Service();
            MTOUCH_Notify_InterruptServiced();  /* Required. */
        }
    }
    #endif

    if (INTCONbits.TMR0IF == 1)
    {
    //LATAbits.LATA5 = 1;
        INTCONbits.TMR0IF = 0;

        tick++;
        if (tick == COUNT_0p5sec)
        {
            tick = 0;
            MTOUCH_Service_Tick();
        }
    //LATAbits.LATA5 = 0;
    }


    /* Other ISR services may be placed here. */
}

void SYSTEM_Init(void)
{
    OSCCON  = (uint8_t)0b11110000;  /* 32 MHz. No PLL.              */
    WDTCON  = (uint8_t)0;           /* Disable watchdog.            */
#if defined(DEBUG_I2C)
    APFCON  = (uint8_t)0b00000000;  /* AD1GRDA on RC4. SDA on RB4.  */
#else
    APFCON  = (uint8_t)0b00010000;  /* AD1GRDA on RC4. SDA on RA3.  */
#endif

#if defined(DEBUG_I2C)
    LATA    = (uint8_t)0;
    ANSELA  = (uint8_t)0;
    TRISA   = (uint8_t)0b00010011;
#else
    LATA    = (uint8_t)0;
    ANSELA  = (uint8_t)0;
    TRISA   = (uint8_t)0b00001011;
                /*         ||||||_____- LED5
                 *         |||||______- LED4
                 *         ||||_______- CS4
                 *         |||________- SDA
                 *         ||_________- ADTRIG
                 *         |__________- CS5
                 */
#endif

#if defined(DEBUG_I2C)
    LATB    = (uint8_t)0;
    ANSELB  = (uint8_t)0;
    TRISB   = (uint8_t)0b11110000;
#else
    LATB    = (uint8_t)0;
    ANSELB  = (uint8_t)0;
    TRISB   = (uint8_t)0b11100000;
                /*       ||||_________- CS1
                 *       |||__________- RX
                 *       ||___________- SCL
                 *       |____________- TX
                 */
#endif

    LATC    = (uint8_t)0;
    ANSELC  = (uint8_t)0;
    TRISC   = (uint8_t)0b10101100;
                /*       ||||||||_____- CS3
                 *       |||||||______- CS2
                 *       ||||||_______- LED0
                 *       |||||________- LED1
                 *       ||||_________- AD1GRDA
                 *       |||__________- LED3
                 *       ||___________- CS0
                 *       |____________- LED2
                 */

    OPTION_REG = (uint8_t)0b10000111;
    INTCONbits.TMR0IE = 1;
}
