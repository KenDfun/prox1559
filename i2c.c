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

#include "i2c.h"
#include "memory.h"

#define I2C_CMD_WRITE   ((uint8_t)0)
#define I2C_BYTE_ADDR   ((uint8_t)0)

void I2C_Init(void)
{
    SSPSTAT = (uint8_t)0b11000000;  /* Slew rate control disabled. 100kHz and 1MHz.
                                     * SMBus-compliant voltage thresholds enabled. */
    SSPCON1 = (uint8_t)0b00000110;  /* Clear write collision and overflow bits.
                                     * SSP module disabled until init is complete.
                                     * I2C slave mode w/o START or STOP interrupts. */
    SSPCON2 = (uint8_t)0b00000001;  /* General call address disabled.
                                     * Clock stretching enabled. */
    SSPCON3 = (uint8_t)0b00010000;  /* Start/Stop interrupts disabled.
                                     * Buffer overwrite is enabled. NACK sent if this occurs.
                                     * Bus collision interrupt disabled.
                                     * Address and Data byte clock stretching disabled. */
    SSPMSK = (uint8_t)0xFF;         /* All address bits required to match. */
    SSPADD = (uint8_t)0b01010000;   /* Address is the same as the CAP1114. */

    SSPCON1bits.SSPEN = (uint8_t)1; /* Enable I2C communications. */
}

void I2C_Service(void)
{
    static  uint8_t             count               = (uint8_t)0;
    static  uint8_t             startAddress        = (uint8_t)0;

    uint8_t data = SSPBUF;

    if (SSPCON1bits.SSPOV != (uint8_t)0)
    {
        /*
         * An overflow has occurred. The only safe recovery is to reset the
         * MSSP module and wait for the start of a new packet.
         */
        SSPCON1bits.SSPEN = (uint8_t)0;
        SSPCON1bits.SSPEN = (uint8_t)1;
    }
    else
    {
        if (SSPSTATbits.R_nW == I2C_CMD_WRITE)
        {
            if (SSPSTATbits.D_nA == I2C_BYTE_ADDR)
            {
                /* Address byte received with 'write' bit set */
                count = (uint8_t)0xFF;
            }
            else
            {
                /* Data byte received while in 'write' mode */
                if (count == (uint8_t)0xFF)
                {
                    /* This is the first byte received after a 'write' device
                     * address match. Store the byte as the start address. */
                    startAddress  = data;
                    count = (uint8_t)0;
                }
                else
                {
                    MEMORY_Write((uint8_t)(startAddress + count), data);
                    count++;
                    if (count == (uint8_t)0xFF)
                    {
                        /* The host should never be writing this many
                         * consecutive bytes. Reset the MSSP.
                         */
                        SSPCON1bits.SSPEN = (uint8_t)0;
                        SSPCON1bits.SSPEN = (uint8_t)1;
                    }
                }
            }
        }
        else /* READ */
        {
            if (SSPSTATbits.D_nA == I2C_BYTE_ADDR)
            {
                /* Address byte received with 'read' bit set. Reset the count
                 * variable to start reading at the currently set startAddress. */
                count = (uint8_t)0;
            }

            SSPBUF = MEMORY_Read(startAddress + count);
            count++;
            if (count == (uint8_t)0xFF)
            {
                /* 
                 * The host should never be reading this many
                 * consecutive bytes. Reset the MSSP.
                 */
                SSPCON1bits.SSPEN = (uint8_t)0;
                SSPCON1bits.SSPEN = (uint8_t)1;
            }
        }
    }
    SSPCON1bits.CKP = (uint8_t)1; /* Release clock stretch */
}
