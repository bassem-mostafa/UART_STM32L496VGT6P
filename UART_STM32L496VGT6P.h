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

/**
 *  @file
 *
 *  @brief Platform UART STM32L496VGT6P Driver
 */

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

/**
 *  @addtogroup Platform_UART_Driver
 *
 *  @{
 */

/**
 *  @defgroup Platform_UART_STM32L496VGT6P STM32L496VGT6P
 *
 *  @{
 */

#ifndef UART_STM32L496VGT6P_H_
    #define UART_STM32L496VGT6P_H_

    #ifdef __cplusplus
extern "C"
{
    #endif /* __cplusplus */

    // #############################################################################
    // #### Include(s) #############################################################
    // #############################################################################

    // #############################################################################
    // #### Public Macro(s) ########################################################
    // #############################################################################

    // #############################################################################
    // #### Public Type(s) #########################################################
    // #############################################################################

    /**
     *  @brief UART STM32L496VGT6P Operation Status Type
     *
     *  @enum UART_STM32L496VGT6P_Status_t
     */
    typedef enum UART_STM32L496VGT6P_Status
    {
        UART_STM32L496VGT6P_Status_Success = 0,     ///< Success
        UART_STM32L496VGT6P_Status_ArgumentInvalid, ///< Argument Invalid
        UART_STM32L496VGT6P_Status_NotSupported,    ///< Not Supported
        UART_STM32L496VGT6P_Status_Error,           ///< General Error
        UART_STM32L496VGT6P_Status_Busy,            ///< Busy
        UART_STM32L496VGT6P_Status_Timeout,         ///< Timeout
    } UART_STM32L496VGT6P_Status_t;

    /**
     *  @brief UART STM32L496VGT6P Type
     *
     *  @enum UART_STM32L496VGT6P_t
     */
    typedef enum UART_STM32L496VGT6P
    {
        UART_STM32L496VGT6P_1 = 0, ///< UART 1 (Minimum)
        UART_STM32L496VGT6P_2,     ///<
        UART_STM32L496VGT6P_3,     ///<
        UART_STM32L496VGT6P_4,     ///<
        UART_STM32L496VGT6P_5,     ///<
        UART_STM32L496VGT6P_6,     ///< UART 6 (Maximum)
        UART_STM32L496VGT6P_Count, ///< Count
    } UART_STM32L496VGT6P_t;

    /**
     *  @brief UART STM32L496VGT6P Instance Context Type
     *
     *  @struct UART_STM32L496VGT6P_Instance_Context_t
     */
    typedef struct UART_STM32L496VGT6P_Instance_Context UART_STM32L496VGT6P_Instance_Context_t;

    /**
     *  @brief UART STM32L496VGT6P Instance Type
     *
     *  @struct UART_STM32L496VGT6P_Instance_t
     */
    typedef struct UART_STM32L496VGT6P_Instance
    {
        UART_STM32L496VGT6P_t UARTx;
        GPIO_t TX;
        GPIO_t RX;
        UART_STM32L496VGT6P_Instance_Context_t * Context;
    } UART_STM32L496VGT6P_Instance_t;

    /**
     *  @brief UART STM32L496VGT6P Data Type
     */
    typedef uint8_t UART_STM32L496VGT6P_Data_t;

    /**
     *  @brief UART STM32L496VGT6P Data Length Type
     */
    typedef uint32_t UART_STM32L496VGT6P_DataLength_t;

    // #############################################################################
    // #### Public Method(s) #######################################################
    // #############################################################################

    /**
     *  @brief Initializes specified UART STM32L496VGT6P Instance
     *
     *  @param[in] Instance Instance
     *
     *  @return UART_STM32L496VGT6P_Status_t
     */
    UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Initialize( UART_STM32L496VGT6P_Instance_t * Instance );

    /**
     *  @brief Cycles specified UART STM32L496VGT6P Instance
     *
     *  @param[in] Instance Instance
     *
     *  @return UART_STM32L496VGT6P_Status_t
     */
    UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Cycle( UART_STM32L496VGT6P_Instance_t * Instance );

    /**
     *  @brief De-initializes specified UART STM32L496VGT6P Instance
     *
     *  @param[in] Instance Instance
     *
     *  @return UART_STM32L496VGT6P_Status_t
     */
    UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_DeInitialize( UART_STM32L496VGT6P_Instance_t * Instance );

    /**
     *  @brief Writes data to specified UART STM32L496VGT6P Instance
     *
     *  @param[in] Instance   Instance
     *  @param[in] Data       Data buffer
     *  @param[in] DataLength Length of data buffer
     *
     *  @return UART_STM32L496VGT6P_Status_t
     */
    UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Write( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength );

    /**
     *  @brief Reads data from specified UART STM32L496VGT6P Instance
     *
     *  @param[in]     Instance   Instance
     *  @param[in,out] Data       Data buffer
     *  @param[in]     DataLength Length of data buffer
     *
     *  @return UART_STM32L496VGT6P_Status_t
     */
    UART_STM32L496VGT6P_Status_t UART_STM32L496VGT6P_Read( UART_STM32L496VGT6P_Instance_t * Instance, UART_STM32L496VGT6P_Data_t * Data, UART_STM32L496VGT6P_DataLength_t DataLength );

    // TODO Add More APIs

    // #############################################################################
    // #### Public Variable(s) #####################################################
    // #############################################################################

    /**
     *  @brief Version
     */
    extern const char UART_STM32L496VGT6P_VERSION[];

    // #############################################################################
    // #### File Guard #############################################################
    // #############################################################################

    #ifdef __cplusplus
} /* extern "C" */
    #endif /* __cplusplus */

#endif /* UART_STM32L496VGT6P_H_ */

/**
 *  @}
 *
 *  @}
 */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
