/***********************************************************************************************************************
 * File Name    : filex_file_operation.c
 * Description  : Contains data structures and functions used in FileX thread.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

#include "filex_file_operation.h"
#include "filex_dir_operation.h"
#include "filex_media_operation.h"

/* Private global variables */
static CHAR g_file_name1[] = FILE_NAME_ONE;
static CHAR g_write_data[WRITE_BUFFER_SIZE] = {RESET_VALUE};

/* Private functions declaration */
static void create_fixed_buffer(void);

/* Functions implementation */

/*******************************************************************************************************************//**
 * @brief       This function creates a fixed data buffer.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
static void create_fixed_buffer(void)
{
    CHAR * p_data = g_write_data;

    /* Clean write buffer */
    memset(p_data, NULL_CHAR, WRITE_BUFFER_SIZE);

    /* Create fixed buffer */
    for (uint16_t i = 0; i < WRITE_BUFFER_SIZE / WRITE_LINE_SIZE ; i ++)
    {
        strncpy(p_data, WRITE_LINE_TEXT, WRITE_LINE_SIZE);
        p_data += WRITE_LINE_SIZE;
    }
}

/*******************************************************************************************************************//**
 * @brief       This function creates a file.
 * @param[in]   None
 * @retval      FX_SUCCESS   Upon successful operation
 * @retval      Any Other Error code apart from FX_SUCCESS
 **********************************************************************************************************************/
UINT file_create(void)
{
    UINT status = FX_SUCCESS;
    entry_info_t entry = {RESET_VALUE};

    /* Verify the current state of the media */
    status = media_verify();
    if (FX_SUCCESS != status)
    {
        return FX_SUCCESS;
    }

    /* Create a new file using the Azure FileX API */
    status = fx_file_create(&g_fx_media, g_file_name1);

    if (FX_ALREADY_CREATED == status)
    {
        PRINT_INFO_STR("File already exists\r\n");
        return FX_SUCCESS;
    }

    if (FX_SUCCESS != status)
    {
        RETURN_ERR_STR(status, "fx_file_create failed\r\n");
    }

    /* Flushes data into the physical media */
    status = fx_media_flush(&g_fx_media);
    RETURN_ERR_STR(status, "fx_media_flush failed\r\n");

    /* Get file name */
    memcpy(entry.name, g_file_name1, strlen(g_file_name1) + ONE_BYTE);

    /* Get file full information */
    status = fx_directory_information_get(&g_fx_media,
                                          entry.name, &entry.attr, &entry.size,
                                          &entry.time.year, &entry.time.month, &entry.time.date,
                                          &entry.time.hour, &entry.time.min, &entry.time.sec);
    RETURN_ERR_STR(status, "fx_directory_information_get failed\r\n");

    /* Display file information */
    PRINT_ENTRY_INFO(entry);

    /* Create a new file successfully */
    PRINT_INFO_STR("\r\nFile created successful\r\n");

    return FX_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief       This function writes fixed data to a file.
 * @param[in]   None
 * @retval      FX_SUCCESS   Upon successful operation
 * @retval      Any Other Error code apart from FX_SUCCESS
 **********************************************************************************************************************/
UINT file_write(void)
{
    UINT status = RESET_VALUE;
    UINT status_temp = FX_SUCCESS;
    ULONG actual_event = RESET_VALUE;
    FX_FILE file = {RESET_VALUE};
    time_new_t time = {RESET_VALUE};
    entry_info_t entry = {RESET_VALUE};

    /* Verify the current state of the media */
    status = media_verify();
    if (FX_SUCCESS != status)
    {
        return FX_SUCCESS;
    }

    /* Open the file for writing by using the Azure FileX API */
    status = fx_file_open(&g_fx_media, &file, g_file_name1, FX_OPEN_FOR_WRITE);

    if (FX_NOT_FOUND == status)
    {
        PRINT_INFO_STR("File does not exist\r\n");
        return FX_SUCCESS;
    }

    if (FX_SUCCESS != status)
    {
        RETURN_ERR_STR(status, "fx_file_open failed\r\n");
    }

    /* Clean the file contents */
    status = fx_file_extended_truncate(&file, TRUNCATE_VALUE);
    if (FX_SUCCESS != status)
    {
        /* Close the file using the Azure FileX API */
        status_temp = fx_file_close(&file);
        RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

        /* Return fx_file_extended_truncate failed status */
        RETURN_ERR_STR(status, "fx_file_extended_truncate failed\r\n");
    }

    /* Create fixed buffer */
    create_fixed_buffer();

    /* Write 4GB content to the opened file */
    for (ULONG i = 0; i < WRITE_TIMES ; i++ )
    {
        /* Write fixed buffer to a file */
        status = fx_file_write(&file, (VOID *)g_write_data, WRITE_BUFFER_SIZE);
        if (FX_SUCCESS != status)
        {
            /* Close the file using the Azure FileX API */
            status_temp = fx_file_close(&file);
            RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

            /* Return fx_file_write failed status */
            RETURN_ERR_STR(status, "fx_file_write failed\r\n");
        }

        /* Wait until the complete event flag is received */
        status = tx_event_flags_get(&g_media_event,
                                    RM_BLOCK_MEDIA_EVENT_WAIT_END,
                                    TX_OR_CLEAR, &actual_event, OPERATION_TIME_OUT);
        if (TX_SUCCESS != status)
        {
            /* Close the file using the Azure FileX API */
            status_temp = fx_file_close(&file);
            RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

            /* Return tx_event_flags_get failed status */
            RETURN_ERR_STR(status, "tx_event_flags_get for RM_BLOCK_MEDIA_EVENT_WAIT_END event failed\r\n");
        }

        if (RESET_VALUE == i % WRITE_ONE_PERCENT)
        {
            PRINT_INFO_STR(".");
        }
    }

    PRINT_INFO_STR("\r\n\r\n");

    /* Get system time */
    status = fx_system_time_get(&time.hour, & time.min, &time.sec);
    if (FX_SUCCESS != status)
    {
        /* Close the file using the Azure FileX API */
        status_temp = fx_file_close(&file);
        RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

        /* Return fx_system_time_get failed status */
        RETURN_ERR_STR(status, "fx_system_time_get failed\r\n");
    }

    /* Get system date */
    status = fx_system_date_get(&time.year, & time.month, &time.date);
    if (FX_SUCCESS != status)
    {
        /* Close the file using the Azure FileX API */
        status_temp = fx_file_close(&file);
        RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

        /* Return fx_system_date_get failed status */
        RETURN_ERR_STR(status, "fx_system_date_get failed\r\n");
    }

    /* Set the date and time information for the opened file */
    status = fx_file_date_time_set(&g_fx_media, g_file_name1,
                                   time.year, time.month, time.date,
                                   time.hour, time.min, time.sec);
    if (FX_SUCCESS != status)
    {
        /* Close the file using the Azure FileX API */
        status_temp = fx_file_close(&file);
        RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

        /* Return fx_file_date_time_set failed status */
        RETURN_ERR_STR(status, "fx_file_date_time_set failed\r\n");
    }

    /* Close the file using the Azure FileX API */
    status = fx_file_close(&file);
    RETURN_ERR_STR(status, "fx_file_close failed\r\n");

    /* Flushes data into the physical media */
    status = fx_media_flush(&g_fx_media);
    RETURN_ERR_STR(status, "fx_media_flush failed\r\n");

    /* Get file name */
    memcpy(entry.name, g_file_name1, strlen(g_file_name1) + ONE_BYTE);

    /* Get file full information */
    status = fx_directory_information_get(&g_fx_media,
                                          entry.name, &entry.attr, &entry.size,
                                          &entry.time.year, &entry.time.month, &entry.time.date,
                                          &entry.time.hour, &entry.time.min, &entry.time.sec);
    RETURN_ERR_STR(status, "fx_directory_information_get failed\r\n");

    /* Display file information */
    PRINT_ENTRY_INFO(entry);

    /* Write to the file successfully. */
    PRINT_INFO_STR("\r\nWrite to a file successful\r\n");

    return FX_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief       This function reads and verifies data from a file.
 * @param[in]   None
 * @retval      FX_SUCCESS   Upon successful operation
 * @retval      Any Other Error code apart from FX_SUCCESS
 **********************************************************************************************************************/
UINT file_read(void)
{
    UINT status = FX_SUCCESS;
    UINT status_temp = FX_SUCCESS;
    ULONG actual_event = RESET_VALUE;
    FX_FILE file = {RESET_VALUE};
    ULONG len = RESET_VALUE;
    entry_info_t entry = {RESET_VALUE};
    CHAR g_read_data[READ_BUFFER_SIZE + ONE_BYTE] = {RESET_VALUE};

    /* Verify the current state of the media */
    status = media_verify();
    if (FX_SUCCESS != status)
    {
        return FX_SUCCESS;
    }

    /* Open the file for reading by using the Azure FileX API */
    status = fx_file_open(&g_fx_media, &file, g_file_name1, FX_OPEN_FOR_READ);

    if (FX_NOT_FOUND == status)
    {
        PRINT_INFO_STR("File does not exist\r\n");
        return FX_SUCCESS;
    }

    if (FX_SUCCESS != status)
    {
        RETURN_ERR_STR(status, "fx_file_open failed\r\n");
    }

    /* Seek to the beginning of the file  */
    status = fx_file_extended_seek(&file, SEEK_VALUE);
    if (FX_SUCCESS != status)
    {
       /* Close the file using the Azure FileX API */
       status_temp = fx_file_close(&file);
       RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

       /* Return fx_file_seek failed status */
       RETURN_ERR_STR(status, "fx_file_extended_seek failed\r\n");
    }

    /* Read data from the file */
    status = fx_file_read(&file, g_read_data, READ_BUFFER_SIZE, &len);

    /* In case reading the file failed */
    if (FX_END_OF_FILE != status && FX_SUCCESS != status)
    {
        /* Close the file using the Azure FileX API */
        status_temp = fx_file_close(&file);
        RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

        /* Return fx_file_read failed status */
        RETURN_ERR_STR(status, "fx_file_read failed\r\n");
    }

    /* Wait until the complete event flag is received */
    status = tx_event_flags_get(&g_media_event,
                                RM_BLOCK_MEDIA_EVENT_WAIT_END,
                                TX_OR_CLEAR, &actual_event, OPERATION_TIME_OUT);
    if (TX_SUCCESS != status)
    {
        /* Close the file using the Azure FileX API */
        status_temp = fx_file_close(&file);
        RETURN_ERR_STR(status_temp, "fx_file_close failed\r\n");

        /* Return tx_event_flags_get failed status */
        RETURN_ERR_STR(status, "tx_event_flags_get media completed flag failed\r\n");
    }

    /* Close the file using the Azure FileX API */
    status = fx_file_close(&file);
    RETURN_ERR_STR(status, "fx_file_close failed\r\n");

    /* Get file name */
    memcpy(entry.name, g_file_name1, strlen(g_file_name1) + ONE_BYTE);

    /* Get file full information */
    status = fx_directory_information_get(&g_fx_media,
                                          entry.name, &entry.attr, &entry.size,
                                          &entry.time.year, &entry.time.month, &entry.time.date,
                                          &entry.time.hour, &entry.time.min, &entry.time.sec);
    RETURN_ERR_STR(status, "fx_directory_information_get failed\r\n");

    /* Display file information */
    PRINT_ENTRY_INFO(entry);

    if (READ_BUFFER_SIZE > len)
    {
        /* Display content of the file */
        PRINT_INFO_STR("\r\nContent of the file\r\n\r\n");
        send_data_to_rtt(RTT_OUTPUT_APP_INFO_STR, len + ONE_BYTE, g_read_data);
    }
    else
    {
        /* Display content of the first 1 kB of the file */
        PRINT_INFO_STR("\r\nContent of the first 1 kB of the file\r\n\r\n");
        send_data_to_rtt(RTT_OUTPUT_APP_INFO_STR, READ_BUFFER_SIZE + ONE_BYTE, g_read_data);
    }

    PRINT_INFO_STR("\r\nEnd\r\n");

    return FX_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief       This function deletes a file.
 * @param[in]   None
 * @retval      FX_SUCCESS   Upon successful operation
 * @retval      Any Other Error code apart from FX_SUCCESS
 **********************************************************************************************************************/
UINT file_delete(void)
{
    UINT status = FX_SUCCESS;

    /* Verify the current state of the media */
    status = media_verify();
    if (FX_SUCCESS != status)
    {
        return FX_SUCCESS;
    }

    /* Delete the file using the Azure FileX API */
    status = fx_file_delete(&g_fx_media, g_file_name1);

    if (FX_NOT_FOUND == status)
    {
        PRINT_INFO_STR("File does not exist\r\n");
        return FX_SUCCESS;
    }

    if (FX_SUCCESS != status)
    {
        RETURN_ERR_STR(status, "fx_file_delete failed\r\n");
    }

    /* Flushes data into the physical media */
    status = fx_media_flush(&g_fx_media);
    RETURN_ERR_STR(status, "fx_media_flush failed\r\n");

    /* Delete the file successfully */
    PRINT_INFO_STR("File has been deleted\r\n");

    return FX_SUCCESS;
}
