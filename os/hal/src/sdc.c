/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    sdc.c
 * @brief   SDC Driver code.
 *
 * @addtogroup SDC
 * @{
 */

#include <string.h>

#include "hal.h"

#if (HAL_USE_SDC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Virtual methods table.
 */
static const struct SDCDriverVMT sdc_vmt = {
  (bool (*)(void *))sdc_lld_is_card_inserted,
  (bool (*)(void *))sdc_lld_is_write_protected,
  (bool (*)(void *))sdcConnect,
  (bool (*)(void *))sdcDisconnect,
  (bool (*)(void *, uint32_t, uint8_t *, uint32_t))sdcRead,
  (bool (*)(void *, uint32_t, const uint8_t *, uint32_t))sdcWrite,
  (bool (*)(void *))sdcSync,
  (bool (*)(void *, BlockDeviceInfo *))sdcGetInfo
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Wait for the card to complete pending operations.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool _sdc_wait_for_transfer_state(SDCDriver *sdcp) {
  uint32_t resp[1];
  int32_t cnt = 2000;

  while (TRUE) {
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SEND_STATUS,
                                   sdcp->rca, resp) ||
        MMCSD_R1_ERROR(resp[0])) {
      return HAL_FAILED;
    }

    cnt --;
    if(cnt <= 0)
      return HAL_FAILED;

    switch (MMCSD_R1_STS(resp[0])) {
    case MMCSD_STS_TRAN:
      return HAL_SUCCESS;
    case MMCSD_STS_DATA:
    case MMCSD_STS_RCV:
    case MMCSD_STS_PRG:
#if SDC_NICE_WAITING == TRUE
      osalThreadSleepMilliseconds(1);
#endif
      continue;
    default:
      /* The card should have been initialized so any other state is not
         valid and is reported as an error.*/
      return HAL_FAILED;
    }
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   SDC Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void sdcInit(void) {

  sdc_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p SDCDriver structure.
 *
 * @param[out] sdcp     pointer to the @p SDCDriver object
 *
 * @init
 */
void sdcObjectInit(SDCDriver *sdcp) {

  sdcp->vmt      = &sdc_vmt;
  sdcp->state    = BLK_STOP;
  sdcp->errors   = SDC_NO_ERROR;
  sdcp->config   = NULL;
  sdcp->capacity = 0;

#if CH_CFG_USE_MUTEXES
  chMtxObjectInit(&sdcp->mutex);
#endif
}

/**
 * @brief   Configures and activates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] config    pointer to the @p SDCConfig object, can be @p NULL if
 *                      the driver supports a default configuration or
 *                      requires no configuration
 *
 * @api
 */
int sdcStart(SDCDriver *sdcp, const SDCConfig *config) {

  osalDbgCheck(sdcp != NULL);
  if (sdcp == NULL) {
    return SDCD_ERROR_NULLPTR;
  }

  osalSysLock();
  osalDbgAssert((sdcp->state == BLK_STOP) || (sdcp->state == BLK_ACTIVE),
                "invalid state");
  if ((sdcp->state != BLK_STOP) && (sdcp->state != BLK_ACTIVE)) {
    chSysUnlock();
    return SDCD_ERROR_STATE;
  }
  sdcp->config = config;
  sdc_lld_start(sdcp);
  sdcp->state = BLK_ACTIVE;
  osalSysUnlock();
  return 0;
}

/**
 * @brief   Deactivates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @api
 */
int sdcStop(SDCDriver *sdcp) {

  if (sdcp == NULL) {
    return SDCD_ERROR_NULLPTR;
  }

  #if CH_CFG_USE_MUTEXES
  chMtxLock(&sdcp->mutex);
  #endif

  osalSysLock();
  if ((sdcp->state != BLK_STOP) && (sdcp->state != BLK_ACTIVE)) {
    osalSysUnlock();

    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif
  
    return SDCD_ERROR_STATE;    
  }
  sdc_lld_stop(sdcp);
  sdcp->state = BLK_STOP;
  osalSysUnlock();

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif
  
  return 0;
}

/**
 * @brief   Performs the initialization procedure on the inserted card.
 * @details This function should be invoked when a card is inserted and
 *          brings the driver in the @p BLK_READY state where it is possible
 *          to perform read and write operations.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @api
 */
bool sdcConnect(SDCDriver *sdcp) {
  uint32_t resp[1];

  if (sdcp == NULL)
    return SDCD_ERROR_NULLPTR;

  #if CH_CFG_USE_MUTEXES
  chMtxLock(&sdcp->mutex);
  #endif

  if ((sdcp->state != BLK_ACTIVE) && (sdcp->state != BLK_READY)){

    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif
    
    return SDCD_ERROR_STATE;    
  }

  /* Connection procedure in progress.*/
  sdcp->state = BLK_CONNECTING;

  /* Card clock initialization.*/
  sdc_lld_start_clk(sdcp);

  /* Enforces the initial card state.*/
  sdc_lld_send_cmd_none(sdcp, MMCSD_CMD_GO_IDLE_STATE, 0);

  /* V2.0 cards detection.*/
  if (!sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SEND_IF_COND,
                                  MMCSD_CMD8_PATTERN, resp)) {
    sdcp->cardmode = SDC_MODE_CARDTYPE_SDV20;
    /* Voltage verification.*/
    if (((resp[0] >> 8) & 0xF) != 1)
      goto failed;
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_APP_CMD, 0, resp) ||
        MMCSD_R1_ERROR(resp[0]))
      goto failed;
  }
  else {
#if SDC_MMC_SUPPORT
    /* MMC or SD V1.1 detection.*/
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_APP_CMD, 0, resp))
      sdcp->cardmode = SDC_MODE_CARDTYPE_MMC;
    else
#endif /* SDC_MMC_SUPPORT */
      sdcp->cardmode = SDC_MODE_CARDTYPE_SDV11;
      sdc_lld_send_cmd_none(sdcp, MMCSD_CMD_GO_IDLE_STATE, 0);
  }

#if SDC_MMC_SUPPORT
  if ((sdcp->cardmode &  SDC_MODE_CARDTYPE_MASK) == SDC_MODE_CARDTYPE_MMC) {
    uint32_t ocr;

    /* MMC initialization.*/
    ocr = OCR_BUSY | MMC_VDD_32_33 | MMC_VDD_33_34;

    /* Some cards seem to need this */
    sdc_lld_send_cmd_none(sdcp, MMCSD_CMD_GO_IDLE_STATE, 0);
    do {
      /* Inquire OCR, arg=0 firstly, then arg=OCR bits */
      if (sdc_lld_send_cmd_short(sdcp, MMCSD_CMD_SEND_OP_COND, 0, resp))
        goto failed;
      ocr = ((MMC_VDD_32_33 | MMC_VDD_33_34) & (resp[0] & OCR_VOLTAGE_MASK)) |
	(resp[0] & OCR_ACCESS_MODE);
      if (sdc_lld_send_cmd_short(sdcp, MMCSD_CMD_SEND_OP_COND, ocr, resp))
        goto failed;
      ocr = resp[0];
    } while (!(ocr & OCR_BUSY));
    if (ocr & OCR_HCS)
      sdcp->cardmode |= SDC_MODE_HIGH_CAPACITY;
  }
  else
#endif /* SDC_MMC_SUPPORT */
  {
    unsigned i;
    uint32_t ocr;

    /* SD initialization.*/
    if ((sdcp->cardmode &  SDC_MODE_CARDTYPE_MASK) == SDC_MODE_CARDTYPE_SDV20)
      ocr = 0xC0100000;
    else
      ocr = 0x80100000;

    /* SD-type initialization. */
    i = 0;
    while (TRUE) {
      if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_APP_CMD, 0, resp) ||
        MMCSD_R1_ERROR(resp[0]))
        goto failed;
      if (sdc_lld_send_cmd_short(sdcp, MMCSD_CMD_APP_OP_COND, ocr, resp))
        goto failed;
      if ((resp[0] & 0x80000000) != 0) {
        if (resp[0] & 0x40000000)
          sdcp->cardmode |= SDC_MODE_HIGH_CAPACITY;
        break;
      }
      if (++i >= SDC_INIT_RETRY)
        goto failed;
      chThdSleepMilliseconds(10);
    }
  }

  /* Reads CID.*/
  if (sdc_lld_send_cmd_long_crc(sdcp, MMCSD_CMD_ALL_SEND_CID, 0, sdcp->cid)) {
    goto failed;
  }

  /* Asks for the RCA.*/
  if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SEND_RELATIVE_ADDR,
                                 0, &sdcp->rca)) {
    goto failed;
  }

  /* Reads CSD.*/
  if (sdc_lld_send_cmd_long_crc(sdcp, MMCSD_CMD_SEND_CSD,
                                sdcp->rca, sdcp->csd))
    goto failed;

  /* Switches to high speed.*/
  sdc_lld_set_data_clk(sdcp);

  /* Selects the card for operations.*/
  if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SEL_DESEL_CARD,
                                 sdcp->rca, resp))
    goto failed;

  /* Block length fixed at 512 bytes.*/
  if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SET_BLOCKLEN,
                                 MMCSD_BLOCK_SIZE, resp) ||
      MMCSD_R1_ERROR(resp[0])) {
    goto failed;
  }

  /* Switches to wide bus mode.*/
  switch (sdcp->cardmode & SDC_MODE_CARDTYPE_MASK) {
  case SDC_MODE_CARDTYPE_SDV11:
  case SDC_MODE_CARDTYPE_SDV20:
    #if defined(HS6601_MCUCONF)
    if (SDC_MODE_1BIT == sdc_lld_get_bus_width(sdcp))
      break;
    #endif
    sdc_lld_set_bus_mode(sdcp, SDC_MODE_4BIT);
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_APP_CMD, sdcp->rca, resp) ||
        MMCSD_R1_ERROR(resp[0]))
      goto failed;
    if (sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_SET_BUS_WIDTH, 2, resp) ||
        MMCSD_R1_ERROR(resp[0]))
      goto failed;
    break;
#if SDC_MMC_SUPPORT
#endif
  }

  /* Determine capacity.*/
  sdcp->capacity = _mmcsd_get_capacity(sdcp->csd);  //, sdcp->cardmode & SDC_MODE_HIGH_CAPACITY);
  if (sdcp->capacity == 0)
    goto failed;

  /* Initialization complete.*/
  sdcp->state = BLK_READY;

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif
  
  return HAL_SUCCESS;

  /* Connection failed, state reset to BLK_ACTIVE.*/
failed:
  sdc_lld_stop_clk(sdcp);
  sdcp->state = BLK_ACTIVE;

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif
  
  return HAL_FAILED;
}

/**
 * @brief   Brings the driver in a state safe for card removal.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @api
 */
bool sdcDisconnect(SDCDriver *sdcp) {

  if (sdcp == NULL)
    return SDCD_ERROR_NULLPTR;

  #if CH_CFG_USE_MUTEXES
  chMtxLock(&sdcp->mutex);
  #endif
  
  osalSysLock();
  if ((sdcp->state != BLK_ACTIVE) && (sdcp->state != BLK_READY)){

    osalSysUnlock();    
    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif  
    return SDCD_ERROR_STATE;    
  }

  if (sdcp->state == BLK_ACTIVE) {    
    osalSysUnlock();    
    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif    
    return HAL_SUCCESS;
  }
  
  sdcp->state = BLK_DISCONNECTING;
  osalSysUnlock();

  /* Waits for eventual pending operations completion.*/
  _sdc_wait_for_transfer_state(sdcp);
  
  /* Card clock stopped.*/
  sdc_lld_stop_clk(sdcp);
  sdcp->state = BLK_ACTIVE;

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif  
  return HAL_SUCCESS;
}

/**
 * @brief   Reads one or more blocks.
 * @pre     The driver must be in the @p BLK_READY state after a successful
 *          sdcConnect() invocation.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer
 * @param[in] n         number of blocks to read
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @api
 */
bool sdcRead(SDCDriver *sdcp, uint32_t startblk, uint8_t *buf, uint32_t n) {
  bool status;

  if((sdcp == NULL) || (buf == NULL) || (n == 0U))
    return HAL_FAILED;

  #if CH_CFG_USE_MUTEXES
  chMtxLock(&sdcp->mutex);
  #endif

  if(sdcp->state != BLK_READY){
    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif 
    return HAL_FAILED;
  }

  if ((startblk + n - 1U) > sdcp->capacity){
    sdcp->errors |= SDC_OVERFLOW_ERROR;

    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif 
    return HAL_FAILED;
  }

  /* Read operation in progress.*/
  sdcp->state = BLK_READING;

  status = sdc_lld_read(sdcp, startblk, buf, n);

  /* Read operation finished.*/
  sdcp->state = BLK_READY;

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif 
  return status;
}

/**
 * @brief   Writes one or more blocks.
 * @pre     The driver must be in the @p BLK_READY state after a successful
 *          sdcConnect() invocation.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[out] buf      pointer to the write buffer
 * @param[in] n         number of blocks to write
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @api
 */
bool sdcWrite(SDCDriver *sdcp, uint32_t startblk,
              const uint8_t *buf, uint32_t n) {
  bool status;

  if((sdcp == NULL) || (buf == NULL) || (n == 0U))
    return HAL_FAILED;

  #if CH_CFG_USE_MUTEXES
  chMtxLock(&sdcp->mutex);
  #endif

  if(sdcp->state != BLK_READY){
    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif 
    return HAL_FAILED;
  }

  if ((startblk + n - 1U) > sdcp->capacity){
    sdcp->errors |= SDC_OVERFLOW_ERROR;

    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif 
    return HAL_FAILED;
  }

  /* Write operation in progress.*/
  sdcp->state = BLK_WRITING;

  status = sdc_lld_write(sdcp, startblk, buf, n);

  /* Write operation finished.*/
  sdcp->state = BLK_READY;

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif 
  
  return status;
}

/**
 * @brief   Returns the errors mask associated to the previous operation.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @return              The errors mask.
 *
 * @api
 */
sdcflags_t sdcGetAndClearErrors(SDCDriver *sdcp) {
  sdcflags_t flags;

  osalDbgCheck(sdcp != NULL);
  osalDbgAssert(sdcp->state == BLK_READY, "invalid state");

  osalSysLock();
  flags = sdcp->errors;
  sdcp->errors = SDC_NO_ERROR;
  osalSysUnlock();
  return flags;
}

/**
 * @brief   Waits for card idle condition.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  the operation succeeded.
 * @retval HAL_FAILED   the operation failed.
 *
 * @api
 */
bool sdcSync(SDCDriver *sdcp) {
  bool result;

  osalDbgCheck(sdcp != NULL);

  if (sdcp->state != BLK_READY) {
    return HAL_FAILED;
  }

  /* Synchronization operation in progress.*/
  sdcp->state = BLK_SYNCING;

  result = sdc_lld_sync(sdcp);

  /* Synchronization operation finished.*/
  sdcp->state = BLK_READY;
  return result;
}

/**
 * @brief   Returns the media info.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[out] bdip     pointer to a @p BlockDeviceInfo structure
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  the operation succeeded.
 * @retval HAL_FAILED   the operation failed.
 *
 * @api
 */
bool sdcGetInfo(SDCDriver *sdcp, BlockDeviceInfo *bdip) {

  osalDbgCheck((sdcp != NULL) && (bdip != NULL));

  if (sdcp->state != BLK_READY) {
    return HAL_FAILED;
  }

  bdip->blk_num = sdcp->capacity;
  bdip->blk_size = MMCSD_BLOCK_SIZE;

  return HAL_SUCCESS;
}

/**
 * @brief   Erases the supplied blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  starting block number
 * @param[in] endblk    ending block number
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  the operation succeeded.
 * @retval HAL_FAILED   the operation failed.
 *
 * @api
 */
bool sdcErase(SDCDriver *sdcp, uint32_t startblk, uint32_t endblk) {
  uint32_t resp[1];

  if(sdcp == NULL)
    return HAL_FAILED;

  #if CH_CFG_USE_MUTEXES
  chMtxLock(&sdcp->mutex);
  #endif

  if(sdcp->state != BLK_READY){
    #if CH_CFG_USE_MUTEXES
    chMtxUnlock(&sdcp->mutex);
    #endif 
    return HAL_FAILED;
  }

  /* Erase operation in progress.*/
  sdcp->state = BLK_WRITING;

  /* Handling command differences between HC and normal cards.*/
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY)) {
    startblk *= MMCSD_BLOCK_SIZE;
    endblk *= MMCSD_BLOCK_SIZE;
  }

  if (_sdc_wait_for_transfer_state(sdcp)) {
    goto failed;
  }

  if ((sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_ERASE_RW_BLK_START,
                                  startblk, resp) != HAL_SUCCESS) ||
      MMCSD_R1_ERROR(resp[0])) {
    goto failed;
  }

  if ((sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_ERASE_RW_BLK_END,
                                  endblk, resp) != HAL_SUCCESS) ||
      MMCSD_R1_ERROR(resp[0])) {
    goto failed;
  }

  if ((sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_ERASE,
                                  0, resp) != HAL_SUCCESS) ||
      MMCSD_R1_ERROR(resp[0])) {
    goto failed;
  }

  /* Quick sleep to allow it to transition to programming or receiving state */
  /* TODO: ??????????????????????????? */

  /* Wait for it to return to transfer state to indicate it has finished erasing */
  if (_sdc_wait_for_transfer_state(sdcp)) {
    goto failed;
  }

  sdcp->state = BLK_READY;

  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif     
  return HAL_SUCCESS;

failed:
  sdcp->state = BLK_READY;
  
  #if CH_CFG_USE_MUTEXES
  chMtxUnlock(&sdcp->mutex);
  #endif 
  return HAL_FAILED;
}

#endif /* HAL_USE_SDC == TRUE */

/** @} */

