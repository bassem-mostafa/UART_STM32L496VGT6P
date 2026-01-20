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

    #define UART_STM32L496VGT6P_BUFFER_SIZE_RECEIVE 64

// #############################################################################
// #### Private Type(s) ########################################################
// #############################################################################

/**
 * @brief UART STM32L496VGT6P Operation
 *
 * @enum UART_STM32L496VGT6P_OperationType_t
 */
typedef enum UART_STM32L496VGT6P_OperationType
{
    UART_STM32L496VGT6P_OperationType_None = 0, ///< None
    UART_STM32L496VGT6P_OperationType_Pending,  ///< Pending
    UART_STM32L496VGT6P_OperationType_Commit,   ///< Commit
    UART_STM32L496VGT6P_OperationType_Transmit, ///< Transmit
} UART_STM32L496VGT6P_OperationType_t;

/**
 * @brief UART STM32L496VGT6P Operation Handler
 */
typedef UART_STM32L496VGT6P_Status_t ( *UART_STM32L496VGT6P_OperationHandler_t )( UART_STM32L496VGT6P_Instance_t * Instance );

/**
 * @brief UART STM32L496VGT6P Operation Context
 *
 * @struct UART_STM32L496VGT6P_OperationContext_t
 */
typedef struct UART_STM32L496VGT6P_OperationContext
{
    UART_STM32L496VGT6P_Data_t * DataTx;
    UART_STM32L496VGT6P_DataLength_t DataTxLength;

    UART_STM32L496VGT6P_Data_t * DataRx;
    UART_STM32L496VGT6P_DataLength_t DataRxLength;
} UART_STM32L496VGT6P_OperationContext_t;

/**
 * @brief UART STM32L496VGT6P Operation
 *
 * @struct UART_STM32L496VGT6P_Operation_t
 */
typedef struct UART_STM32L496VGT6P_Operation
{
    UART_STM32L496VGT6P_OperationType_t Type;       ///< Type
    UART_STM32L496VGT6P_OperationHandler_t Handler; ///< Handler
    UART_STM32L496VGT6P_Status_t Status;            ///< Status
    TIM_Timestamp_t Timeout;                        ///< Timeout
    UART_STM32L496VGT6P_OperationContext_t Context; ///< Context
} UART_STM32L496VGT6P_Operation_t;

/**
 * @brief UART STM32L496VGT6P Process Type
 *
 * @enum UART_STM32L496VGT6P_ProcessType_t
 */
typedef enum UART_STM32L496VGT6P_ProcessType
{
    UART_STM32L496VGT6P_ProcessType_None = 0,   ///< None
    UART_STM32L496VGT6P_ProcessType_Initialize, ///< Initialize
    UART_STM32L496VGT6P_ProcessType_Transmit,   ///< Transmit
} UART_STM32L496VGT6P_ProcessType_t;

/**
 * @brief UART STM32L496VGT6P Process Handler
 */
typedef UART_STM32L496VGT6P_Status_t ( *UART_STM32L496VGT6P_ProcessHandler_t )( UART_STM32L496VGT6P_Instance_t * Instance );

/**
 * @brief UART STM32L496VGT6P Process Context
 *
 * @struct UART_STM32L496VGT6P_ProcessContext_t
 */
typedef struct UART_STM32L496VGT6P_ProcessContext
{
    UART_STM32L496VGT6P_Operation_t Operation; ///< Operation
} UART_STM32L496VGT6P_ProcessContext_t;

/**
 * @brief UART STM32L496VGT6P Process
 *
 * @struct UART_STM32L496VGT6P_Process_t
 */
typedef struct UART_STM32L496VGT6P_Process
{
    UART_STM32L496VGT6P_ProcessType_t Type;       ///< Type
    UART_STM32L496VGT6P_ProcessHandler_t Handler; ///< Handler
    UART_STM32L496VGT6P_ProcessContext_t Context; ///< Context
} UART_STM32L496VGT6P_Process_t;

typedef struct UART_STM32L496VGT6P_BufferReceive
{
    volatile uint32_t Length;
    uint8_t Content[ UART_STM32L496VGT6P_BUFFER_SIZE_RECEIVE ];
} UART_STM32L496VGT6P_BufferReceive_t;

typedef enum UART_STM32L496VGT6P_Event
{
    UART_STM32L496VGT6P_Event_None = 0,
    UART_STM32L496VGT6P_Event_Interrupt = UTIL_BIT( 0 ),
    UART_STM32L496VGT6P_Event_TxComplete = UTIL_BIT( 1 ),
    UART_STM32L496VGT6P_Event_RxComplete = UTIL_BIT( 2 ),
    UART_STM32L496VGT6P_Event_AbortTxComplete = UTIL_BIT( 3 ),
    UART_STM32L496VGT6P_Event_AbortRxComplete = UTIL_BIT( 4 ),
    UART_STM32L496VGT6P_Event_AbortComplete = UTIL_BIT( 5 ),
    UART_STM32L496VGT6P_Event_ErrorParity = UTIL_BIT( 6 ),
    UART_STM32L496VGT6P_Event_ErrorNoise = UTIL_BIT( 7 ),
    UART_STM32L496VGT6P_Event_ErrorFrame = UTIL_BIT( 8 ),
    UART_STM32L496VGT6P_Event_ErrorOverrun = UTIL_BIT( 9 ),
    UART_STM32L496VGT6P_Event_ErrorDMA = UTIL_BIT( 10 ),
    UART_STM32L496VGT6P_Event_ErrorReceiverTimeout = UTIL_BIT( 11 ),
    UART_STM32L496VGT6P_Event_RxEvent = UTIL_BIT( 12 ),
    UART_STM32L496VGT6P_Event_RxError = UTIL_BIT( 13 ),
} UART_STM32L496VGT6P_Event_t;

typedef struct UART_STM32L496VGT6P_Instance_Context
{
    UART_HandleTypeDef UARTx;

    UART_STM32L496VGT6P_BufferReceive_t Receive;

    UART_STM32L496VGT6P_Event_t Event;

    UART_STM32L496VGT6P_Process_t Process;
} UART_STM32L496VGT6P_Instance_Context_t;

typedef struct UART_STM32L496VGT6P_Context
{
    TIM_Timestamp_t Timestamp;
    UART_STM32L496VGT6P_Instance_Context_t Context[ UART_STM32L496VGT6P_Count ];
} UART_STM32L496VGT6P_Context_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################

void USART1_IRQHandler( void );
void USART2_IRQHandler( void );
void USART3_IRQHandler( void );
void UART4_IRQHandler( void );
void UART5_IRQHandler( void );
void LPUART1_IRQHandler( void );

void HAL_UART_TxCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_TxHalfCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_RxCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_RxHalfCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_ErrorCallback( UART_HandleTypeDef * huart );
void HAL_UART_AbortCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_AbortTransmitCpltCallback( UART_HandleTypeDef * huart );
void HAL_UART_AbortReceiveCpltCallback( UART_HandleTypeDef * huart );
void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef * huart, uint16_t Size );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_Initialize( void );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_Cycle( void );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Context_DeInitialize( void );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Initialize( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Cycle( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_DeInitialize( UART_STM32L496VGT6P_Instance_t * Instance );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_SetProcess( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_ProcessType_t ProcessType );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_ProcessInitialize( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_ProcessTransmit( UART_STM32L496VGT6P_Instance_t * Instance );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationCommitExecute( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationCommitResolve( UART_STM32L496VGT6P_Instance_t * Instance );

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationTransmitExecute( UART_STM32L496VGT6P_Instance_t * Instance );
static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationTransmitResolve( UART_STM32L496VGT6P_Instance_t * Instance );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static UART_STM32L496VGT6P_Context_t UART_STM32L496VGT6P_Context;

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

void USART1_IRQHandler( void )
{
    UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ];

    Context->Event |= UART_STM32L496VGT6P_Event_Interrupt;

    HAL_UART_IRQHandler( &Context->UARTx );
}

void USART2_IRQHandler( void )
{
    UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ];

    Context->Event |= UART_STM32L496VGT6P_Event_Interrupt;

    HAL_UART_IRQHandler( &Context->UARTx );
}

void USART3_IRQHandler( void )
{
    UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_3 ];

    Context->Event |= UART_STM32L496VGT6P_Event_Interrupt;

    HAL_UART_IRQHandler( &Context->UARTx );
}

void UART4_IRQHandler( void )
{
    UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_4 ];

    Context->Event |= UART_STM32L496VGT6P_Event_Interrupt;

    HAL_UART_IRQHandler( &Context->UARTx );
}

void UART5_IRQHandler( void )
{
    UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_5 ];

    Context->Event |= UART_STM32L496VGT6P_Event_Interrupt;

    HAL_UART_IRQHandler( &Context->UARTx );
}

void LPUART1_IRQHandler( void )
{
    UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_6 ];

    Context->Event |= UART_STM32L496VGT6P_Event_Interrupt;

    HAL_UART_IRQHandler( &Context->UARTx );
}

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
                Context->Event |= UART_STM32L496VGT6P_Event_RxError;
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
        __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ].UARTx, hdmatx, hdma_usart1_tx );
        __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_1 ].UARTx, hdmarx, hdma_usart1_rx );

        extern UART_HandleTypeDef huart2;
        extern DMA_HandleTypeDef hdma_usart2_tx;
        extern DMA_HandleTypeDef hdma_usart2_rx;
        extern void MX_USART2_UART_Init( void );
        MX_USART2_UART_Init( );
        UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx = huart2;
        __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx, hdmatx, hdma_usart2_tx );
        __HAL_LINKDMA( &UART_STM32L496VGT6P_Context.Context[ UART_STM32L496VGT6P_2 ].UARTx, hdmarx, hdma_usart2_rx );
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

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_GetTimestamp( UART_TIM, &UART_STM32L496VGT6P_Context.Timestamp ) ) != TIM_Status_Success )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

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

        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Instance_Initialize( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];

        // TODO GPIOs Configurations

        Context->Receive.Length = 0;
        Context->Receive.Content[ Context->Receive.Length ] = 0;

        Context->Event = UART_STM32L496VGT6P_Event_None;

        Instance->Context = Context;

        Status = UART_STM32L496VGT6P_SetProcess( Instance, UART_STM32L496VGT6P_ProcessType_Initialize );
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

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Operation->Handler != NULL )
        {
            UART_STM32L496VGT6P_Status_t STM32L496VGT6P_Status = UART_STM32L496VGT6P_Status_Error;
            if ( ( STM32L496VGT6P_Status = Operation->Handler( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
            {
                Status = STM32L496VGT6P_Status;
                // FIXME Operation reported non success status, is there any action ?
            }
        }

        if ( Process->Handler != NULL )
        {
            UART_STM32L496VGT6P_Status_t STM32L496VGT6P_Status = UART_STM32L496VGT6P_Status_Error;
            if ( ( STM32L496VGT6P_Status = Process->Handler( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
            {
                Status = STM32L496VGT6P_Status;
                // FIXME Process reported non success status, is there any action ?
            }
        }

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_Interrupt ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_Interrupt;
            UART_Trace( "Interrupt: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
            // TODO Invoke Callback
        }

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

        if ( ( Instance->Context->Event & UART_STM32L496VGT6P_Event_RxError ) != 0 )
        {
            Instance->Context->Event &= ~UART_STM32L496VGT6P_Event_RxError;
            UART_Debug( "RX Error: Instance=%p, UARTx=%d", Instance, Instance->UARTx );
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

        Status = UART_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_SetProcess( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_ProcessType_t ProcessType )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p, ProcessType=%d )", __FUNCTION__, Instance, ProcessType );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        switch ( ProcessType )
        {
            case UART_STM32L496VGT6P_ProcessType_None:
                Process->Handler = NULL;
                break;

            case UART_STM32L496VGT6P_ProcessType_Initialize:
                Process->Handler = UART_STM32L496VGT6P_ProcessInitialize;
                break;

            case UART_STM32L496VGT6P_ProcessType_Transmit:
                Process->Handler = UART_STM32L496VGT6P_ProcessTransmit;
                break;

            default:
                UART_Warning( "%s Not Handled Type %d", __FUNCTION__, ProcessType );
                Status = UART_STM32L496VGT6P_Status_NotSupported;
                break;
        }
        if ( Status != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        Process->Type = ProcessType;

        Operation->Handler = NULL;
        Operation->Status = UART_STM32L496VGT6P_Status_Success;
        Operation->Timeout = UART_STM32L496VGT6P_Context.Timestamp;

        switch ( ProcessType )
        {
            case UART_STM32L496VGT6P_ProcessType_None:
                Operation->Type = UART_STM32L496VGT6P_OperationType_None;
                break;

            default:
                Operation->Type = UART_STM32L496VGT6P_OperationType_Pending;
                break;
        }
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_ProcessInitialize( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != UART_STM32L496VGT6P_ProcessType_Initialize )
        {
            UART_Error( "%s Got %d Expected %d", __FUNCTION__, Process->Type, UART_STM32L496VGT6P_ProcessType_Initialize );
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

        if ( Operation->Handler != NULL )
        {
            // Operation In-progress
            break;
        }

        if ( Operation->Status != UART_STM32L496VGT6P_Status_Success )
        {
            Operation->Type = UART_STM32L496VGT6P_OperationType_None;
        }

        switch ( Operation->Type )
        {
            case UART_STM32L496VGT6P_OperationType_Pending:
                Operation->Status = UART_STM32L496VGT6P_OperationCommitExecute( Instance );
                break;

            case UART_STM32L496VGT6P_OperationType_Commit:
            default:
                if ( Instance->OnComplete != NULL )
                {
                    Instance->OnComplete( Instance, Operation->Status );
                }
                Status = UART_STM32L496VGT6P_SetProcess( Instance, UART_STM32L496VGT6P_ProcessType_None );
                break;
        }
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_ProcessTransmit( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != UART_STM32L496VGT6P_ProcessType_Transmit )
        {
            UART_Error( "%s Got %d Expected %d", __FUNCTION__, Process->Type, UART_STM32L496VGT6P_ProcessType_Transmit );
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

        if ( Operation->Handler != NULL )
        {
            // Operation In-progress
            break;
        }

        if ( Operation->Status != UART_STM32L496VGT6P_Status_Success )
        {
            Operation->Type = UART_STM32L496VGT6P_OperationType_None;
        }

        switch ( Operation->Type )
        {
            case UART_STM32L496VGT6P_OperationType_Pending:
                Operation->Status = UART_STM32L496VGT6P_OperationTransmitExecute( Instance );
                break;

            case UART_STM32L496VGT6P_OperationType_Transmit:
            default:
                if ( Instance->OnComplete != NULL )
                {
                    Instance->OnComplete( Instance, Operation->Status );
                }
                Status = UART_STM32L496VGT6P_SetProcess( Instance, UART_STM32L496VGT6P_ProcessType_None );
                break;
        }
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationCommitExecute( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

    // FIXME Keep CubeMX generated configurations as is for now
    #if 0
      HAL_StatusTypeDef HAL_Status = HAL_ERROR;
      if ( ( HAL_Status = HAL_UART_Init( &Instance->Context->UARTx ) ) != HAL_OK )
      {
        Status = UART_STM32L496VGT6P_Status_Error;
        break;
      }
    #endif

        Operation->Type = UART_STM32L496VGT6P_OperationType_Commit;
        Operation->Handler = UART_STM32L496VGT6P_OperationCommitResolve;
        Operation->Status = UART_STM32L496VGT6P_Status_Success;
        Operation->Timeout = UART_STM32L496VGT6P_Context.Timestamp;

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_Timestamp_AddMillisecond( &Operation->Timeout, 0 ) ) != TIM_Status_Success )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationCommitResolve( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Operation->Type != UART_STM32L496VGT6P_OperationType_Commit )
        {
            UART_Error( "%s Got %d Expected %d", __FUNCTION__, Operation->Type, UART_STM32L496VGT6P_OperationType_Commit );
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_IsExpiredTimestamp( UART_TIM, &Operation->Timeout ) ) == TIM_Status_Success )
        {
            Operation->Status = UART_STM32L496VGT6P_Status_Success;
            Operation->Handler = NULL;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationTransmitExecute( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        HAL_StatusTypeDef HAL_Status = HAL_ERROR;
        if ( ( HAL_Status = HAL_UART_Transmit_DMA( &Instance->Context->UARTx, Operation->Context.DataTx, Operation->Context.DataTxLength ) ) != HAL_OK )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

        Operation->Type = UART_STM32L496VGT6P_OperationType_Transmit;
        Operation->Handler = UART_STM32L496VGT6P_OperationTransmitResolve;
        Operation->Status = UART_STM32L496VGT6P_Status_Success;
        Operation->Timeout = UART_STM32L496VGT6P_Context.Timestamp;

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_Timestamp_AddMillisecond( &Operation->Timeout, 1000 ) ) != TIM_Status_Success )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_OperationTransmitResolve( UART_STM32L496VGT6P_Instance_t * Instance )
{
    UART_STM32L496VGT6P_Status_t Status = UART_STM32L496VGT6P_Status_Success;

    do
    {
        UART_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Operation->Type != UART_STM32L496VGT6P_OperationType_Transmit )
        {
            UART_Error( "%s Got %d Expected %d", __FUNCTION__, Operation->Type, UART_STM32L496VGT6P_OperationType_Transmit );
            Status = UART_STM32L496VGT6P_Status_Error;
            break;
        }

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_IsExpiredTimestamp( UART_TIM, &Operation->Timeout ) ) == TIM_Status_Success )
        {
            Operation->Status = UART_STM32L496VGT6P_Status_Timeout;
            Operation->Handler = NULL;
            break;
        }

        if ( ( Context->Event & UART_STM32L496VGT6P_Event_TxComplete ) != 0 )
        {
            Operation->Status = UART_STM32L496VGT6P_Status_Success;
            Operation->Handler = NULL;
        }
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

        if ( Instance == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        if ( ( Status = UART_STM32L496VGT6P_Context_Initialize( ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        if ( ( Status = UART_STM32L496VGT6P_Instance_Initialize( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
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

        if ( ( Status = UART_STM32L496VGT6P_Context_Cycle( ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        if ( ( Status = UART_STM32L496VGT6P_Instance_Cycle( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
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

        if ( ( Status = UART_STM32L496VGT6P_Instance_DeInitialize( Instance ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        if ( ( Status = UART_STM32L496VGT6P_Context_DeInitialize( ) ) != UART_STM32L496VGT6P_Status_Success )
        {
            break;
        }
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

        UART_STM32L496VGT6P_Instance_Context_t * Context = &UART_STM32L496VGT6P_Context.Context[ Instance->UARTx ];
        UART_STM32L496VGT6P_Process_t * Process = &Context->Process;
        UART_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != UART_STM32L496VGT6P_ProcessType_None
             || Operation->Type != UART_STM32L496VGT6P_OperationType_None )
        {
            Status = UART_STM32L496VGT6P_Status_Busy;
            break;
        }

        Status = UART_STM32L496VGT6P_SetProcess( Instance, UART_STM32L496VGT6P_ProcessType_Transmit );

        Operation->Context.DataTx = Data;
        Operation->Context.DataTxLength = DataLength;
    }
    while ( 0 );

    return Status;
}

UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Read( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength )
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

        if ( Instance->Context == NULL )
        {
            Status = UART_STM32L496VGT6P_Status_Error;
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

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

const char UART_STM32L496VGT6P_VERSION[] = "0.0.0.v20260120-0211";

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* STM32L496xx */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
