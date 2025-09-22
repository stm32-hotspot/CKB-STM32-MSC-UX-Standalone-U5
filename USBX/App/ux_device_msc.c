/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_msc.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
  ******************************************************************************
   * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ux_device_msc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BLOCK_START_ADDR         0     /* Block start address      */
#define NUM_OF_BLOCKS            5     /* Total number of blocks   */
#define BUFFER_WORDS_SIZE        ((MMC_BLOCKSIZE * NUM_OF_BLOCKS) >> 2) /* Total data size in bytes */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

__IO uint8_t MMC_READ_FLAG =0;
__IO uint8_t MMC_WRITE_FLAG=0;

#define MMC_TIMEOUT     1000U

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

__IO uint8_t eMMC_Init = 0;

/* USER CODE END PD */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_STORAGE_Activate
  *         This function is called when insertion of a storage device.
  * @param  storage_instance: Pointer to the storage class instance.
  * @retval none
  */
VOID USBD_STORAGE_Activate(VOID *storage_instance)
{
  /* USER CODE BEGIN USBD_STORAGE_Activate */
  UX_PARAMETER_NOT_USED(storage_instance);
  /* USER CODE END USBD_STORAGE_Activate */

  return;
}

/**
  * @brief  USBD_STORAGE_Deactivate
  *         This function is called when extraction of a storage device.
  * @param  storage_instance: Pointer to the storage class instance.
  * @retval none
  */
VOID USBD_STORAGE_Deactivate(VOID *storage_instance)
{
  /* USER CODE BEGIN USBD_STORAGE_Deactivate  */
  UX_PARAMETER_NOT_USED(storage_instance);
  /* USER CODE END USBD_STORAGE_Deactivate */

  return;
}

/**
  * @brief  USBD_STORAGE_Read
  *         This function is invoked to read from media.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  data_pointer: Address of the buffer to be used for reading or writing.
  * @param  number_blocks: number of sectors to read/write.
  * @param  lba: Logical block address is the sector address to read.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Read(VOID *storage_instance, ULONG lun, UCHAR *data_pointer,
                       ULONG number_blocks, ULONG lba, ULONG *media_status)
{
   UINT status = UX_SUCCESS;
  /* USER CODE BEGIN USBD_STORAGE_Read */
   uint32_t timeout = 10000;
//  UX_PARAMETER_NOT_USED(storage_instance);
//  UX_PARAMETER_NOT_USED(lun);
//  UX_PARAMETER_NOT_USED(media_status);
  if (BSP_MMC_ReadBlocks((uint32_t*)data_pointer, lba, number_blocks, timeout) == BSP_ERROR_NONE)
  {
    // Wait until MMC card is ready for next operation
    while (BSP_MMC_GetCardState() != MMC_TRANSFER_OK)
    {
      if (timeout-- == 0)
      {
           while(1);
      }
    }
    status = UX_STATE_NEXT;
    //  MMC_READ_FLAG=0;
  } else
 {
   while(1);
 }

#if (ENABLE_MMC_DMA_CACHE_MAINTENANCE == 1)
  // Cache maintenance if needed (adjust alignedAddr and size accordingly)
  uint32_t alignedAddr = ((uint32_t)data_pointer) & ~0x1F;
  SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, number_blocks * MMC_BLOCKSIZE + ((uint32_t)data_pointer - alignedAddr));
#endif
  
  /* USER CODE END USBD_STORAGE_Read */

  return status;
}

/**
  * @brief  USBD_STORAGE_Write
  *         This function is invoked to write in media.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  data_pointer: Address of the buffer to be used for reading or writing.
  * @param  number_blocks: number of sectors to read/write.
  * @param  lba: Logical block address is the sector address to read.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Write(VOID *storage_instance, ULONG lun, UCHAR *data_pointer,
                        ULONG number_blocks, ULONG lba, ULONG *media_status)
{
  UINT status = UX_SUCCESS;
    /* USER CODE BEGIN USBD_STORAGE_Write */
  uint32_t timeout = 1000000;
//  UX_PARAMETER_NOT_USED(storage_instance);
//  UX_PARAMETER_NOT_USED(lun);
//  UX_PARAMETER_NOT_USED(media_status);
 if (BSP_MMC_WriteBlocks((uint32_t*)data_pointer, lba, number_blocks, timeout) == BSP_ERROR_NONE)
 {
   // Wait until MMC card is ready for next operation
   while (BSP_MMC_GetCardState() != MMC_TRANSFER_OK)
   {
      if (timeout-- == 0)
     {
      status= UX_ERROR;  // Timeout expired
      }
   }
   status = UX_STATE_NEXT;
  //MMC_WRITE_FLAG=0;
 }
 else
 {
   while(1);
 }

  // Cache maintenance is not needed if MPU is configured as write-through (as per your comment)
  /* USER CODE END USBD_STORAGE_Write */

  return status;
}

/**
  * @brief  USBD_STORAGE_Flush
  *         This function is invoked to flush media.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  number_blocks: number of sectors to read/write.
  * @param  lba: Logical block address is the sector address to read.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Flush(VOID *storage_instance, ULONG lun, ULONG number_blocks,
                        ULONG lba, ULONG *media_status)
{
  UINT status = UX_SUCCESS;
  /* USER CODE BEGIN USBD_STORAGE_Flush */
  uint32_t timeout = 10000;
//  UX_PARAMETER_NOT_USED(storage_instance);
//  UX_PARAMETER_NOT_USED(lun);
//  UX_PARAMETER_NOT_USED(number_blocks);
//  UX_PARAMETER_NOT_USED(lba);
//  UX_PARAMETER_NOT_USED(media_status);
  // Optional: Erase blocks if needed before flush
  // Note: USB MSC does not require explicit erase, so this is optional and depends on your use case

  if (BSP_MMC_Erase(BLOCK_START_ADDR, MMC_BLOCKSIZE * NUM_OF_BLOCKS) == BSP_ERROR_NONE)
  {
    while (BSP_MMC_GetCardState() != MMC_TRANSFER_OK)
    {
      if (timeout-- == 0)
      {
        status= UX_ERROR;  // Timeout expired
      }
    }
    status = UX_STATE_NEXT;   
  }
  else
  {
    // If erase is not supported or fails, still return success to avoid blocking USB
  while(1);
  }
   status = UX_STATE_NEXT;
  /* USER CODE END USBD_STORAGE_Flush */

  return status;
}

/**
  * @brief  USBD_STORAGE_Status
  *         This function is invoked to obtain the status of the device.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  media_id: is not currently used.
  * @param  media_status: should be filled out exactly like the media status
  *                       callback return value.
  * @retval status
  */
UINT USBD_STORAGE_Status(VOID *storage_instance, ULONG lun, ULONG media_id,
                         ULONG *media_status)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Status */
  HAL_MMC_CardCIDTypeDef pCID;
  HAL_MMC_CardCSDTypeDef pCSD;
//  UX_PARAMETER_NOT_USED(storage_instance);
//  UX_PARAMETER_NOT_USED(lun);
//  UX_PARAMETER_NOT_USED(media_id);
//  UX_PARAMETER_NOT_USED(media_status);
  if (BSP_MMC_GetCardState() == MMC_TRANSFER_OK)
  {
    status = UX_SUCCESS;
  }
  else
  {
    status = UX_ERROR;
  }
  HAL_MMC_GetCardCID(&uSdHandle, &pCID);
  HAL_MMC_GetCardCSD(&uSdHandle, &pCSD);
  /* USER CODE END USBD_STORAGE_Status */

  return status;
}

/**
  * @brief  USBD_STORAGE_Notification
  *         This function is invoked to obtain the notification of the device.
  * @param  storage_instance : Pointer to the storage class instance.
  * @param  lun: Logical unit number is the command is directed to.
  * @param  media_id: is not currently used.
  * @param  notification_class: specifies the class of notification.
  * @param  media_notification: response for the notification.
  * @param  media_notification_length: length of the response buffer.
  * @retval status
  */
UINT USBD_STORAGE_Notification(VOID *storage_instance, ULONG lun, ULONG media_id,
                               ULONG notification_class, UCHAR **media_notification,
                               ULONG *media_notification_length)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_STORAGE_Notification */
  UX_PARAMETER_NOT_USED(storage_instance);
  UX_PARAMETER_NOT_USED(lun);
  UX_PARAMETER_NOT_USED(media_id);
  UX_PARAMETER_NOT_USED(notification_class);
  UX_PARAMETER_NOT_USED(media_notification);
  UX_PARAMETER_NOT_USED(media_notification_length);
  /* USER CODE END USBD_STORAGE_Notification */

  return status;
}

/**
  * @brief  USBD_STORAGE_GetMediaLastLba
  *         Get Media last LBA.
  * @param  none
  * @retval last lba
  */
ULONG USBD_STORAGE_GetMediaLastLba(VOID)
{
  ULONG LastLba = 0U;

  /* USER CODE BEGIN USBD_STORAGE_GetMediaLastLba */
if(eMMC_Init == 0)
{
  eMMC_Init = 1;
  BSP_MMC_Init();
}
  HAL_MMC_CardInfoTypeDef pCardInfo = {0};
  HAL_MMC_GetCardInfo(&uSdHandle, &pCardInfo);

  LastLba = pCardInfo.BlockNbr-1;
  /* USER CODE END USBD_STORAGE_GetMediaLastLba */

  return LastLba;
}

/**
  * @brief  USBD_STORAGE_GetMediaBlocklength
  *         Get Media block length.
  * @param  none.
  * @retval block length.
  */
ULONG USBD_STORAGE_GetMediaBlocklength(VOID)
{
  ULONG MediaBlockLen = 0U;
  /* USER CODE BEGIN USBD_STORAGE_GetMediaBlocklength */  
  if(eMMC_Init == 0)
  {
    eMMC_Init = 1;
    BSP_MMC_Init();
  }
  HAL_MMC_CardInfoTypeDef pCardInfo = {0};
  HAL_MMC_GetCardInfo(&uSdHandle, &pCardInfo);
    MediaBlockLen = pCardInfo.BlockSize;
  /* USER CODE END USBD_STORAGE_GetMediaBlocklength */
  return MediaBlockLen;
}

/* USER CODE BEGIN 1 */
void BSP_MMC_WriteCpltCallback(void)
{
  MMC_WRITE_FLAG=1;
  
}
  void BSP_MMC_ReadCpltCallback(void)
{
  MMC_READ_FLAG=1;
  
}
/* USER CODE END 1 */
