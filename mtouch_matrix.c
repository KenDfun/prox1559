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
#include "mtouch_matrix.h"

#if defined(MTOUCH_MATRIX_ENABLED)
extern void MTOUCH_CALLBACK_Matrix_NodePressed  (uint8_t node);
extern void MTOUCH_CALLBACK_Matrix_NodeReleased (uint8_t node);

static MTOUCH_MATRIX_MASK_t MTOUCH_matrix_state;

static MTOUCH_MATRIX_MASK_t MTOUCH_Matrix_StateMachine         (void);
static MTOUCH_MATRIX_MASK_t MTOUCH_Matrix_SingleRowDecode      (void);

/* Validate correct configuration for this firmware */
#if     (MTOUCH_MATRIX_NODES != 16)
    #error("Matrix firmware developed for 16 nodes only. Source code modification required.")
#elif   !defined(MTOUCH_MATRIX_ROW1) || \
        !defined(MTOUCH_MATRIX_ROW2) || \
        !defined(MTOUCH_MATRIX_ROW3) || \
        !defined(MTOUCH_MATRIX_ROW4) || \
        !defined(MTOUCH_MATRIX_COL1) || \
        !defined(MTOUCH_MATRIX_COL2) || \
        !defined(MTOUCH_MATRIX_COL3)
    #error("Matrix firmware requires definition of ROW1-4 and COL1-3.")
#elif    defined(MTOUCH_MATRIX_ROW5) || \
         defined(MTOUCH_MATRIX_COL4)
    #warning("Matrix firmware does not use ROW5 or COL4 sensors.")
#endif

void MTOUCH_Matrix_Init(void)
{
    MTOUCH_matrix_state = (MTOUCH_MATRIX_MASK_t)0;
}

void MTOUCH_Matrix_Decode(void)
{
    MTOUCH_MATRIX_MASK_t newState;
    MTOUCH_MATRIX_MASK_t change;
    uint8_t i;

    if  (
            (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) == MTOUCH_STATE_initializing)
         || (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) == MTOUCH_STATE_initializing)
         || (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) == MTOUCH_STATE_initializing)
         || (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) == MTOUCH_STATE_initializing)
         || (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) == MTOUCH_STATE_initializing)
         || (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) == MTOUCH_STATE_initializing)
         || (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) == MTOUCH_STATE_initializing)
        )
    {
        /* Do not perform matrix decoding if any of the sensors are currently being initialized. */
    }
    else
    {
        newState = MTOUCH_Matrix_StateMachine();
        change = newState ^ MTOUCH_matrix_state;

        /* If something has changed with the matrix state */
        if (change != (MTOUCH_MATRIX_MASK_t)0)     
        {
            change &= newState;

            /* new & (new ^ old) = presses */
            for (i = (uint8_t)0; i < (uint8_t)MTOUCH_MATRIX_NODES; i++)
            {
                if ((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << i) & change)
                {
                    /*lint -e522*/
                    MTOUCH_CALLBACK_Matrix_NodePressed(i);     /* Do not care if this has no side effects. */
                    /*lint +e522*/
                }
            }

            change  = newState ^ MTOUCH_matrix_state;
            change &= MTOUCH_matrix_state;

            /* old & (new ^ old) = releases */
            for (i = (uint8_t)0; i < (uint8_t)MTOUCH_MATRIX_NODES; i++)
            {
                if ((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << i) & change)
                {
                    /*lint -e522*/
                    MTOUCH_CALLBACK_Matrix_NodeReleased(i);     /* Do not care if this has no side effects. */
                    /*lint +e522*/
                }
            }
        }
        else
        {
            /* No state change. */
        }
        
        MTOUCH_matrix_state = newState;
    }
}

MTOUCH_MATRIX_MASK_t MTOUCH_Matrix_GetState(void)
{
    return MTOUCH_matrix_state;
}

static MTOUCH_MATRIX_MASK_t MTOUCH_Matrix_StateMachine(void)
{
    MTOUCH_MATRIX_MASK_t state = (MTOUCH_MATRIX_MASK_t)0;

    /**************************************************************************
     *  NO ROW/COLUMN TEST
     *
     *      If there are either no rows or no columns pressed in the matrix,
     *      then the state of the matrix is immediately known to be empty.
     *
     **************************************************************************/
    if  (
            (   /* If no rows are pressed... */
                (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) != MTOUCH_STATE_pressed)
             && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) != MTOUCH_STATE_pressed)
             && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) != MTOUCH_STATE_pressed)
             && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) != MTOUCH_STATE_pressed)
            )
         ||
            (   /* Or no columns are pressed... */
                (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) != MTOUCH_STATE_pressed)
             && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) != MTOUCH_STATE_pressed)
             && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) != MTOUCH_STATE_pressed)
            )
        )
    {
        /* No rows or no columns are pressed, so no nodes are activated. */
    }

    /**************************************************************************
     *  SINGLE COLUMN AND SINGLE ROW TEST
     *
     *      If only one row and only one column are being pressed, activate
     *      the node.
     *
     **************************************************************************/
    else if 
        (
            (
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) == MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) != MTOUCH_STATE_pressed)
                )
             ||
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) == MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) != MTOUCH_STATE_pressed)
                )
             ||
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) == MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) != MTOUCH_STATE_pressed)
                )
             ||
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) == MTOUCH_STATE_pressed)
                )
            )
         &&
            (
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) == MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) != MTOUCH_STATE_pressed)
                )
             ||
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) == MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) != MTOUCH_STATE_pressed)
                )
             ||
                (
                    (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) != MTOUCH_STATE_pressed)
                 && (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) == MTOUCH_STATE_pressed)
                )
            )
        )
    {
        state = MTOUCH_Matrix_SingleRowDecode();
    }
    else
    {
        /**************************************************************************
         *  MULTIPLE ROWS AND COLUMNS
         *
         *      If multiple rows and columns are pressed, reinitialize the entire
         *      matrix and disable any active presses.
         *
         **************************************************************************/
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_ROW1);
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_ROW2);
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_ROW3);
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_ROW4);
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_COL1);
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_COL2);
        MTOUCH_Button_State_Reset((uint8_t)MTOUCH_MATRIX_COL3);
    }
    
    return state;
}


/*
 * THIS LOGIC TREE WAS IMPLEMENTED INTENTIONALLY TO SAVE RAM AT THE EXPENSE
 * OF PROGRAM MEMORY.
 */
 
/*lint -e835*/  
/* Zero's as left-shift argument are intentional. They provide better readibility and are optimized away by the compiler. */
static MTOUCH_MATRIX_MASK_t MTOUCH_Matrix_SingleRowDecode(void)
{
    /* ASSUME: We know that only one row is pressed. */
    MTOUCH_MATRIX_MASK_t state = (MTOUCH_MATRIX_MASK_t)0;

    if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW1) == MTOUCH_STATE_pressed)
    {
        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)0) << (uint8_t)0);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)1) << (uint8_t)0);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)2) << (uint8_t)0);
        }
    }
    else if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW2) == MTOUCH_STATE_pressed)
    {
        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)0) << (uint8_t)3);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)1) << (uint8_t)3);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)2) << (uint8_t)3);
        }
    }
    else if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW3) == MTOUCH_STATE_pressed)
    {
        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)0) << (uint8_t)6);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)1) << (uint8_t)6);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)2) << (uint8_t)6);
        }
    }
    else if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_ROW4) == MTOUCH_STATE_pressed)
    {
        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL1) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)0) << (uint8_t)9);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL2) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)1) << (uint8_t)9);
        }

        if (MTOUCH_Button_State_Get((uint8_t)MTOUCH_MATRIX_COL3) == MTOUCH_STATE_pressed)
        {
            state |= (MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)((MTOUCH_MATRIX_MASK_t)1 << (uint8_t)2) << (uint8_t)9);
        }
    }
    else
    {
        state = (MTOUCH_MATRIX_MASK_t)0;
    }
    return state;
}

#endif
