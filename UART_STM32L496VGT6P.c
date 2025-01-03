// #############################################################################
// #### Copyright ##############################################################
// #############################################################################

/*
 * Copyright 2024 BaSSeM
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// #############################################################################
// #### Description ############################################################
// #############################################################################

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

#include "Platform.h"

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

#ifndef DEBUG
    #define DEBUG
#endif

#ifdef DEBUG
    #undef DEBUG
#endif

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#ifdef STM32L496xx

// #############################################################################
// #### Include(s) #############################################################
// #############################################################################

    #include "../../UART_Internal.h"
    #include "UART_STM32L496VGT6P.h"

    #include "stm32l4xx.h"
    #include "stm32l4xx_hal_uart.h"

// #############################################################################
// #### Private Macro(s) #######################################################
// #############################################################################

    #define UART_STM32L496VGT6P_TIMEOUT_BYTE_TX     ( 1 ) // in milliseconds
    #define UART_STM32L496VGT6P_BUFFER_SIZE_RECEIVE 128

// #############################################################################
// #### Private Type(s) ########################################################
// #############################################################################

typedef enum UART_STM32L496VGT6P_Event
{
    UART_STM32L496VGT6P_Event_None = 0,
    UART_STM32L496VGT6P_Event_TxComplete = UTIL_BIT( 0 ),
    UART_STM32L496VGT6P_Event_RxComplete = UTIL_BIT( 1 ),
    UART_STM32L496VGT6P_Event_AbortTxComplete = UTIL_BIT( 2 ),
    UART_STM32L496VGT6P_Event_AbortRxComplete = UTIL_BIT( 3 ),
    UART_STM32L496VGT6P_Event_AbortComplete = UTIL_BIT( 4 ),
    UART_STM32L496VGT6P_Event_ErrorParity = UTIL_BIT( 5 ),
    UART_STM32L496VGT6P_Event_ErrorNoise = UTIL_BIT( 6 ),
    UART_STM32L496VGT6P_Event_ErrorFrame = UTIL_BIT( 7 ),
    UART_STM32L496VGT6P_Event_ErrorOverrun = UTIL_BIT( 8 ),
    UART_STM32L496VGT6P_Event_ErrorDMA = UTIL_BIT( 9 ),
    UART_STM32L496VGT6P_Event_ErrorReceiverTimeout = UTIL_BIT( 10 ),
    UART_STM32L496VGT6P_Event_RxEvent = UTIL_BIT( 11 ),
} UART_STM32L496VGT6P_Event_t;

typedef struct UART_STM32L496VGT6P_Buffer_Receive
{
    volatile uint32_t Length;
    uint8_t Content[ UART_STM32L496VGT6P_BUFFER_SIZE_RECEIVE ];
} UART_STM32L496VGT6P_Buffer_Receive_t;

typedef struct UART_STM32L496VGT6P_Instance_Context
{
    UART_HandleTypeDef UARTx;
    UART_STM32L496VGT6P_Buffer_Receive_t Receive;
    UART_STM32L496VGT6P_Event_t Event; // TODO Does it need a context to be associated ? in addition to a queue ?
} UART_STM32L496VGT6P_Instance_Context_t;

typedef struct UART_STM32L496VGT6P_Context
{
    TIM_Timestamp_t Timestamp;
    UART_STM32L496VGT6P_Instance_Context_t Context[ UART_STM32L496VGT6P_Count ];
} UART_STM32L496VGT6P_Context_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################

void HAL_UART_TxCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_TxHalfCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_RxCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_RxHalfCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_ErrorCallback( UART_HandleTypeDef * huart );
void HAL_UART_AbortCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_AbortTransmitCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_AbortReceiveCpltCallback( UART_HandleTypeDef * huart );
void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef * huart, uint16_t Size );

void USART1_IRQHandler( void );
void USART2_IRQHandler( void );
void USART3_IRQHandler( void );
void UART4_IRQHandler( void );
void UART5_IRQHandler( void );
void LPUART1_IRQHandler( void );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Write( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Read( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Commit( UART_STM32L496VGT6P_Instance_t * Instance );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Initialize( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Cycle( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_DeInitialize( UART_STM32L496VGT6P_Instance_t * Instance );
//
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_Initialize( void );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_Cycle( void );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_DeInitialize( void );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static UART_STM32L496VGT6P_Context_t UART_STM32L496VGT6P_Context;

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

void HAL_UART_TxCpltCallback( UART_HandleTypeDef * huart )
{
    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            Context->Event |= UART_STM32L496VGT6P_Event_TxComplete;
            break;
        }
    }
}

void HAL_UART_TxHalfCpltCallback( UART_HandleTypeDef * huart )
{
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef * huart )
{
    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            Context->Event |= UART_STM32L496VGT6P_Event_RxComplete;

            Context->Receive.Length += huart->RxXferSize;
            if ( Context->Receive.Length >= UTIL_SizeOf( Context->Receive.Content ) )
            {
                UART_Warning( "%s: %s", __FUNCTION__, "MAX Length Reached!" );
                Context->Receive.Length = 0;
            }
            if ( HAL_UART_Receive_IT( &Context->UARTx, Context->Receive.Content + Context->Receive.Length, 1 ) != HAL_OK )
            {
                // FIXME
            }
        }
    }
}

void HAL_UART_RxHalfCpltCallback( UART_HandleTypeDef * huart )
{
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef * huart )
{
    uint32_t error = HAL_UART_GetError( huart );

    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            if ( ( error & HAL_UART_ERROR_PE ) == HAL_UART_ERROR_PE )
            {
                Context->Event |= UART_STM32L496VGT6P_Event_ErrorParity;
            }
            if ( ( error & HAL_UART_ERROR_NE ) == HAL_UART_ERROR_NE )
            {
                Context->Event |= UART_STM32L496VGT6P_Event_ErrorNoise;
            }
            if ( ( error & HAL_UART_ERROR_FE ) == HAL_UART_ERROR_FE )
            {
                Context->Event |= UART_STM32L496VGT6P_Event_ErrorFrame;
            }
            if ( ( error & HAL_UART_ERROR_ORE ) == HAL_UART_ERROR_ORE )
            {
                Context->Event |= UART_STM32L496VGT6P_Event_ErrorOverrun;
            }
            if ( ( error & HAL_UART_ERROR_DMA ) == HAL_UART_ERROR_DMA )
            {
                Context->Event |= UART_STM32L496VGT6P_Event_ErrorDMA;
            }
            if ( ( error & HAL_UART_ERROR_RTO ) == HAL_UART_ERROR_RTO )
            {
                Context->Event |= UART_STM32L496VGT6P_Event_ErrorReceiverTimeout;
            }
            break;
        }
    }
}

void HAL_UART_AbortCpltCallback( UART_HandleTypeDef * huart )
{
    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            Context->Event |= UART_STM32L496VGT6P_Event_AbortComplete;
            break;
        }
    }
}

void HAL_UART_AbortTransmitCpltCallback( UART_HandleTypeDef * huart )
{
    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            Context->Event |= UART_STM32L496VGT6P_Event_AbortTxComplete;
            break;
        }
    }
}

void HAL_UART_AbortReceiveCpltCallback( UART_HandleTypeDef * huart )
{
    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            Context->Event |= UART_STM32L496VGT6P_Event_AbortRxComplete;
            break;
        }
    }
}

void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef * huart, uint16_t Size )
{
    for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
    {
        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

        if ( &Context->UARTx == huart )
        {
            Context->Event |= UART_STM32L496VGT6P_Event_RxEvent;
            break;
        }
    }
}

void USART1_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ].UARTx );
}

void USART2_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx );
}

void USART3_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_3 ].UARTx );
}

void UART4_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_4 ].UARTx );
}

void UART5_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_5 ].UARTx );
}

void LPUART1_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_6 ].UARTx );
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Write( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p, Data=%p, Length=%d )", __FUNCTION__, Instance, Data, DataLength );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        HAL_StatusTypeDef HAL_Status = HAL_ERROR;
        // TODO Make Use Of DMA
    #if 0
    while ( ( HAL_Status = HAL_UART_Transmit_DMA( &Instance->Context->UARTx, Data, DataLength ) ) != HAL_OK )
    {
//      Status = UART_STM32L496VGT6P_Status_Error;
//      break;
    }
    #else
        if ( ( HAL_Status = HAL_UART_Transmit( &Instance->Context->UARTx, Data, DataLength, UART_STM32L496VGT6P_TIMEOUT_BYTE_TX * DataLength ) ) != HAL_OK )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }
    #endif

        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Read( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p, Data=%p, Length=%d )", __FUNCTION__, Instance, Data, DataLength );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        if ( Instance->Context->Receive.Length < DataLength )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }
        UTIL_MemoryCopy( Data, Instance->Context->Receive.Content, DataLength );
        KERNEL_InterruptDisable( );
        UTIL_MemoryCopy( Instance->Context->Receive.Content, Instance->Context->Receive.Content + DataLength, Instance->Context->Receive.Length - DataLength );
        Instance->Context->Receive.Length -= DataLength;
        Instance->Context->Receive.Content[ Instance->Context->Receive.Length ] = 0;
        Instance->Context->UARTx.pRxBuffPtr -= DataLength;
        KERNEL_InterruptEnable( );
        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Commit( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        if ( Instance->Context == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

        Status = UART_STM32L496VGT6P_Status_Success;

        // FIXME Keep CubeMX generated configurations as is for now
    #if 0
    HAL_StatusTypeDef HAL_Status = HAL_ERROR;
    if ( ( HAL_Status = HAL_UART_Init( &Instance->Context->UARTx ) ) != HAL_OK )
    {
      Status = UART_STM32L496VGT6P_Status_Error;
      break;
    }
    #endif
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Initialize( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        // TODO GPIO Initialization

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];

        Status = UART_STM32L496VGT6P_Status_Success;

        // FIXME Keep CubeMX generated configurations as is for now
        switch ( Instance->UARTx )
        {
            case UART_STM32L496VGT6P_1:
                extern UART_HandleTypeDef huart1;
                Context->UARTx = huart1;
                break;

            case UART_STM32L496VGT6P_2:
                extern UART_HandleTypeDef huart2;
                Context->UARTx = huart2;
                break;

            default:
                Status = UART_STM32L496VGT6P_Status_NotSupported;
                break;
        }
        if ( Status != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        Instance->Context = Context;
        Instance->Context->Receive.Length = 0;
        UTIL_MemorySetZero( Instance->Context->Receive.Content, UTIL_SizeOf( Instance->Context->Receive.Content ) );

        if ( ( Status = UART_STM32L496VGT6P_Instance_Commit( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Cycle( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        Status = UART_STM32L496VGT6P_Status_Success;

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_TxComplete ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_TxComplete;
            UART_Debug( "TX Complete: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_RxComplete ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_RxComplete;
            UART_Debug( "RX Complete: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_AbortTxComplete ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_AbortTxComplete;
            UART_Debug( "Abort TX Complete: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_AbortRxComplete ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_AbortRxComplete;
            UART_Debug( "Abort RX Complete: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_AbortComplete ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_AbortComplete;
            UART_Debug( "Abort Complete: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_ErrorParity ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_ErrorParity;
            UART_Debug( "Parity Error: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_ErrorNoise ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_ErrorNoise;
            UART_Debug( "Noise Error: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_ErrorFrame ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_ErrorFrame;
            UART_Debug( "Frame Error: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_ErrorOverrun ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_ErrorOverrun;
            UART_Debug( "Overrun Error: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_ErrorDMA ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_ErrorDMA;
            UART_Debug( "DMA Error: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_ErrorReceiverTimeout ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_ErrorReceiverTimeout;
            UART_Debug( "Receiver Timeout: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_RxEvent ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_RxEvent;
            UART_Debug( "RX Event: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_DeInitialize( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_Initialize( void )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( void )", __FUNCTION__ );

        // FIXME Remove the usage of `MX_USARTx_UART_Init()`
    #if 1
        extern UART_HandleTypeDef huart1;
        extern DMA_HandleTypeDef hdma_usart1_tx;
        extern DMA_HandleTypeDef hdma_usart1_rx;
        extern void MX_USART1_UART_Init( void );
        MX_USART1_UART_Init( );
        UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ].UARTx = huart1;
        // TODO Make Use Of DMA
        #if 0
    __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ].UARTx, hdmatx, hdma_usart1_tx );
    __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ].UARTx, hdmarx, hdma_usart1_rx );
        #endif

        extern UART_HandleTypeDef huart2;
        extern DMA_HandleTypeDef hdma_usart2_tx;
        extern DMA_HandleTypeDef hdma_usart2_rx;
        extern void MX_USART2_UART_Init( void );
        MX_USART2_UART_Init( );
        UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx = huart2;
        // TODO Make Use Of DMA
        #if 0
    __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx, hdmatx, hdma_usart2_tx );
    __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx, hdmarx, hdma_usart2_rx );
        #endif
    #endif

        for ( UART_STM32L496VGT6P_t UART_x = UART_STM32L496VGT6P_1; UART_x < UART_STM32L496VGT6P_Count; ++UART_x )
        {
            UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_x ];

            HAL_StatusTypeDef HAL_Status = HAL_ERROR;
            if ( ( HAL_Status = HAL_UART_Receive_IT( &Context->UARTx, ( uint8_t * ) Context->Receive.Content, 1 ) ) != HAL_OK )
            {
                Status = UART_STM32L496VGT6P_Status_Error;
                break;
            }
        }

        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_Cycle( void )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( void )", __FUNCTION__ );
        UTIL_UNUSED( UART_STM32L496VGT6P_Context ); // FIXME Skip Warning
        // Nothing to be done
        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_DeInitialize( void )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( void )", __FUNCTION__ );
        UTIL_UNUSED( UART_STM32L496VGT6P_Context ); // FIXME Skip Warning
        // Nothing to be done
        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

// #############################################################################
// #### Public Method(s) #######################################################
// #############################################################################

UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Initialize( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        if ( ( Status = UART_STM32L496VGT6P_Context_Initialize( ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
        Status = UART_STM32L496VGT6P_Instance_Initialize( Instance );
    }
    while ( 0 );
    return Status;
}

UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Cycle( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        if ( ( Status = UART_STM32L496VGT6P_Context_Cycle( ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
        Status = UART_STM32L496VGT6P_Instance_Cycle( Instance );
    }
    while ( 0 );
    return Status;
}

UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_DeInitialize( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        if ( ( Status = UART_STM32L496VGT6P_Instance_DeInitialize( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
        Status = UART_STM32L496VGT6P_Context_DeInitialize( );
    }
    while ( 0 );
    return Status;
}

UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Write( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        Status = UART_STM32L496VGT6P_Instance_Write( Instance, Data, DataLength );
    }
    while ( 0 );
    return Status;
}

UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Read( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Error;
    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        Status = UART_STM32L496VGT6P_Instance_Read( Instance, Data, DataLength );
    }
    while ( 0 );
    return Status;
}

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

const char UART_STM32L496VGT6P_VERSION[] = "0.0.0.v20260117-1502";

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* STM32L496xx */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
