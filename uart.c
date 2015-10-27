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
#include "uart.h"
#include "memory.h"

enum UART_STATES
{
    UART_STATE_idle = 0,
    UART_STATE_read,
    UART_STATE_write
};

void    UART_SendChar_Hex(uint8_t value);
uint8_t UART_HexToBinary(uint8_t* data);

void UART_Init(void)
{
#if defined(UART_ENABLED)
    TRISBbits.TRISB7 = 1;
    TXSTA   = 0b00100100;
    RCSTA   = 0b10000000;
    BAUDCON = 0b00001000;
    SPBRG   = 68;           /* 115200 @ 32MHz Fosc */
#endif
}

void UART_SendSignedLong(int32_t value)
{
    uint8_t d = (uint8_t)0;

    if (value < 0)
    {
        UART_PutChar('-');
        value = ~value;
        value++;
    }
    else
        UART_PutChar(' ');

    while (value >= (int32_t)1000000000) { value -= (int32_t)1000000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t) 100000000) { value -= (int32_t) 100000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)  10000000) { value -= (int32_t)  10000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)   1000000) { value -= (int32_t)   1000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)    100000) { value -= (int32_t)    100000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)     10000) { value -= (int32_t)     10000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)      1000) { value -= (int32_t)      1000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)       100) { value -= (int32_t)       100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)        10) { value -= (int32_t)        10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int32_t)         1) { value -= (int32_t)         1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendLong(uint32_t value)
{
    uint8_t d = (uint8_t)0;

    while (value >= (uint32_t)1000000000) { value -= (uint32_t)1000000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t) 100000000) { value -= (uint32_t) 100000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)  10000000) { value -= (uint32_t)  10000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)   1000000) { value -= (uint32_t)   1000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)    100000) { value -= (uint32_t)    100000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)     10000) { value -= (uint32_t)     10000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)      1000) { value -= (uint32_t)      1000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)       100) { value -= (uint32_t)       100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)        10) { value -= (uint32_t)        10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint32_t)         1) { value -= (uint32_t)         1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendShortLong(uint24_t value)
{
    uint8_t d = (uint8_t)0;

    while (value >= (uint24_t)10000000) { value -= (uint24_t)10000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t) 1000000) { value -= (uint24_t) 1000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t)  100000) { value -= (uint24_t)  100000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t)   10000) { value -= (uint24_t)   10000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t)    1000) { value -= (uint24_t)    1000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t)     100) { value -= (uint24_t)     100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t)      10) { value -= (uint24_t)      10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint24_t)       1) { value -= (uint24_t)       1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendSignedShortLong(int24_t value)
{
    uint8_t d = (uint8_t)0;

    if (value < 0)
    {
        UART_PutChar('-');
        value = ~value;
        value++;
    }
    else
        UART_PutChar(' ');

    while (value >= (int24_t)1000000) { value -= (int24_t)1000000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int24_t) 100000) { value -= (int24_t) 100000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int24_t)  10000) { value -= (int24_t)  10000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int24_t)   1000) { value -= (int24_t)   1000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int24_t)    100) { value -= (int24_t)    100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int24_t)     10) { value -= (int24_t)     10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int24_t)      1) { value -= (int24_t)      1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendInt(uint16_t value)
{
    uint8_t d = (uint8_t)0;

    while (value >= (uint16_t)10000) { value -= (uint16_t)10000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint16_t) 1000) { value -= (uint16_t) 1000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint16_t)  100) { value -= (uint16_t)  100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint16_t)   10) { value -= (uint16_t)   10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint16_t)    1) { value -= (uint16_t)    1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendSignedInt(int16_t value)
{
    uint8_t d = (uint8_t)0;

    if (value < 0)
    {
        UART_PutChar('-');
        value = ~value;
        value++;
    }
    else
        UART_PutChar(' ');

    while (value >= (int16_t)10000) { value -= (int16_t)10000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int16_t) 1000) { value -= (int16_t) 1000; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int16_t)  100) { value -= (int16_t)  100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int16_t)   10) { value -= (int16_t)   10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int16_t)    1) { value -= (int16_t)    1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendChar(uint8_t value)
{
    uint8_t d = (uint8_t)0;

    while (value >= (uint8_t)100) { value -= (uint8_t)100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint8_t) 10) { value -= (uint8_t) 10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (uint8_t)  1) { value -= (uint8_t)  1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

void UART_SendSignedChar(int8_t value)
{
    uint8_t d = (uint8_t)0;

    if (value < 0)
    {
        UART_PutChar('-');
        value = ~value;
        value++;
    }
    else
        UART_PutChar(' ');

    while (value >= (int8_t)100) { value -= (int8_t)100; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int8_t) 10) { value -= (int8_t) 10; d++; } UART_PutChar(d + (uint8_t)0x30); d = (uint8_t)0;
    while (value >= (int8_t)  1) { value -= (int8_t)  1; d++; } UART_PutChar(d + (uint8_t)0x30);
    UART_PutChar((uint8_t)UART_DELIMITER);
}

#define UART_OUTPUT_NIBBLE(value, shift)                            \
    nibble = (uint8_t) value >> shift;                              \
    if (nibble <= 9) { nibble += 0x30; } else { nibble += 0x37; }   \
    UART_PutChar(nibble);

void UART_SendChar_Hex(uint8_t value)
{
    uint8_t nibble;

    UART_OUTPUT_NIBBLE(value,         4);
    UART_OUTPUT_NIBBLE(value & 0x0F,  0);
}

void UART_SendNewLine(void)
{
    UART_PutChar((uint8_t)'\r');
    UART_PutChar((uint8_t)'\n');
}

void UART_PutChar(uint8_t value)
{
    while(PIR1bits.TXIF == 0) {}
    TXREG = value;
}


void UART_Receive_Service(void)
{
    static  uint8_t             buffer[2]           = {0,0};
    static  enum UART_STATES    state               = UART_STATE_idle;
    static  uint8_t             startAddress        = 0;
    static  uint8_t             first               = 0;
    static  uint8_t             count               = 0;
    static  uint8_t             readCount           = 0;
    static  uint8_t             i                   = 0;
            uint8_t             data                = RCREG;

    /* Overrun error detected */
    if (OERR)
    {
        CREN = 0;                                   /* Reset the UART module    */
        NOP();
        NOP();
        CREN = 1;

        UART_PutChar(UART_NACK);                    /* Send ASCII NACK to host  */
        state = UART_STATE_read;
        return;
    }

    switch(state)
    {
        case UART_STATE_idle:
            count = 0;                              /* So far, no data received */
            i     = 0;
            first = 1;                              /* First byte incoming      */
            if      (data == 'W') { state = UART_STATE_write; }
            else if (data == 'R') { state = UART_STATE_read;  }
            else                  { /* Error! */              }
            break;

        case UART_STATE_read:                        /* Receive read byte count*/
            if (data == '\n')                     /* End of a packet and start to send data to host*/
            {
                for (i = 0; i < readCount; i++)
                {
                    UART_SendChar_Hex(MEMORY_Read((uint8_t)(startAddress + i)));    /* Send data */
                }
                UART_SendNewLine();
                state = UART_STATE_idle;
            }
            else if (data != '\r')
            {
                buffer[count] = data;
                count++;
                if(count == 2)
                {
                    count = 0;
                    readCount = UART_HexToBinary(buffer);
                }
            }
            break;

        case UART_STATE_write:
           if      (data == '\n') { state = UART_STATE_idle; } /* End of packet */
           else if (data == 'R')  { state = UART_STATE_read; } /* Read register after setting the start address */
           else
           {
               buffer[count] = data;
               count++;
               if (count == 2)
               {
                    count = 0;

                    if (first == 0)
                    {
                        MEMORY_Write((uint8_t)(startAddress + i), UART_HexToBinary(buffer));
                        i++;
                    }
                    else
                    {
                        first = 0;
                        startAddress = UART_HexToBinary(buffer);
                    }
               }
           }
           break;
    }
}

uint8_t UART_HexToBinary(uint8_t* data)
{
    uint8_t output = 0;
    
    if ((data[0] & 0x40) == 1) { output  = data[0] - 0x37; } /* data[0] is A-F */
    else                       { output  = data[0] - 0x30; } /* data[0] is 0-9 */
    
    output <<= 4;
    
    if ((data[1] & 0x40) == 1) { output += data[1] - 0x37; } /* data[0] is A-F */
    else                       { output += data[1] - 0x30; } /* data[0] is 0-9 */
    
    return output;
}