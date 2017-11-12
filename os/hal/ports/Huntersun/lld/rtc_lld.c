/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn

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
/*
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    hs66xx/rtc_lld.c
 * @brief   HS66xx RTC subsystem low level driver header.
 *
 * @addtogroup RTC
 * @{
 */

#include <time.h>
#include "ch.h"
#include "hal.h"

#if HAL_USE_RTC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief RTC driver identifier.
 */
RTCDriver RTCD0;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Wait for synchronization of RTC registers with APB bus.
 * @details This function must be invoked before trying to read RTC registers.
 *
 * @notapi
 */
#define rtc_lld_apb_sync() {while ((RTCD0.id_rtc->ISR & RTC_ISR_RSF) == 0);}

/**
 * @brief   Beginning of configuration procedure.
 *
 * @notapi
 */
#define rtc_lld_enter_init() {                                                \
  RTCD0.id_rtc->ISR |= RTC_ISR_INIT;                                          \
  while ((RTCD0.id_rtc->ISR & RTC_ISR_INITF) == 0)                            \
    ;                                                                         \
}

/**
 * @brief   Finalizing of configuration procedure.
 *
 * @notapi
 */
#define rtc_lld_exit_init() {RTCD0.id_rtc->ISR &= ~RTC_ISR_INIT;}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   RTC interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(RTC_IRQHandler)
{
  uint32_t flags;

  CH_IRQ_PROLOGUE();

  /* This wait works only when AHB1 bus was previously powered off by any
     reason (standby, reset, etc). In other cases it does nothing.*/
  rtc_lld_apb_sync();

  /* Mask of all enabled and pending sources.*/
  flags = RTCD0.id_rtc->ISR;

  RTCD0.id_rtc->ISR &= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF 
                         | RTC_ISR_TSOVF | RTC_ISR_WUTF | RTC_ISR_TSF );
 
  /* not compatible with STM32F4 */
  /* disable Alarm A and Alarm B to clear wakeup signal to PMU */
  RTCD0.id_rtc->CR &= ~(RTC_CR_ALRBE | RTC_CR_ALRAE/* | RTC_CR_WUTE | RTC_CR_TSE*/);

  if ((flags & RTC_ISR_ALRAF) && RTCD0.callback)
    RTCD0.callback(&RTCD0, RTC_EVENT_ALARMA);

  if ((flags & RTC_ISR_ALRBF) && RTCD0.callback)
    RTCD0.callback(&RTCD0, RTC_EVENT_ALARMB);

  if ((flags & RTC_ISR_TSOVF) && RTCD0.callback)
    RTCD0.callback(&RTCD0, RTC_EVENT_TIMSTAMP_OVERFLOW);
  
  if ((flags & RTC_ISR_WUTF) && RTCD0.callback)
    RTCD0.callback(&RTCD0, RTC_EVENT_AUTO_WAKEUP);

  if ((flags & RTC_ISR_TSF) && RTCD0.callback)
    RTCD0.callback(&RTCD0, RTC_EVENT_TIMSTAMP); 
  
  CH_IRQ_EPILOGUE();
}
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Enable access to registers.
 *
 * @api
 */
void rtc_lld_init(void){
  RTCD0.id_rtc = HS_RTC;

  /* Asynchronous part of preloader. Set it to maximum value. */
  uint32_t prediv_a = 0x7F;

  /* Disable write protection. */
  RTCD0.id_rtc->WPR = 0x00;
  RTCD0.id_rtc->WPR = 0xCA;
  RTCD0.id_rtc->WPR = 0x53;

  /* If calendar not init yet (RTC_ISR_INITS status is kept in deep sleep) */
  if (!(HS_RTC->ISR & RTC_ISR_INITS)) {
    rtc_lld_enter_init();

    /* Prescaler register must be written in two SEPARATE writes. */
    prediv_a = (prediv_a << 16) |
      (((cpm_get_clock(HS_RTC_CLK) / (prediv_a + 1)) - 1) & 0x7FFF);
    RTCD0.id_rtc->PRER = prediv_a;
    RTCD0.id_rtc->PRER = prediv_a;
    
    rtc_lld_exit_init();
  }

  /* Callback initially disabled.*/
  RTCD0.callback = NULL;

  /* IRQ vector permanently assigned to this driver.*/
  nvicEnableVector(IRQ_RTC, ANDES_PRIORITY_MASK(HS_RTC_IRQ_PRIORITY));
}

/**
 * @brief   Set current time.
 * @note    Fractional part will be silently ignored.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] timespec  pointer to a @p RTCTime structure
 *
 * @api
 */
void rtc_lld_set_time(RTCDriver *rtcp, const RTCTime *timespec) {
  (void)rtcp;

  rtc_lld_enter_init();
  if (timespec->h12)
    RTCD0.id_rtc->CR |= RTC_CR_FMT;
  else
    RTCD0.id_rtc->CR &= ~RTC_CR_FMT;
  RTCD0.id_rtc->TR = timespec->tv_time;
  RTCD0.id_rtc->DR = timespec->tv_date;
  rtc_lld_exit_init();
}

/**
 * @brief   Get current time.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timespec pointer to a @p RTCTime structure
 *
 * @api
 */
void rtc_lld_get_time(RTCDriver *rtcp, RTCTime *timespec) {
  (void)rtcp;
  uint32_t prer = RTCD0.id_rtc->PRER;

  rtc_lld_apb_sync();

#if defined(CONFIG_CH_DBG)
  /* flash to service interrupt */
  chSysUnlock(); __NOP(); chSysLock();
#endif
  timespec->tv_msec =
      (1000 * ((prer & 0x7FFF) - RTCD0.id_rtc->SSR)) /
      ((prer & 0x7FFF) + 1);
  timespec->tv_time = RTCD0.id_rtc->TR;
  timespec->tv_date = RTCD0.id_rtc->DR;
}

/**
 * @brief     Set alarm time.
 *
 * @note      Default value after BKP domain reset for both comparators is 0.
 * @note      Function does not performs any checks of alarm time validity.
 *
 * @param[in] rtcp      Pointer to RTC driver structure.
 * @param[in] alarm     Alarm identifier. Can be 1 or 2.
 * @param[in] alarmspec Pointer to a @p RTCAlarm structure.
 *
 * @api
 */
void rtc_lld_set_alarm(RTCDriver *rtcp,
                       rtcalarm_t alarm,
                       const RTCAlarm *alarmspec) {
  if (alarm == 1){
    if (alarmspec != NULL){
      do {
      rtcp->id_rtc->CR &= ~RTC_CR_ALRAE;
      while(!(rtcp->id_rtc->ISR & RTC_ISR_ALRAWF))
        ;
      rtcp->id_rtc->ALRMAR = alarmspec->tv_datetime;
      rtcp->id_rtc->CR |= RTC_CR_ALRAE;
      rtcp->id_rtc->CR |= RTC_CR_ALRAIE;
      } while (rtcp->id_rtc->ALRMAR != alarmspec->tv_datetime);
    }
    else {
      rtcp->id_rtc->CR &= ~RTC_CR_ALRAIE;
      rtcp->id_rtc->CR &= ~RTC_CR_ALRAE;
    }
  }
  else{
    if (alarmspec != NULL){
      do {
      rtcp->id_rtc->CR &= ~RTC_CR_ALRBE;
      while(!(rtcp->id_rtc->ISR & RTC_ISR_ALRBWF))
        ;
      rtcp->id_rtc->ALRMBR = alarmspec->tv_datetime;
      rtcp->id_rtc->CR |= RTC_CR_ALRBE;
      rtcp->id_rtc->CR |= RTC_CR_ALRBIE;
      } while (rtcp->id_rtc->ALRMBR != alarmspec->tv_datetime);
    }
    else {
      rtcp->id_rtc->CR &= ~RTC_CR_ALRBIE;
      rtcp->id_rtc->CR &= ~RTC_CR_ALRBE;
    }
  }
}

/**
 * @brief   Get alarm time.
 *
 * @param[in] rtcp       pointer to RTC driver structure
 * @param[in] alarm      alarm identifier
 * @param[out] alarmspec pointer to a @p RTCAlarm structure
 *
 * @api
 */
void rtc_lld_get_alarm(RTCDriver *rtcp,
                       rtcalarm_t alarm,
                       RTCAlarm *alarmspec) {
  if (alarm == 1)
    alarmspec->tv_datetime = rtcp->id_rtc->ALRMAR;
  else
    alarmspec->tv_datetime = rtcp->id_rtc->ALRMBR;
}

/**
 * @brief   init the rtc timestamp.
 *
 * @param[in] No    1: map to AF1
 *                  2: map to AF2
 *
 * @api
 */
void rtc_timestamp_init(RTCDriver *rtcp, int No)
{
  if (No == 0) {   //AF1  rising edge
    rtcp->id_rtc->CR |= RTC_CR_TSE | RTC_CR_TSIE ;
    rtcp->id_rtc->TAFCR &= ~RTC_TAFCR_TSINSEL;
  }
  if (No == 1) {   //AF1  falling edge
    rtcp->id_rtc->CR |= RTC_CR_TSEDGE ;
    rtcp->id_rtc->TAFCR &= ~RTC_TAFCR_TSINSEL;
    rtcp->id_rtc->CR |= RTC_CR_TSE | RTC_CR_TSIE ;
  }
  if (No == 2) {  //AF2  rising edge
    rtcp->id_rtc->CR |= RTC_CR_TSE | RTC_CR_TSIE ;
    rtcp->id_rtc->TAFCR |= RTC_TAFCR_TSINSEL;
  }
  if (No == 3) {  //AF2  falling edge
    rtcp->id_rtc->CR |= RTC_CR_TSEDGE ;
    rtcp->id_rtc->TAFCR |= RTC_TAFCR_TSINSEL;
    rtcp->id_rtc->CR |= RTC_CR_TSE | RTC_CR_TSIE ;
  }
}

/**
 * @brief   Get current timestamp.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timespec pointer to a @p RTCTime structure
 *
 * @api
 */
void rtc_lld_get_timestamp(RTCDriver *rtcp, RTCTime *timespec) {
  (void)rtcp;
  uint32_t prer = RTCD0.id_rtc->PRER;

  rtc_lld_apb_sync();

  timespec->tv_msec =
      (1000 * ((prer & 0x7FFF) - RTCD0.id_rtc->TSSSR)) /
      ((prer & 0x7FFF) + 1);
  timespec->tv_time = RTCD0.id_rtc->TSTR;
  timespec->tv_date = RTCD0.id_rtc->TSDR;
}
/**
 * @brief     Sets time of periodic wakeup.
 *
 * @note      Default value after BKP domain reset is 0x0000FFFF
 *
 * @param[in] rtcp       pointer to RTC driver structure
 * @param[in] wakeupspec pointer to a @p RTCWakeup structure
 *
 * @api
 */
void rtcSetPeriodicWakeup(RTCDriver *rtcp, RTCWakeup *wakeupspec){
  chDbgCheck((wakeupspec->wakeup != 0x30000));

  if (wakeupspec != NULL){
    rtcp->id_rtc->CR &= ~RTC_CR_WUTE;
    while(!(rtcp->id_rtc->ISR & RTC_ISR_WUTWF))
      ;
    rtcp->id_rtc->WUTR = wakeupspec->wakeup & 0xFFFF;
    rtcp->id_rtc->CR   = (wakeupspec->wakeup >> 16) & 0x7;
    /* disallow interrupt if longer wakeup */
    if (((wakeupspec->wakeup >> 16) & 0x7) < RTC_WAKEUP_CLK_CK_SPRE_1Hz)
      rtcp->id_rtc->CR |= RTC_CR_WUTIE;
    rtcp->id_rtc->CR |= RTC_CR_WUTE;
  }
  else {
    rtcp->id_rtc->CR &= ~RTC_CR_WUTIE;
    rtcp->id_rtc->CR &= ~RTC_CR_WUTE;
  }
}

/**
 * @brief     Gets time of periodic wakeup.
 *
 * @note      Default value after BKP domain reset is 0x0000FFFF
 *
 * @param[in] rtcp        pointer to RTC driver structure
 * @param[out] wakeupspec pointer to a @p RTCWakeup structure
 *
 * @api
 */
void rtcGetPeriodicWakeup(RTCDriver *rtcp, RTCWakeup *wakeupspec){
  wakeupspec->wakeup  = 0;
  wakeupspec->wakeup |= rtcp->id_rtc->WUTR;
  wakeupspec->wakeup |= (((uint32_t)rtcp->id_rtc->CR) & 0x7) << 16;
}

/**
 * @brief   Get current time in format suitable for usage in FatFS.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @return              FAT time value.
 *
 * @api
 */
uint32_t rtc_lld_get_time_fat(RTCDriver *rtcp) {
  uint32_t fattime;
  RTCTime timespec;
  uint32_t tv_time;
  uint32_t tv_date;
  uint32_t v;


  chSysLock();
  rtcGetTimeI(rtcp, &timespec);
  chSysUnlock();

  tv_time = timespec.tv_time;
  tv_date = timespec.tv_date;

  v =  (tv_time & RTC_TR_SU) >> RTC_TR_SU_OFFSET;
  v += ((tv_time & RTC_TR_ST) >> RTC_TR_ST_OFFSET) * 10;
  fattime  = v >> 1;

  v =  (tv_time & RTC_TR_MNU) >> RTC_TR_MNU_OFFSET;
  v += ((tv_time & RTC_TR_MNT) >> RTC_TR_MNT_OFFSET) * 10;
  fattime |= v << 5;

  v =  (tv_time & RTC_TR_HU) >> RTC_TR_HU_OFFSET;
  v += ((tv_time & RTC_TR_HT) >> RTC_TR_HT_OFFSET) * 10;
  v += 12 * ((tv_time & RTC_TR_PM) >> RTC_TR_PM_OFFSET);
  fattime |= v << 11;

  v =  (tv_date & RTC_DR_DU) >> RTC_DR_DU_OFFSET;
  v += ((tv_date & RTC_DR_DT) >> RTC_DR_DT_OFFSET) * 10;
  fattime |= v << 16;

  v =  (tv_date & RTC_DR_MU) >> RTC_DR_MU_OFFSET;
  v += ((tv_date & RTC_DR_MT) >> RTC_DR_MT_OFFSET) * 10;
  fattime |= v << 21;

  v =  (tv_date & RTC_DR_YU) >> RTC_DR_YU_OFFSET;
  v += ((tv_date & RTC_DR_YT) >> RTC_DR_YT_OFFSET) * 10;
  v += 2000 - 1900 - 80;
  fattime |= v << 25;

  return fattime;
}

#endif /* HAL_USE_RTC */

/** @} */
