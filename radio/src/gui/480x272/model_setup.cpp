/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"
#include "storage/modelslist.h"

uint8_t g_moduleIdx;

enum MenuModelSetupItems {
  ITEM_MODEL_SETUP_NAME,
  ITEM_MODEL_SETUP_BITMAP,
  ITEM_MODEL_SETUP_TIMER1,
  ITEM_MODEL_SETUP_TIMER1_NAME,
  ITEM_MODEL_SETUP_TIMER1_PERSISTENT,
  ITEM_MODEL_SETUP_TIMER1_MINUTE_BEEP,
  ITEM_MODEL_SETUP_TIMER1_COUNTDOWN_BEEP,
#if TIMERS > 1
  ITEM_MODEL_SETUP_TIMER2,
  ITEM_MODEL_SETUP_TIMER2_NAME,
  ITEM_MODEL_SETUP_TIMER2_PERSISTENT,
  ITEM_MODEL_SETUP_TIMER2_MINUTE_BEEP,
  ITEM_MODEL_SETUP_TIMER2_COUNTDOWN_BEEP,
#endif
#if TIMERS > 2
  ITEM_MODEL_SETUP_TIMER3,
  ITEM_MODEL_SETUP_TIMER3_NAME,
  ITEM_MODEL_SETUP_TIMER3_PERSISTENT,
  ITEM_MODEL_SETUP_TIMER3_MINUTE_BEEP,
  ITEM_MODEL_SETUP_TIMER3_COUNTDOWN_BEEP,
#endif
  ITEM_MODEL_SETUP_EXTENDED_LIMITS,
  ITEM_MODEL_SETUP_EXTENDED_TRIMS,
  ITEM_MODEL_SETUP_DISPLAY_TRIMS,
  ITEM_MODEL_SETUP_TRIM_INC,
  ITEM_MODEL_SETUP_THROTTLE_LABEL,
  ITEM_MODEL_SETUP_THROTTLE_REVERSED,
  ITEM_MODEL_SETUP_THROTTLE_TRACE,
  ITEM_MODEL_SETUP_THROTTLE_TRIM,
  ITEM_MODEL_SETUP_PREFLIGHT_LABEL,
  ITEM_MODEL_SETUP_CHECKLIST_DISPLAY,
  ITEM_MODEL_SETUP_THROTTLE_WARNING,
  ITEM_MODEL_SETUP_SWITCHES_WARNING,
  ITEM_MODEL_SETUP_POTS_SLIDERS_WARNING_STATE,
  ITEM_MODEL_SETUP_POTS_WARNING,
  ITEM_MODEL_SETUP_SLIDERS_WARNING,
  ITEM_MODEL_SETUP_BEEP_CENTER,
  ITEM_MODEL_SETUP_USE_GLOBAL_FUNCTIONS,

#if defined(PXX2)
  ITEM_MODEL_SETUP_REGISTRATION_ID,
#endif

  ITEM_MODEL_SETUP_INTERNAL_MODULE_LABEL,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_TYPE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_BIND,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_MODEL_NUM,
#if defined(EXTERNAL_ANTENNA)
  ITEM_MODEL_SETUP_INTERNAL_MODULE_ANTENNA,
#endif
  ITEM_MODEL_SETUP_INTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_REGISTER_RANGE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_OPTIONS,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_2,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_3,

  ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_TYPE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_POWER,
#if defined(MULTIMODULE)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_STATUS,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_SYNCSTATUS,
#endif
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_NOT_ACCESS_BIND,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_OPTIONS,
#if defined(MULTIMODULE)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AUTOBIND,
#endif
  ITEM_MODEL_SETUP_TRAINER_LABEL,
  ITEM_MODEL_SETUP_TRAINER_MODE,
#if defined(BLUETOOTH)
  ITEM_MODEL_SETUP_TRAINER_BLUETOOTH,
#endif
  ITEM_MODEL_SETUP_TRAINER_CHANNELS,
  ITEM_MODEL_SETUP_TRAINER_PPM_PARAMS,
  ITEM_MODEL_SETUP_MAX
};

#define MODEL_SETUP_2ND_COLUMN         200
#define MODEL_SETUP_3RD_COLUMN         270
#define MODEL_SETUP_4TH_COLUMN         350
#define MODEL_SETUP_BIND_OFS           40
#define MODEL_SETUP_RANGE_OFS          80
#define MODEL_SETUP_SET_FAILSAFE_OFS   100
#define MODEL_SETUP_SLIDPOT_SPACING    45

#define CURRENT_MODULE_EDITED(k)       (k >= ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE)
#define CURRENT_RECEIVER_EDITED(k)      (k - (k >= ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL ? k : ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1))

#include "common/stdlcd/model_setup_pxx1.cpp"

bool isPXX2ReceiverEmpty(uint8_t moduleIdx, uint8_t receiverIdx)
{
  return is_memclear(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME);
}

void removePXX2Receiver(uint8_t moduleIdx, uint8_t receiverIdx)
{
  memclear(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME);
  g_model.moduleData[moduleIdx].pxx2.receivers &= ~(1 << receiverIdx);
  storageDirty(EE_MODEL);
}

void removePXX2ReceiverIfEmpty(uint8_t moduleIdx, uint8_t receiverIdx)
{
  if (isPXX2ReceiverEmpty(moduleIdx, receiverIdx)) {
    removePXX2Receiver(moduleIdx, receiverIdx);
  }
}

const char * STR_BIND_8CH_WITH_TELEM = TR("8CH with telem.", "8CH with telemetry");
const char * STR_BIND_16CH_WITH_TELEM = TR("16CH with telem.", "16CH with telemetry");
const char * STR_BIND_16CH_WITHOUT_TELEM = TR("16CH without telem.", "16CH without telemetry");
const char * STR_BIND_FLEX_868 = "Flex 868MHz";
const char * STR_BIND_FLEX_915 = "Flex 915MHz";

void onPXX2R9MBindModeMenu(const char * result)
{
  if (result == STR_BIND_16CH_WITH_TELEM) {
    reusableBuffer.moduleSetup.bindInformation.lbtMode = 1;
  }
  else if (result == STR_BIND_16CH_WITHOUT_TELEM) {
    reusableBuffer.moduleSetup.bindInformation.lbtMode = 2;
  }
  else if (result == STR_BIND_FLEX_868) {
    reusableBuffer.moduleSetup.bindInformation.flexMode = 0;
  }
  else if (result == STR_BIND_FLEX_915) {
    reusableBuffer.moduleSetup.bindInformation.flexMode = 1;
  }
  else {
    // the user pressed [Exit]
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition);
    uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition);
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
    return;
  }

#if defined(SIMU)
  uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition);
  uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition);
  memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[reusableBuffer.moduleSetup.bindInformation.selectedReceiverIndex], PXX2_LEN_RX_NAME);
  storageDirty(EE_MODEL);
  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  reusableBuffer.moduleSetup.bindInformation.step = BIND_OK;
  POPUP_INFORMATION(STR_BIND_OK);
#else
  reusableBuffer.moduleSetup.bindInformation.step = BIND_START;
#endif
}

// TODO code duplicated


void onResetReceiverConfirm(const char * result)
{
  if (result == STR_OK) {
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
    moduleState[moduleIdx].mode = MODULE_MODE_RESET;
    removePXX2Receiver(moduleIdx, receiverIdx);
  }
}

void onPXX2ReceiverMenu(const char * result)
{
  uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
  uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);

  if (result == STR_OPTIONS) {
    memclear(&reusableBuffer.hardwareAndSettings, sizeof(reusableBuffer.hardwareAndSettings));
    reusableBuffer.hardwareAndSettings.receiverSettings.receiverId = receiverIdx;
    g_moduleIdx = moduleIdx;
    // TODO pushMenu(menuModelReceiverOptions);
  }
  else if (result == STR_BIND) {
    memclear(&reusableBuffer.moduleSetup.bindInformation, sizeof(BindInformation));
    reusableBuffer.moduleSetup.bindInformation.rxUid = receiverIdx;
    if (isModuleR9MAccess(moduleIdx)) {
#if defined(SIMU)
      reusableBuffer.moduleSetup.pxx2.moduleInformation.information.modelID = 1;
      reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant = 2;
#else
      moduleState[moduleIdx].readModuleInformation(&reusableBuffer.moduleSetup.pxx2.moduleInformation, PXX2_HW_INFO_TX_ID, PXX2_HW_INFO_TX_ID);
#endif
    }
    else {
      moduleState[moduleIdx].startBind(&reusableBuffer.moduleSetup.bindInformation);
    }
    s_editMode = 1;
  }
  else if (result == STR_SHARE) {
    reusableBuffer.moduleSetup.pxx2.shareReceiverIndex = receiverIdx;
    moduleState[moduleIdx].mode = MODULE_MODE_SHARE;
    s_editMode = 1;
  }
  else if (result == STR_DELETE || result == STR_RESET) {
    memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
    reusableBuffer.moduleSetup.pxx2.resetReceiverIndex = receiverIdx;
    reusableBuffer.moduleSetup.pxx2.resetReceiverFlags = (result == STR_RESET ? 0xFF : 0x01);
    POPUP_CONFIRMATION(result == STR_RESET ? STR_RECEIVER_RESET : STR_RECEIVER_DELETE, onResetReceiverConfirm);
  }
  else {
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
  }
}

// TODO code duplicated
void onPXX2BindMenu(const char * result)
{
  if (result != STR_EXIT) {
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    reusableBuffer.moduleSetup.bindInformation.selectedReceiverIndex = (result - reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[0]) / sizeof(reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[0]);
    if (isModuleR9MAccess(moduleIdx) && reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant == PXX2_VARIANT_EU) {
      reusableBuffer.moduleSetup.bindInformation.step = BIND_RX_NAME_SELECTED;
      POPUP_MENU_ADD_ITEM(STR_16CH_WITH_TELEMETRY);
      POPUP_MENU_ADD_ITEM(STR_16CH_WITHOUT_TELEMETRY);
      POPUP_MENU_START(onPXX2R9MBindModeMenu);
    }
    else if (isModuleR9MAccess(moduleIdx) && reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant == PXX2_VARIANT_FLEX) {
      reusableBuffer.moduleSetup.bindInformation.step = BIND_RX_NAME_SELECTED;
      POPUP_MENU_ADD_ITEM(STR_FLEX_868);
      POPUP_MENU_ADD_ITEM(STR_FLEX_915);
      POPUP_MENU_START(onPXX2R9MBindModeMenu);
    }
    else {
#if defined(SIMU)
      uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
      memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], result, PXX2_LEN_RX_NAME);
      storageDirty(EE_MODEL);
      moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
      reusableBuffer.moduleSetup.bindInformation.step = BIND_OK;
      POPUP_INFORMATION(STR_BIND_OK);
#else
      reusableBuffer.moduleSetup.bindInformation.step = BIND_START;
#endif
    }
  }
  else {
    // the user pressed [Exit]
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
    s_editMode = 0;
  }
}

enum PopupRegisterItems {
  ITEM_REGISTER_PASSWORD,
  ITEM_REGISTER_MODULE_INDEX,
  ITEM_REGISTER_RECEIVER_NAME,
  ITEM_REGISTER_BUTTONS
};

constexpr coord_t REGISTER_TOP = POPUP_Y + 10;
constexpr coord_t REGISTER_H = 6 * FH - 4;
constexpr coord_t REGISTER_COLUMN_1 = POPUP_X + 10;
constexpr coord_t REGISTER_COLUMN_2 = POPUP_X + 150;

void runPopupRegister(event_t event)
{
  uint8_t backupVerticalPosition = menuVerticalPosition;
  uint8_t backupHorizontalPosition = menuHorizontalPosition;
  uint8_t backupVerticalOffset = menuVerticalOffset;
  int8_t backupEditMode = s_editMode;

  menuVerticalPosition = reusableBuffer.moduleSetup.pxx2.registerPopupVerticalPosition;
  menuHorizontalPosition = reusableBuffer.moduleSetup.pxx2.registerPopupHorizontalPosition;
  s_editMode = reusableBuffer.moduleSetup.pxx2.registerPopupEditMode;

  switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      if (menuVerticalPosition != ITEM_REGISTER_BUTTONS) {
        break;
      }
      else if (reusableBuffer.moduleSetup.pxx2.registerStep >= REGISTER_RX_NAME_RECEIVED && menuHorizontalPosition == 0) {
        // [Enter] pressed
        reusableBuffer.moduleSetup.pxx2.registerStep = REGISTER_RX_NAME_SELECTED;
        backupEditMode = EDIT_MODIFY_FIELD; // so that the [Register] button blinks and the REGISTER process can continue
      }
      // no break

    case EVT_KEY_LONG(KEY_EXIT):
      s_editMode = 0;
      // no break;

    case EVT_KEY_BREAK(KEY_EXIT):
      if (s_editMode <= 0) {
        warningText = nullptr;
      }
      break;
  }

  if (warningText) {
    const uint8_t dialogRows[] = { 0, 0, uint8_t(reusableBuffer.moduleSetup.pxx2.registerStep < REGISTER_RX_NAME_RECEIVED ? READONLY_ROW : 0), uint8_t(reusableBuffer.moduleSetup.pxx2.registerStep < REGISTER_RX_NAME_RECEIVED ? 0 : 1)};
    check(event, 0, nullptr, 0, dialogRows, 3, 4); // TODO add a comment for 3 - HEADER_LINE once understood

    drawPopupBackgroundAndBorder(POPUP_X, REGISTER_TOP, POPUP_W, REGISTER_H);
    // showMessageBox(warningText);

    // registration password
    lcdDrawText(REGISTER_COLUMN_1, REGISTER_TOP + 8, STR_REG_ID);
    editName(REGISTER_COLUMN_2, REGISTER_TOP + 8, g_model.modelRegistrationID, PXX2_LEN_REGISTRATION_ID, event, menuVerticalPosition == ITEM_REGISTER_PASSWORD);

    // loop index (will be removed in future)
    lcdDrawText(REGISTER_COLUMN_1, REGISTER_TOP + 8 + FH, "UID");
    lcdDrawNumber(REGISTER_COLUMN_2, REGISTER_TOP + 8 + FH, reusableBuffer.moduleSetup.pxx2.registerLoopIndex, menuVerticalPosition == ITEM_REGISTER_MODULE_INDEX ? (s_editMode ? INVERS + BLINK : INVERS) : 0);
    if (menuVerticalPosition == ITEM_REGISTER_MODULE_INDEX && s_editMode) {
      CHECK_INCDEC_MODELVAR_ZERO(event, reusableBuffer.moduleSetup.pxx2.registerLoopIndex, 2);
    }

    // RX name
    if (reusableBuffer.moduleSetup.pxx2.registerStep < REGISTER_RX_NAME_RECEIVED) {
      lcdDrawText(REGISTER_COLUMN_1, REGISTER_TOP + 8 + 2 * FH, STR_WAITING);
      lcdDrawText(REGISTER_COLUMN_1, REGISTER_TOP + 8 + 4 * FH, TR_EXIT, menuVerticalPosition == ITEM_REGISTER_BUTTONS ? INVERS : 0);
    }
    else {
      lcdDrawText(REGISTER_COLUMN_1, REGISTER_TOP + 8 + 2 * FH, STR_RX_NAME);
      editName(REGISTER_COLUMN_2, REGISTER_TOP + 8 + 2 * FH, reusableBuffer.moduleSetup.pxx2.registerRxName, PXX2_LEN_RX_NAME, event, menuVerticalPosition == ITEM_REGISTER_RECEIVER_NAME);
      lcdDrawText(REGISTER_COLUMN_1, REGISTER_TOP + 8 + 4 * FH, TR_ENTER, menuVerticalPosition == ITEM_REGISTER_BUTTONS && menuHorizontalPosition == 0 ? INVERS : 0);
      lcdDrawText(REGISTER_COLUMN_2, REGISTER_TOP + 8 + 4 * FH, TR_EXIT, menuVerticalPosition == ITEM_REGISTER_BUTTONS && menuHorizontalPosition == 1 ? INVERS : 0);
    }

    reusableBuffer.moduleSetup.pxx2.registerPopupVerticalPosition = menuVerticalPosition;
    reusableBuffer.moduleSetup.pxx2.registerPopupHorizontalPosition = menuHorizontalPosition;
    reusableBuffer.moduleSetup.pxx2.registerPopupEditMode = s_editMode;
  }

  menuVerticalPosition = backupVerticalPosition;
  menuHorizontalPosition = backupHorizontalPosition;
  menuVerticalOffset = backupVerticalOffset;
  s_editMode = backupEditMode;
}

void startRegisterDialog(uint8_t module)
{
  memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
  moduleState[module].mode = MODULE_MODE_REGISTER;
  s_editMode = 0;
  POPUP_INPUT("", runPopupRegister);
}

void checkModelIdUnique(uint8_t moduleIdx)
{
  if (isModuleXJTD8(moduleIdx))
    return;

  char * warn_buf = reusableBuffer.moduleSetup.msg;

  // cannot rely exactly on WARNING_LINE_LEN so using WARNING_LINE_LEN-2
  size_t warn_buf_len = sizeof(reusableBuffer.moduleSetup.msg) - WARNING_LINE_LEN - 2;
  if (!modelslist.isModelIdUnique(moduleIdx,warn_buf,warn_buf_len)) {
    if (warn_buf[0] != 0) {
      POPUP_WARNING(STR_MODELIDUSED);
      SET_WARNING_INFO(warn_buf, sizeof(reusableBuffer.moduleSetup.msg), 0);
    }
  }
}

void onModelSetupBitmapMenu(const char * result)
{
  if (result == STR_UPDATE_LIST) {
    if (!sdListFiles(BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap)-LEN_BITMAPS_EXT, NULL)) {
      POPUP_WARNING(STR_NO_BITMAPS_ON_SD);
    }
  }
  else if (result != STR_EXIT) {
    // The user choosed a bmp file in the list
    copySelection(g_model.header.bitmap, result, sizeof(g_model.header.bitmap));
    storageDirty(EE_MODEL);
    if (modelslist.getCurrentModel())
      modelslist.getCurrentModel()->resetBuffer();
  }
}

void editTimerMode(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData & timer = g_model.timers[timerIdx];
  if (attr && menuHorizontalPosition < 0) {
    lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, 115+2*INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
  }
  drawStringWithIndex(MENUS_MARGIN_LEFT, y, STR_TIMER, timerIdx+1);
  drawTimerMode(MODEL_SETUP_2ND_COLUMN, y, timer.mode, (menuHorizontalPosition<=0 ? attr : 0));
  drawTimer(MODEL_SETUP_2ND_COLUMN+50, y, timer.start, (menuHorizontalPosition!=0 ? attr|TIMEHOUR : TIMEHOUR));
  if (attr && s_editMode>0) {
    switch (menuHorizontalPosition) {
      case 0:
      {
        int32_t timerMode = timer.mode;
        if (timerMode < 0) timerMode -= TMRMODE_COUNT-1;
        CHECK_INCDEC_MODELVAR_CHECK(event, timerMode, -TMRMODE_COUNT-SWSRC_LAST+1, TMRMODE_COUNT+SWSRC_LAST-1, isSwitchAvailableInTimers);
        if (timerMode < 0) timerMode += TMRMODE_COUNT-1;
        timer.mode = timerMode;
#if defined(AUTOSWITCH)
        if (s_editMode>0) {
          int8_t val = timer.mode - (TMRMODE_COUNT-1);
          int8_t switchVal = checkIncDecMovedSwitch(val);
          if (val != switchVal) {
            timer.mode = switchVal + (TMRMODE_COUNT-1);
            storageDirty(EE_MODEL);
          }
        }
#endif
        break;
      }
      case 1:
      {
        const int stopsMinutes[] = { 8, 60, 120, 180, 240, 300, 600, 900, 1200 };
        timer.start = checkIncDec(event, timer.start, 0, TIMER_MAX, EE_MODEL, NULL, (const CheckIncDecStops&)stopsMinutes);
        break;
      }
    }
  }
}

void editTimerCountdown(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData & timer = g_model.timers[timerIdx];
  lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BEEPCOUNTDOWN);
  lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VBEEPCOUNTDOWN, timer.countdownBeep, (menuHorizontalPosition == 0 ? attr : 0));
  if (timer.countdownBeep != COUNTDOWN_SILENT) {
    lcdDrawNumber(MODEL_SETUP_3RD_COLUMN, y, TIMER_COUNTDOWN_START(timerIdx), (menuHorizontalPosition == 1 ? attr : 0) | LEFT, 0, NULL, "s");
  }
  if (attr && s_editMode > 0) {
    switch (menuHorizontalPosition) {
      case 0:
        CHECK_INCDEC_MODELVAR(event, timer.countdownBeep, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1);
        break;
      case 1:
        timer.countdownStart = -checkIncDecModel(event, -timer.countdownStart, -1, +2);
        break;
    }
  }
}

int getSwitchWarningsCount()
{
  int count = 0;
  for (int i=0; i<NUM_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      ++count;
    }
  }
  return count;
}

#define IF_INTERNAL_MODULE_ON(x)             (IS_INTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)
#define IF_EXTERNAL_MODULE_ON(x)             (IS_EXTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)

#define IF_PXX2_MODULE(module, xxx)          (isModulePXX2(module) ? (uint8_t)(xxx) : HIDDEN_ROW)
#define IF_NOT_PXX2_MODULE(module, xxx)      (isModulePXX2(module) ? HIDDEN_ROW : (uint8_t)(xxx))
#define IF_ACCESS_MODULE_RF(module, xxx)     (isModuleRFAccess(module) ? (uint8_t)(xxx) : HIDDEN_ROW)
#define IF_NOT_ACCESS_MODULE_RF(module, xxx) (isModuleRFAccess(module) ? HIDDEN_ROW : (uint8_t)(xxx))

#if defined(INTERNAL_MODULE_PXX1)
#define INTERNAL_MODULE_TYPE_ROWS            ((isModuleXJT(INTERNAL_MODULE) || isModulePXX2(INTERNAL_MODULE)) ? (uint8_t)1 : (uint8_t)0) // Module type + RF protocols
#else
#define INTERNAL_MODULE_TYPE_ROWS            0 // Module type + RF protocols
#endif

#define PORT_CHANNELS_ROWS(x)                (x==INTERNAL_MODULE ? INTERNAL_MODULE_CHANNELS_ROWS : (x==EXTERNAL_MODULE ? EXTERNAL_MODULE_CHANNELS_ROWS : 1))

#define TIMER_ROWS(x)                        NAVIGATION_LINE_BY_LINE|1, 0, 0, 0, g_model.timers[x].countdownBeep != COUNTDOWN_SILENT ? (uint8_t)1 : (uint8_t)0

#define EXTERNAL_MODULE_MODE_ROWS            (isModuleXJT(EXTERNAL_MODULE) || isModuleR9MNonAccess(EXTERNAL_MODULE) || isModuleDSM2(EXTERNAL_MODULE) || isModuleMultimodule(EXTERNAL_MODULE)) ? (uint8_t)1 : (uint8_t)0

#if TIMERS == 1
#define TIMERS_ROWS                          TIMER_ROWS(0)
#elif TIMERS == 2
#define TIMERS_ROWS                          TIMER_ROWS(0), TIMER_ROWS(1)
#elif TIMERS == 3
#define TIMERS_ROWS                          TIMER_ROWS(0), TIMER_ROWS(1), TIMER_ROWS(2)
#endif

#define TRAINER_CHANNELS_ROW                 (g_model.trainerData.mode == TRAINER_MODE_SLAVE ? (uint8_t)1 : (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH ? (uint8_t)0 : HIDDEN_ROW))
#define TRAINER_PPM_PARAMS_ROW               (g_model.trainerData.mode == TRAINER_MODE_SLAVE ? (uint8_t)2 : HIDDEN_ROW)
#define IF_BT_TRAINER_ON(x)                  (g_eeGeneral.bluetoothMode == BLUETOOTH_TRAINER ? (uint8_t)(x) : HIDDEN_ROW)
#define TRAINER_BLUETOOTH_M_ROW              ((bluetooth.distantAddr[0] == '\0' || bluetooth.state == BLUETOOTH_STATE_CONNECTED) ? (uint8_t)0 : (uint8_t)1)
#define TRAINER_BLUETOOTH_S_ROW              (bluetooth.distantAddr[0] == '\0' ? HIDDEN_ROW : LABEL())
#define TRAINER_BLUETOOTH_ROW                (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH ? TRAINER_BLUETOOTH_M_ROW : (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH ? TRAINER_BLUETOOTH_S_ROW : HIDDEN_ROW))
#define TRAINER_ROWS                         LABEL(Trainer), 0, IF_BT_TRAINER_ON(TRAINER_BLUETOOTH_ROW), TRAINER_CHANNELS_ROW, TRAINER_PPM_PARAMS_ROW

#if defined(PXX2)
#define REGISTRATION_ID_ROWS                 uint8_t((isDefaultModelRegistrationID() || (warningText && popupFunc == runPopupRegister)) ? HIDDEN_ROW : READONLY_ROW),
#else
#define REGISTRATION_ID_ROWS
#endif

void drawReceiverName(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t receiverIdx, LcdFlags flags)
{
  if (isModulePXX2(moduleIdx)) {
    if (g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx][0] != '\0')
      lcdDrawSizedText(x, y, g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], effectiveLen(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME), flags);
    else
      lcdDrawText(x, y, "---", flags);
  }
#if defined(HARDWARE_INTERNAL_MODULE)
  else if (moduleIdx == INTERNAL_MODULE) {
    lcdDrawText(x, y, "Internal", flags);
  }
#endif
  else {
    lcdDrawText(x, y, "External", flags);
  }
}

#if defined(EXTERNAL_ANTENNA)
#define ANTENNA_ROW IF_NOT_PXX2_MODULE(INTERNAL_MODULE, IF_INTERNAL_MODULE_ON(0)), // Antenna
void onAntennaSwitchConfirm(const char * result)
{
  if (result == STR_OK) {
    // Switch to external antenna confirmation
    g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = XJT_EXTERNAL_ANTENNA;
  }
}
#else
#define ANTENNA_ROW
#endif

bool menuModelSetup(event_t event)
{
  bool CURSOR_ON_CELL = (menuHorizontalPosition >= 0);

  int8_t old_editMode = s_editMode;
  MENU(STR_MENUSETUP, MODEL_ICONS, menuTabModel, MENU_MODEL_SETUP, ITEM_MODEL_SETUP_MAX,
       { 0, // Model name 
         0, // Model bitmap 
         TIMERS_ROWS,
         0, // Extended limits
         1, // Extended trims
         0, // Show trims
         0, // Trims step
         
         LABEL(Throttle), 
           0, // Throttle reverse
           0, // Throttle trace source
           0, // Throttle trim
           
         LABEL(PreflightCheck), 
           0, // Display checklist
           0, // Throttle state
           uint8_t(NAVIGATION_LINE_BY_LINE|(getSwitchWarningsCount()-1)), // Switches warnings
           0, // Pots & Sliders warning mode
           g_model.potsWarnMode ? uint8_t(NAVIGATION_LINE_BY_LINE|(NUM_POTS-1)) : HIDDEN_ROW, // Pots positions
           g_model.potsWarnMode ? uint8_t(NAVIGATION_LINE_BY_LINE|(NUM_SLIDERS-1)) : HIDDEN_ROW, // Sliders positions

         NAVIGATION_LINE_BY_LINE|(NUM_STICKS+NUM_POTS+NUM_SLIDERS-1), // Center beeps
         0, // Global functions

         REGISTRATION_ID_ROWS

         LABEL(InternalModule),
           INTERNAL_MODULE_TYPE_ROWS,
           INTERNAL_MODULE_CHANNELS_ROWS,
           IF_NOT_ACCESS_MODULE_RF(INTERNAL_MODULE, IF_INTERNAL_MODULE_ON(IF_INTERNAL_MODULE_ON(isModuleModelIndexAvailable(INTERNAL_MODULE) ? (uint8_t)2 : (uint8_t)1))),
           IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0), // RxNum
           ANTENNA_ROW
           IF_INTERNAL_MODULE_ON(FAILSAFE_ROWS(INTERNAL_MODULE)),
           IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 1), /* Range check and Register buttons */ \
           IF_PXX2_MODULE(INTERNAL_MODULE, 0), /* Module options */ \
           IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0), /* Receiver 1 */ \
           IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0), /* Receiver 2 */  \
           IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0), /* Receiver 3 */

         LABEL(ExternalModule),
           EXTERNAL_MODULE_MODE_ROWS,
           EXTERNAL_MODULE_POWER_ROW,
           MULTIMODULE_STATUS_ROWS
           EXTERNAL_MODULE_CHANNELS_ROWS,
           EXTERNAL_MODULE_BIND_ROWS,
           FAILSAFE_ROWS(EXTERNAL_MODULE),
           EXTERNAL_MODULE_OPTION_ROW,
           MULTIMODULE_MODULE_ROWS

         TRAINER_ROWS
       });

  if (event == EVT_ENTRY) {
    reusableBuffer.moduleSetup.r9mPower = g_model.moduleData[EXTERNAL_MODULE].pxx.power;
  }

  if (menuEvent) {
    moduleState[0].mode = 0;
    moduleState[1].mode = 0;
  }

  int sub = menuVerticalPosition;

  for (uint8_t i=0; i<NUM_BODY_LINES; ++i) {
    coord_t y = MENU_CONTENT_TOP + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j] == HIDDEN_ROW)
        k++;
    }

    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch(k) {
      case ITEM_MODEL_SETUP_NAME:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MODELNAME);
        editName(MODEL_SETUP_2ND_COLUMN, y, g_model.header.name, sizeof(g_model.header.name), event, attr);
        break;

      case ITEM_MODEL_SETUP_BITMAP:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BITMAP);
        if (ZEXIST(g_model.header.bitmap))
          lcdDrawSizedText(MODEL_SETUP_2ND_COLUMN, y, g_model.header.bitmap, sizeof(g_model.header.bitmap), attr);
        else
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VCSWFUNC, 0, attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER) && READ_ONLY_UNLOCKED()) {
          s_editMode = 0;
          if (sdListFiles(BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap)-LEN_BITMAPS_EXT, g_model.header.bitmap, LIST_NONE_SD_FILE | LIST_SD_FILE_EXT)) {
            POPUP_MENU_START(onModelSetupBitmapMenu);
          }
          else {
            POPUP_WARNING(STR_NO_BITMAPS_ON_SD);
          }
        }
        break;

      case ITEM_MODEL_SETUP_TIMER1:
        editTimerMode(0, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER1_NAME:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_NAME);
        editName(MODEL_SETUP_2ND_COLUMN, y, g_model.timers[0].name, LEN_TIMER_NAME, event, attr);
        break;

      case ITEM_MODEL_SETUP_TIMER1_MINUTE_BEEP:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_MINUTEBEEP);
        g_model.timers[0].minuteBeep = editCheckBox(g_model.timers[0].minuteBeep, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER1_COUNTDOWN_BEEP:
        editTimerCountdown(0, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER1_PERSISTENT:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PERSISTENT);
        g_model.timers[0].persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_VPERSISTENT, g_model.timers[0].persistent, 0, 2, attr, event);
        break;

#if TIMERS > 1
      case ITEM_MODEL_SETUP_TIMER2:
        editTimerMode(1, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER2_NAME:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_NAME);
        editName(MODEL_SETUP_2ND_COLUMN, y, g_model.timers[1].name, LEN_TIMER_NAME, event, attr);
        break;

      case ITEM_MODEL_SETUP_TIMER2_MINUTE_BEEP:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_MINUTEBEEP);
        g_model.timers[1].minuteBeep = editCheckBox(g_model.timers[1].minuteBeep, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER2_COUNTDOWN_BEEP:
        editTimerCountdown(1, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER2_PERSISTENT:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PERSISTENT);
        g_model.timers[1].persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_VPERSISTENT, g_model.timers[1].persistent, 0, 2, attr, event);
        break;
#endif

#if TIMERS > 2
      case ITEM_MODEL_SETUP_TIMER3:
        editTimerMode(2, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER3_NAME:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_NAME);
        editName(MODEL_SETUP_2ND_COLUMN, y, g_model.timers[2].name, LEN_TIMER_NAME, event, attr);
        break;

      case ITEM_MODEL_SETUP_TIMER3_MINUTE_BEEP:
        lcdDrawText(MENUS_MARGIN_LEFT, y, INDENT TR_MINUTEBEEP);
        g_model.timers[2].minuteBeep = editCheckBox(g_model.timers[2].minuteBeep, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER3_COUNTDOWN_BEEP:
        editTimerCountdown(2, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_TIMER3_PERSISTENT:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PERSISTENT);
        g_model.timers[2].persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_VPERSISTENT, g_model.timers[2].persistent, 0, 2, attr, event);
        break;
#endif

      case ITEM_MODEL_SETUP_EXTENDED_LIMITS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_ELIMITS);
        g_model.extendedLimits = editCheckBox(g_model.extendedLimits, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_EXTENDED_TRIMS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_ETRIMS);
        g_model.extendedTrims = editCheckBox(g_model.extendedTrims, MODEL_SETUP_2ND_COLUMN, y, menuHorizontalPosition<=0 ? attr : 0, event==EVT_KEY_BREAK(KEY_ENTER) ? event : 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+18, y, STR_RESET_BTN, menuHorizontalPosition>0  && !NO_HIGHLIGHT() ? attr : 0);
        if (attr && menuHorizontalPosition>0) {
          s_editMode = 0;
          if (event==EVT_KEY_LONG(KEY_ENTER)) {
            START_NO_HIGHLIGHT();
            for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
              memclear(&g_model.flightModeData[i], TRIMS_ARRAY_SIZE);
            }
            storageDirty(EE_MODEL);
            AUDIO_WARNING1();
          }
        }
        break;

      case ITEM_MODEL_SETUP_DISPLAY_TRIMS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_DISPLAY_TRIMS);
        g_model.displayTrims = editChoice(MODEL_SETUP_2ND_COLUMN, y, "\006No\0   ChangeYes", g_model.displayTrims, 0, 2, attr, event);
        break;

      case ITEM_MODEL_SETUP_TRIM_INC:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TRIMINC);
        g_model.trimInc = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_VTRIMINC, g_model.trimInc, -2, 2, attr, event);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_THROTTLE_LABEL);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_REVERSED:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_THROTTLEREVERSE);
        g_model.throttleReversed = editCheckBox(g_model.throttleReversed, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_TRACE:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TTRACE);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, g_model.thrTraceSrc, NUM_POTS+NUM_SLIDERS+MAX_OUTPUT_CHANNELS);
        uint8_t idx = g_model.thrTraceSrc + MIXSRC_Thr;
        if (idx > MIXSRC_Thr)
          idx += 1;
        if (idx >= MIXSRC_FIRST_POT+NUM_POTS+NUM_SLIDERS)
          idx += MIXSRC_CH1 - MIXSRC_FIRST_POT - NUM_POTS - NUM_SLIDERS;
        drawSource(MODEL_SETUP_2ND_COLUMN, y, idx, attr);
        break;
      }

      case ITEM_MODEL_SETUP_THROTTLE_TRIM:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TTRIM);
        g_model.thrTrim = editCheckBox(g_model.thrTrim, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_PREFLIGHT_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PREFLIGHT);
        break;

      case ITEM_MODEL_SETUP_CHECKLIST_DISPLAY:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_CHECKLIST);
        g_model.displayChecklist = editCheckBox(g_model.displayChecklist, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_WARNING:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_THROTTLEWARNING);
        g_model.disableThrottleWarning = !editCheckBox(!g_model.disableThrottleWarning, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        break;

      case ITEM_MODEL_SETUP_SWITCHES_WARNING:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_SWITCHWARNING);
        if (!READ_ONLY() && attr && menuHorizontalPosition<0 && event==EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(event);
          START_NO_HIGHLIGHT();
          getMovedSwitch();
          for (int i=0; i<NUM_SWITCHES; i++) {
            bool enabled = ((g_model.switchWarningState >> (3*i)) & 0x07);
            if (enabled) {
              g_model.switchWarningState &= ~(0x07 << (3*i));
              unsigned int newState = (switches_states >> (2*i) & 0x03) + 1;
              g_model.switchWarningState |= (newState << (3*i));
            }
          }
          AUDIO_WARNING1();
          storageDirty(EE_MODEL);
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, (NUM_SWITCHES-1)*25+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        unsigned int newStates = 0;
        for (int i=0, current=0; i<NUM_SWITCHES; i++) {
          if (SWITCH_WARNING_ALLOWED(i)) {
            unsigned int state = ((g_model.switchWarningState >> (3*i)) & 0x07);
            LcdFlags color = (state > 0 ? TEXT_COLOR : TEXT_DISABLE_COLOR);
            if (attr && menuHorizontalPosition < 0) {
              color |= INVERS;
            }
            char s[3];
            s[0] = 'A' + i;
            s[1] = "x\300-\301"[state];
            s[2] = '\0';
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+i*25, y, s, color|(menuHorizontalPosition==current ? attr : 0));
            if (!READ_ONLY() && attr && menuHorizontalPosition==current) {
              CHECK_INCDEC_MODELVAR_ZERO_CHECK(event, state, 3, IS_CONFIG_3POS(i) ? NULL : isSwitch2POSWarningStateAvailable);
            }
            newStates |= (state << (3*i));
            ++current;
          }
        }
        g_model.switchWarningState = newStates;
        break;
      }

      case ITEM_MODEL_SETUP_POTS_SLIDERS_WARNING_STATE:
        lcdDrawText(MENUS_MARGIN_LEFT, y,STR_POTWARNINGSTATE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, "\004""OFF\0""Man\0""Auto", g_model.potsWarnMode, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR(event, g_model.potsWarnMode, POTS_WARN_OFF, POTS_WARN_AUTO);
          storageDirty(EE_MODEL);
        }
        break;

      case ITEM_MODEL_SETUP_POTS_WARNING:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_POTWARNING);
        if (attr) {
          if (!READ_ONLY() && menuHorizontalPosition >= 0 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
              SAVE_POT_POSITION(menuHorizontalPosition);
              AUDIO_WARNING1();
              storageDirty(EE_MODEL);
            }
          }

          if (!READ_ONLY() &&  menuHorizontalPosition >= 0 && s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition));
            storageDirty(EE_MODEL);
          }
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, NUM_POTS*MODEL_SETUP_SLIDPOT_SPACING+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN;
          for (int i=0; i<NUM_POTS; ++i) {
            LcdFlags flags = (((menuHorizontalPosition==i) && attr) ? INVERS : 0);
            flags |= (g_model.potsWarnEnabled & (1 << i)) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
            if (attr && menuHorizontalPosition < 0) {
              flags |= INVERS;
            }
            lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS+1+i, flags);
            x += MODEL_SETUP_SLIDPOT_SPACING;
          }
        }
        break;
      }

      case ITEM_MODEL_SETUP_SLIDERS_WARNING:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_SLIDERWARNING);
        if (attr) {
          if (!READ_ONLY() && menuHorizontalPosition+1 && event==EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
              SAVE_POT_POSITION(menuHorizontalPosition+NUM_POTS);
              AUDIO_WARNING1();
              storageDirty(EE_MODEL);
            }
          }

          if (!READ_ONLY() && menuHorizontalPosition+1 && s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition+NUM_POTS));
            storageDirty(EE_MODEL);
          }
        }

        if (attr && menuHorizontalPosition < 0) {
          lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN-INVERT_HORZ_MARGIN, y-INVERT_VERT_MARGIN+1, NUM_SLIDERS*MODEL_SETUP_SLIDPOT_SPACING+INVERT_HORZ_MARGIN, INVERT_LINE_HEIGHT, TEXT_INVERTED_BGCOLOR);
        }

        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN;
          for (int i=NUM_POTS; i<NUM_POTS+NUM_SLIDERS; ++i) {
            LcdFlags flags = (((menuHorizontalPosition==i-NUM_POTS) && attr) ? INVERS : 0);
            flags |= (g_model.potsWarnEnabled & (1 << i)) ? TEXT_DISABLE_COLOR : TEXT_COLOR;
            if (attr && menuHorizontalPosition < 0) {
              flags |= INVERS;
            }
            lcdDrawTextAtIndex(x, y, STR_VSRCRAW, NUM_STICKS+1+i, flags);
            x += MODEL_SETUP_SLIDPOT_SPACING;
          }
        }
        break;

      case ITEM_MODEL_SETUP_BEEP_CENTER:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_BEEPCTR);
        lcdNextPos = MODEL_SETUP_2ND_COLUMN - 3;
        for (int i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
          LcdFlags flags = ((menuHorizontalPosition==i && attr) ? INVERS : 0);
          flags |= (g_model.beepANACenter & ((BeepANACenter)1<<i)) ? TEXT_COLOR : (TEXT_DISABLE_COLOR | NO_FONTCACHE);
          if (attr && menuHorizontalPosition < 0) flags |= INVERS;
          lcdDrawTextAtIndex(lcdNextPos+3, y, STR_RETA123, i, flags);
        }
        if (attr && CURSOR_ON_CELL) {
          if (event==EVT_KEY_BREAK(KEY_ENTER)) {
            if (READ_ONLY_UNLOCKED()) {
              s_editMode = 0;
              g_model.beepANACenter ^= ((BeepANACenter)1<<menuHorizontalPosition);
              storageDirty(EE_MODEL);
            }
          }
        }
        break;

      case ITEM_MODEL_SETUP_USE_GLOBAL_FUNCTIONS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_USE_GLOBAL_FUNCS);
        drawCheckBox(MODEL_SETUP_2ND_COLUMN, y, !g_model.noGlobalFunctions, attr);
        if (attr) g_model.noGlobalFunctions = !checkIncDecModel(event, !g_model.noGlobalFunctions, 0, 1);
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_INTERNALRF);
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_TYPE:
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_INTERNAL_MODULE_PROTOCOLS, g_model.moduleData[INTERNAL_MODULE].type, menuHorizontalPosition==0 ? attr : 0);
        if (isModuleXJT(INTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_XJT_ACCST_RF_PROTOCOLS, 1+g_model.moduleData[INTERNAL_MODULE].subType, menuHorizontalPosition==1 ? attr : 0);
        else if (isModuleISRM(INTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_ISRM_RF_PROTOCOLS, g_model.moduleData[INTERNAL_MODULE].subType, menuHorizontalPosition==1 ? attr : 0);
        if (attr) {
          if (menuHorizontalPosition == 0) {
            uint8_t moduleType = checkIncDec(event, g_model.moduleData[INTERNAL_MODULE].type, MODULE_TYPE_NONE, MODULE_TYPE_MAX, EE_MODEL, isInternalModuleAvailable);
            if (checkIncDec_Ret) {
              // TODO this code should be common, in module.h (X10_new_UI branch)
              memclear(&g_model.moduleData[INTERNAL_MODULE], sizeof(ModuleData));
              g_model.moduleData[INTERNAL_MODULE].type = moduleType;
              g_model.moduleData[INTERNAL_MODULE].channelsCount = defaultModuleChannels_M8(INTERNAL_MODULE);
            }
          }
          else if (isModuleXJT(INTERNAL_MODULE)) {
            g_model.moduleData[INTERNAL_MODULE].rfProtocol = checkIncDec(event, g_model.moduleData[INTERNAL_MODULE].subType, 0, MODULE_SUBTYPE_PXX1_LAST, EE_MODEL, isRfProtocolAvailable);
            if (checkIncDec_Ret) {
              g_model.moduleData[0].type = MODULE_TYPE_XJT_PXX1;
              g_model.moduleData[0].channelsStart = 0;
              g_model.moduleData[0].channelsCount = defaultModuleChannels_M8(INTERNAL_MODULE);
            }
          }
          else if (isModulePXX2(INTERNAL_MODULE)) {
            g_model.moduleData[INTERNAL_MODULE].subType = checkIncDec(event, g_model.moduleData[INTERNAL_MODULE].subType, 0, MODULE_SUBTYPE_ISRM_PXX2_ACCST_LR12, EE_MODEL, isRfProtocolAvailable);
          }
        }
        break;

#if defined(EXTERNAL_ANTENNA)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_ANTENNA:
      {
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_ANTENNASELECTION);
        uint8_t newAntennaSel = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_VANTENNATYPES, g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna, 0, 1, attr, event);
        if (newAntennaSel != g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna && newAntennaSel == XJT_EXTERNAL_ANTENNA) {
          POPUP_CONFIRMATION(STR_ANTENNACONFIRM1, onAntennaSwitchConfirm);
          const char * w = STR_ANTENNACONFIRM2;
          SET_WARNING_INFO(w, strlen(w), 0);
        }
        else {
          g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = newAntennaSel;
        }
        break;
      }
#endif

      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_EXTERNALRF);
        break;

      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_TYPE:
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_EXTERNAL_MODULE_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].type, menuHorizontalPosition==0 ? attr : 0);
        if (isModuleXJT(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_XJT_ACCST_RF_PROTOCOLS, 1+g_model.moduleData[EXTERNAL_MODULE].subType, (menuHorizontalPosition==1 ? attr : 0));
        else if (isModuleDSM2(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_DSM_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, (menuHorizontalPosition==1 ? attr : 0));
        else if (isModuleR9MNonAccess(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_R9M_REGION, g_model.moduleData[EXTERNAL_MODULE].subType, (menuHorizontalPosition==1 ? attr : 0));
#if defined(MULTIMODULE)
      else if (isModuleMultimodule(EXTERNAL_MODULE)) {
          int multi_rfProto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
          if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto) {
            lcdDrawText(MODEL_SETUP_3RD_COLUMN, y, STR_MULTI_CUSTOM, menuHorizontalPosition == 1 ? attr : 0);
            lcdDrawNumber(MODEL_SETUP_4TH_COLUMN, y, multi_rfProto, menuHorizontalPosition==2 ? attr : 0, 2);
            lcdDrawNumber(MODEL_SETUP_4TH_COLUMN + MODEL_SETUP_BIND_OFS, y, g_model.moduleData[EXTERNAL_MODULE].subType, menuHorizontalPosition==3 ? attr : 0, 2);
          }
          else {
            const mm_protocol_definition * pdef = getMultiProtocolDefinition(multi_rfProto);
            lcdDrawTextAtIndex(MODEL_SETUP_3RD_COLUMN, y, STR_MULTI_PROTOCOLS, multi_rfProto, menuHorizontalPosition == 1 ? attr : 0);
            if (pdef->subTypeString != nullptr)
              lcdDrawTextAtIndex(MODEL_SETUP_4TH_COLUMN, y, pdef->subTypeString, g_model.moduleData[EXTERNAL_MODULE].subType, menuHorizontalPosition==2 ? attr : 0);
          }
        }
#endif
        if (attr) {
          if (s_editMode > 0) {
            switch (menuHorizontalPosition) {
              case 0:
                g_model.moduleData[EXTERNAL_MODULE].type = checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].type,
                                                                       MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1, EE_MODEL,
                                                                       isExternalModuleAvailable);
                if (checkIncDec_Ret) {
                  g_model.moduleData[EXTERNAL_MODULE].rfProtocol = 0;
                  g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                  g_model.moduleData[EXTERNAL_MODULE].channelsCount = defaultModuleChannels_M8(EXTERNAL_MODULE);
                  if (isModuleSBUS(EXTERNAL_MODULE))
                    g_model.moduleData[EXTERNAL_MODULE].sbus.refreshRate = -31;
                  if (isModulePPM(EXTERNAL_MODULE))
                    SET_DEFAULT_PPM_FRAME_LENGTH(EXTERNAL_MODULE);
                }
                break;
              case 1:
                if (isModuleDSM2(EXTERNAL_MODULE))
                  CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, DSM2_PROTO_LP45, DSM2_PROTO_DSMX);
#if defined(MULTIMODULE)
                  else if (isModuleMultimodule(EXTERNAL_MODULE)) {
                  int multiRfProto = g_model.moduleData[EXTERNAL_MODULE].multi.customProto == 1 ? MODULE_SUBTYPE_MULTI_CUSTOM : g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
                  CHECK_INCDEC_MODELVAR(event, multiRfProto, MODULE_SUBTYPE_MULTI_FIRST, MODULE_SUBTYPE_MULTI_LAST);
                  if (checkIncDec_Ret) {
                    g_model.moduleData[EXTERNAL_MODULE].multi.customProto = (multiRfProto == MODULE_SUBTYPE_MULTI_CUSTOM);
                    if (!g_model.moduleData[EXTERNAL_MODULE].multi.customProto)
                      g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(multiRfProto);
                    g_model.moduleData[EXTERNAL_MODULE].subType = 0;
                    // Sensible default for DSM2 (same as for ppm): 7ch@22ms + Autodetect settings enabled
                    if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MODULE_SUBTYPE_MULTI_DSM2) {
                      g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 1;
                    }
                    else {
                      g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 0;
                    }
                    g_model.moduleData[EXTERNAL_MODULE].multi.optionValue = 0;
                  }
                }
#endif
                else if (isModuleR9MNonAccess(EXTERNAL_MODULE)) {
                  g_model.moduleData[EXTERNAL_MODULE].subType = checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].subType, MODULE_SUBTYPE_R9M_FCC,
                                                                            MODULE_SUBTYPE_R9M_LAST, EE_MODEL, isR9MModeAvailable);
                }
                else {
                  CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, MODULE_SUBTYPE_PXX1_ACCST_D16, MODULE_SUBTYPE_PXX1_LAST);
                }
                if (checkIncDec_Ret) {
                  g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                  g_model.moduleData[EXTERNAL_MODULE].channelsCount = defaultModuleChannels_M8(EXTERNAL_MODULE);
                }
                break;
#if defined(MULTIMODULE)
              case 2:
                if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto) {
                  g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), 0, 63, EE_MODEL));
                  break;
                } else {
                  const mm_protocol_definition *pdef = getMultiProtocolDefinition(g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false));
                  if (pdef->maxSubtype > 0)
                    CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, pdef->maxSubtype);
                }
                break;
              case 3:
                // Custom protocol, third column is subtype
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, 7);
                break;
#endif
            }
          }
#if POPUP_LEVEL > 1
          else if (old_editMode > 0) {
            if (isModuleR9MNonAccess(EXTERNAL_MODULE)) {
              if (g_model.moduleData[EXTERNAL_MODULE].subType > MODULE_SUBTYPE_R9M_EU) {
                POPUP_WARNING(STR_R9M_PROTO_FLEX_WARN_LINE1);
                SET_WARNING_INFO(STR_R9M_PROTO_WARN_LINE2, sizeof(TR_R9M_PROTO_WARN_LINE2) - 1, 0);
              }
#if POPUP_LEVEL >= 3
              else if (g_model.moduleData[EXTERNAL_MODULE].subType == MODULE_SUBTYPE_R9M_EU) {
                POPUP_WARNING(STR_R9M_PROTO_EU_WARN_LINE1);
                SET_WARNING_INFO(STR_R9M_PROTO_WARN_LINE2, sizeof(TR_R9M_PROTO_WARN_LINE2) - 1, 0);
              }
              else {
                POPUP_WARNING(STR_R9M_PROTO_FCC_WARN_LINE1);
                SET_WARNING_INFO(STR_R9M_PROTO_WARN_LINE2, sizeof(TR_R9M_PROTO_WARN_LINE2) - 1, 0);
              }
#endif
            }
          }
#endif          
        }
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_CHANNELS:
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_CHANNELS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_CHANNELRANGE);
        if ((int8_t)PORT_CHANNELS_ROWS(moduleIdx) >= 0) {
          drawStringWithIndex(MODEL_SETUP_2ND_COLUMN, y, STR_CH, moduleData.channelsStart+1, menuHorizontalPosition==0 ? attr : 0);
          lcdDrawText(lcdNextPos+5, y, "-");
          drawStringWithIndex(lcdNextPos+5, y, STR_CH, moduleData.channelsStart+sentModuleChannels(moduleIdx), menuHorizontalPosition==1 ? attr : 0);
          const char * delay = getModuleDelay(moduleIdx);
          if (delay)
            lcdDrawText(lcdNextPos + 15, y, delay);
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.channelsStart, 32-8-moduleData.channelsCount);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.channelsCount, -4, min<int8_t>(maxModuleChannels_M8(moduleIdx), 32-8-moduleData.channelsStart));
                if (k == ITEM_MODEL_SETUP_EXTERNAL_MODULE_CHANNELS && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM)
                  SET_DEFAULT_PPM_FRAME_LENGTH(moduleIdx);
                break;
            }
          }
        }
        break;
      }

      case ITEM_MODEL_SETUP_TRAINER_LABEL:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_TRAINER);
        break;

      case ITEM_MODEL_SETUP_TRAINER_MODE:
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_MODE);
        g_model.trainerData.mode = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_VTRAINERMODES, g_model.trainerData.mode, 0, TRAINER_MODE_MAX(), attr, event);
        if (attr && checkIncDec_Ret) {
          bluetooth.state = BLUETOOTH_STATE_OFF;
          bluetooth.distantAddr[0] = 0;
        }
        break;

      case ITEM_MODEL_SETUP_TRAINER_BLUETOOTH:
        if (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH) {
          if (attr) {
            s_editMode = 0;
          }
          if (bluetooth.distantAddr[0]) {
            lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, bluetooth.distantAddr);
            if (bluetooth.state != BLUETOOTH_STATE_CONNECTED) {
              drawButton(MODEL_SETUP_2ND_COLUMN, y, "Bind", menuHorizontalPosition == 0 ? attr : 0);
              drawButton(MODEL_SETUP_2ND_COLUMN+60, y, "Clear", menuHorizontalPosition == 1 ? attr : 0);
            }
            else {
              drawButton(MODEL_SETUP_2ND_COLUMN, y, "Clear", attr);
            }
            if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
              if (bluetooth.state == BLUETOOTH_STATE_CONNECTED || menuHorizontalPosition == 1) {
                bluetooth.state = BLUETOOTH_STATE_OFF;
                bluetooth.distantAddr[0] = 0;
              }
              else {
                bluetooth.state = BLUETOOTH_STATE_BIND_REQUESTED;
              }
            }
          }
          else {
            lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, "---");
            if (bluetooth.state < BLUETOOTH_STATE_IDLE)
              drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_BLUETOOTH_INIT, attr);
            else
              drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_BLUETOOTH_DISC, attr);
            if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
              if (bluetooth.state < BLUETOOTH_STATE_IDLE)
                bluetooth.state = BLUETOOTH_STATE_OFF;
              else
                bluetooth.state = BLUETOOTH_STATE_DISCOVER_REQUESTED;
            }
          }
        }
        break;

      case ITEM_MODEL_SETUP_TRAINER_CHANNELS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_CHANNELRANGE);
        drawStringWithIndex(MODEL_SETUP_2ND_COLUMN, y, STR_CH, g_model.trainerData.channelsStart+1, menuHorizontalPosition==0 ? attr : 0);
        lcdDrawText(lcdNextPos+5, y, "-");
        drawStringWithIndex(lcdNextPos+5, y, STR_CH, g_model.trainerData.channelsStart + 8 + g_model.trainerData.channelsCount, menuHorizontalPosition==1 ? attr : 0);
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.trainerData.channelsStart, 32-8-g_model.trainerData.channelsCount);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.trainerData.channelsCount, -4, min<int8_t>(MAX_TRAINER_CHANNELS_M8, 32-8-g_model.trainerData.channelsStart));
              break;
          }
        }
        break;

      case ITEM_MODEL_SETUP_TRAINER_PPM_PARAMS:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PPMFRAME);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)g_model.trainerData.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+80, y, (g_model.trainerData.delay*50)+300, (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr|LEFT : LEFT, 0, NULL, "us");
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+160, y, g_model.trainerData.pulsePol ? "+" : "-", (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
        if (attr && s_editMode>0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.trainerData.frameLength, -20, 35);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.trainerData.delay, -4, 10);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.trainerData.pulsePol, 1);
              break;
          }
        }
        break;

#if defined(PXX2)
      case ITEM_MODEL_SETUP_REGISTRATION_ID:
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_REG_ID);
        if (isDefaultModelRegistrationID())
          lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_PXX2_DEFAULT);
        else
          lcdDrawSizedText(MODEL_SETUP_2ND_COLUMN, y, g_model.modelRegistrationID, PXX2_LEN_REGISTRATION_ID, ZCHAR);
        break;
#endif

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_MODEL_NUM:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_RECEIVER_NUM);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], attr | LEADING0 | LEFT, 2);
        if (attr) {
          CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], getMaxRxNum(moduleIdx));
          if (event == EVT_KEY_LONG(KEY_ENTER)) {
            killEvents(event);
            uint8_t newVal = modelslist.findNextUnusedModelId(moduleIdx);
            if (newVal != g_model.header.modelId[moduleIdx]) {
              g_model.header.modelId[moduleIdx] = newVal;
              storageDirty(EE_MODEL);
            }
          }
        }
      }
      break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_REGISTER_RANGE:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_MODULE);
        drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_REGISTER, (menuHorizontalPosition == 0 ? attr : 0));
        drawButton(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_MODULE_RANGE, (menuHorizontalPosition == 1 ? attr : 0));
        if (attr) {
          if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL && s_editMode > 0) {
            if (menuHorizontalPosition == 0 && event == EVT_KEY_BREAK(KEY_ENTER)) {
              startRegisterDialog(moduleIdx);
            }
            else if (menuHorizontalPosition == 1) {
              moduleState[moduleIdx].mode = MODULE_MODE_RANGECHECK;
            }
          }
          if (s_editMode == 0 && !warningText) {
            moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
          }
          if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
            // REGISTER finished
            s_editMode = 0;
          }
        }
      }
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_OPTIONS:
        lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_OPTIONS);
        drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_SET, attr);
        if (event == EVT_KEY_BREAK(KEY_ENTER) && attr) {
          g_moduleIdx = CURRENT_MODULE_EDITED(k);
          memclear(&reusableBuffer.hardwareAndSettings, sizeof(reusableBuffer.hardwareAndSettings));
          // TODO pushMenu(menuModelModuleOptions);
        }
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1:
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_2:
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_3:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(k);
        ModuleInformation & moduleInformation = reusableBuffer.moduleSetup.pxx2.moduleInformation;

        drawStringWithIndex(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_RECEIVER, receiverIdx + 1);

        if (!(g_model.moduleData[moduleIdx].pxx2.receivers & (1 << receiverIdx))) {
          drawButton(MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_BIND, attr);
          if (attr && s_editMode > 0) {
            s_editMode = 0;
            killEvents(event);
            g_model.moduleData[moduleIdx].pxx2.receivers |= (1 << receiverIdx);
            memclear(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME);
            storageDirty(EE_MODEL);
          }
          else {
            break;
          }
        }

        drawReceiverName(MODEL_SETUP_2ND_COLUMN, y, moduleIdx, receiverIdx, attr);

        if (s_editMode && isModuleR9MAccess(moduleIdx) && moduleState[moduleIdx].mode == MODULE_MODE_NORMAL && moduleInformation.information.modelID) {
          moduleInformation.information.modelID = 0;
          moduleState[moduleIdx].startBind(&reusableBuffer.moduleSetup.bindInformation);
        }

        if (attr && (moduleState[moduleIdx].mode == 0 || s_editMode == 0)) {
          if (moduleState[moduleIdx].mode) {
            moduleState[moduleIdx].mode = 0;
            removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
            killEvents(event); // we stopped BIND / SHARE, we don't want to re-open the menu
            event = 0;
          }
          s_editMode = 0;
        }

        if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
          if (reusableBuffer.moduleSetup.bindInformation.step == BIND_INIT && reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount > 0) {
            popupMenuItemsCount = min<uint8_t>(reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount, PXX2_MAX_RECEIVERS_PER_MODULE);
            for (uint8_t i=0; i<popupMenuItemsCount; i++) {
              popupMenuItems[i] = reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[i];
            }
            //popupMenuTitle = STR_PXX2_SELECT_RX;  // TODO : fix
            POPUP_MENU_START(onPXX2BindMenu);
          }
        }

        if (attr && EVT_KEY_MASK(event) == KEY_ENTER) {
          killEvents(event);
          if (isPXX2ReceiverEmpty(moduleIdx, receiverIdx)) {
            onPXX2ReceiverMenu(STR_BIND);
          }
          else {
            POPUP_MENU_ADD_ITEM(STR_BIND);
            POPUP_MENU_ADD_ITEM(STR_OPTIONS);
            POPUP_MENU_ADD_ITEM(STR_SHARE);
            POPUP_MENU_ADD_ITEM(STR_DELETE);
            POPUP_MENU_ADD_ITEM(STR_RESET);
          }
          POPUP_MENU_START(onPXX2ReceiverMenu);
        }
      }
      break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_BIND:
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_NOT_ACCESS_BIND:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (isModulePPM(moduleIdx)) {
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_PPMFRAME);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+80, y, (moduleData.ppm.delay*50)+300, (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr|LEFT : LEFT, 0, NULL, "us");
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+160, y, moduleData.ppm.pulsePol ? "+" : "-", (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.delay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.ppm.pulsePol, 1);
                break;
            }
          }
        }
        else if (isModuleSBUS(moduleIdx)) {
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_REFRESHRATE);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT, 0, NULL, STR_MS);
          lcdDrawText(MODEL_SETUP_3RD_COLUMN, y, moduleData.sbus.noninverted ? "not inverted" : "normal", (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0);

          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -33, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.sbus.noninverted, 1);
                break;
            }
          }
        }
        else {
          int l_posHorz = menuHorizontalPosition;
          coord_t xOffsetBind = MODEL_SETUP_BIND_OFS;
          if (!isModuleModelIndexAvailable(moduleIdx)) {
            xOffsetBind = 0;
            lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_RECEIVER);
            if (attr) l_posHorz += 1;
          }
          else {
            lcdDrawText(MENUS_MARGIN_LEFT + INDENT_WIDTH, y, STR_RECEIVER_NUM);
          }
          if (isModuleBindRangeAvailable(moduleIdx)) {
            if (xOffsetBind)
              lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], (l_posHorz==0 ? attr : 0) | LEADING0 | LEFT, 2);
            if (attr && l_posHorz==0) {
              if (s_editMode>0) {
                CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], getMaxRxNum(moduleIdx));
                if (event == EVT_KEY_LONG(KEY_ENTER)) {
                  killEvents(event);
                  uint8_t newVal = modelslist.findNextUnusedModelId(moduleIdx);
                  if (newVal != g_model.header.modelId[moduleIdx]) {
                    g_model.header.modelId[moduleIdx] = newVal;
                    storageDirty(EE_MODEL);
                  }
                }
              }
            }
            drawButton(MODEL_SETUP_2ND_COLUMN+xOffsetBind, y, STR_MODULE_BIND, (moduleState[moduleIdx].mode == MODULE_MODE_BIND ? BUTTON_ON : BUTTON_OFF) | (l_posHorz==1 ? attr : 0));
            drawButton(MODEL_SETUP_2ND_COLUMN+MODEL_SETUP_RANGE_OFS+xOffsetBind, y, STR_MODULE_RANGE, (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK ? BUTTON_ON : BUTTON_OFF) | (l_posHorz==2 ? attr : 0));
            uint8_t newFlag = 0;
#if defined(MULTIMODULE)
            if (multiBindStatus == MULTI_BIND_FINISHED) {
              multiBindStatus = MULTI_NORMAL_OPERATION;
              s_editMode = 0;
            }
#endif
            if (attr && l_posHorz>0) {
              if (s_editMode>0) {
                if (l_posHorz == 1) {
                  if (isModuleR9MNonAccess(moduleIdx) || isModuleD16(moduleIdx)) {
                    if (event == EVT_KEY_BREAK(KEY_ENTER)) {
                      startBindMenu(moduleIdx);
                      continue;
                    }
                    if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
                      newFlag = MODULE_MODE_BIND;
                    }
                    else {
                      if (!popupMenuItemsCount) {
                        s_editMode = 0;  // this is when popup is exited before a choice is made
                      }
                    }
                  }
                  else {
                    newFlag = MODULE_MODE_BIND;
                  }
                }
                else if (l_posHorz == 2) {
                  newFlag = MODULE_MODE_RANGECHECK;
                }
              }
            }
            moduleState[moduleIdx].mode = newFlag;
#if defined(MULTIMODULE)
            if (newFlag == MODULE_MODE_BIND)
              multiBindStatus = MULTI_BIND_INITIATED;
#endif
          }
        }
        break;
      }

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_FAILSAFE:
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_FAILSAFE:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_FAILSAFE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VFAILSAFE, moduleData.failsafeMode, menuHorizontalPosition==0 ? attr : 0);
        if (moduleData.failsafeMode == FAILSAFE_CUSTOM) {
          drawButton(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_SET, menuHorizontalPosition==1 ? attr : 0);
        }
        if (attr) {
          if (moduleData.failsafeMode != FAILSAFE_CUSTOM)
            menuHorizontalPosition = 0;
          if (menuHorizontalPosition==0) {
            if (s_editMode>0) {
              CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.failsafeMode, isModuleR9M(moduleIdx) ? FAILSAFE_NOPULSES : FAILSAFE_LAST);
              if (checkIncDec_Ret) SEND_FAILSAFE_NOW(moduleIdx);
            }
          }
          else if (menuHorizontalPosition==1) {
            s_editMode = 0;
            if (moduleData.failsafeMode == FAILSAFE_CUSTOM) {
              if (event == EVT_KEY_LONG(KEY_ENTER)) {
                killEvents(event);
                setCustomFailsafe(moduleIdx);
                storageDirty(EE_MODEL);
                AUDIO_WARNING1();
                SEND_FAILSAFE_NOW(moduleIdx);
              }
              else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
                g_moduleIdx = moduleIdx;
                pushMenu(menuModelFailsafe);
              }
            }
          }
          else {
            lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN, y, LCD_W - MODEL_SETUP_2ND_COLUMN - 2, 8, TEXT_COLOR);
          }
        }
        break;
      }

      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_OPTIONS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
#if defined(MULTIMODULE)
        if (isModuleMultimodule(moduleIdx)) {
          int optionValue = g_model.moduleData[moduleIdx].multi.optionValue;

          const uint8_t multi_proto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true);
          const mm_protocol_definition *pdef = getMultiProtocolDefinition(multi_proto);
          if (pdef->optionsstr)
            lcdDrawText(MENUS_MARGIN_LEFT, y, pdef->optionsstr);

          if (multi_proto == MODULE_SUBTYPE_MULTI_FS_AFHDS2A)
            optionValue = 50 + 5 * optionValue;

          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, optionValue, LEFT | attr);
          if (attr) {
            if (multi_proto == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, 0, 70);
            }
            else if (multi_proto == MODULE_SUBTYPE_MULTI_OLRS) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -1, 7);
            }
            else {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -128, 127);
            }
          }
        }
#endif
        if (isModuleR9MNonAccess(moduleIdx)) {
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MODULE_TELEMETRY);
          if (isSportLineUsedByInternalModule())
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_DISABLE_INTERNAL);
          else
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_TELEM_ON);
        }
        else if (isModuleSBUS(moduleIdx)) {
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_WARN_BATTVOLTAGE);
          drawValueWithUnit(MODEL_SETUP_4TH_COLUMN, y, getBatteryVoltage(), UNIT_VOLTS, attr|PREC2|LEFT);
        }
        break;
      }

      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_POWER:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        if (isModuleR9MNonAccess(moduleIdx)) {
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_RFPOWER);
          if(isModuleR9M_FCC_VARIANT(moduleIdx)) {
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_FCC_POWER_VALUES, g_model.moduleData[moduleIdx].pxx.power, LEFT | attr);
            if (attr)
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].pxx.power, 0, R9M_FCC_POWER_MAX);
          }
          else {
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_LBT_POWER_VALUES, g_model.moduleData[moduleIdx].pxx.power, LEFT | attr);
            if (attr)
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].pxx.power, 0, R9M_LBT_POWER_MAX);
            if (attr && s_editMode == 0 && reusableBuffer.moduleSetup.r9mPower != g_model.moduleData[moduleIdx].pxx.power) {
              if((reusableBuffer.moduleSetup.r9mPower + g_model.moduleData[moduleIdx].pxx.power) < 5) { //switching between mode 2 and 3 does not require rebind
                POPUP_WARNING(STR_WARNING);
                SET_WARNING_INFO(STR_REBIND, sizeof(TR_REBIND), 0);
              }
              reusableBuffer.moduleSetup.r9mPower = g_model.moduleData[moduleIdx].pxx.power;
            }
          }
        }
#if defined(MULTIMODULE)
        else if (isModuleMultimodule(moduleIdx)) {
          lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MULTI_LOWPOWER);
          g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode, MODEL_SETUP_2ND_COLUMN, y, attr, event);
        }
#endif
      }
        break;

#if defined(MULTIMODULE)
    case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AUTOBIND:
      if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MODULE_SUBTYPE_MULTI_DSM2)
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MULTI_DSM_AUTODTECT);
      else
        lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MULTI_AUTOBIND);
      g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, attr, event);
      break;

    case ITEM_MODEL_SETUP_EXTERNAL_MODULE_STATUS: {
      lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MODULE_STATUS);

      char statusText[64];
      multiModuleStatus.getStatusString(statusText);
      lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;

    case ITEM_MODEL_SETUP_EXTERNAL_MODULE_SYNCSTATUS: {
      lcdDrawText(MENUS_MARGIN_LEFT, y, STR_MODULE_SYNC);

      char statusText[64];
      multiSyncStatus.getRefreshString(statusText);
      lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;
      }
    }
#endif
    }
  }

  if (IS_RANGECHECK_ENABLE()) {
    theme->drawMessageBox("RSSI :", NULL, NULL, WARNING_TYPE_INFO);
    lcdDrawNumber(WARNING_LINE_X, WARNING_INFOLINE_Y, TELEMETRY_RSSI(), DBLSIZE|LEFT);
  }

  // some field just finished being edited
  if (old_editMode > 0 && s_editMode == 0) {
    ModelCell* mod_cell = modelslist.getCurrentModel();
    if (mod_cell) {
      switch(menuVerticalPosition) {
        case ITEM_MODEL_SETUP_NAME:
          mod_cell->setModelName(g_model.header.name);
          break;

        case ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_BIND:
          if (menuHorizontalPosition != 0)
            break;
        case ITEM_MODEL_SETUP_INTERNAL_MODULE_TYPE:
          mod_cell->setRfData(&g_model);
          checkModelIdUnique(INTERNAL_MODULE);
          break;

        case ITEM_MODEL_SETUP_EXTERNAL_MODULE_NOT_ACCESS_BIND:
          if (menuHorizontalPosition != 0)
            break;
        case ITEM_MODEL_SETUP_EXTERNAL_MODULE_TYPE:
          mod_cell->setRfData(&g_model);
          if (g_model.moduleData[EXTERNAL_MODULE].type != MODULE_TYPE_NONE)
            checkModelIdUnique(EXTERNAL_MODULE);
      }
    }
  }

  return true;
}