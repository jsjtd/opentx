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

#define RECEIVER_OPTIONS_2ND_COLUMN 80

extern uint8_t g_moduleIdx;

void onRxOptionsUpdateConfirm(const char * result)
{
  if (result == STR_OK) {
    reusableBuffer.hardwareAndSettings.receiverSettings.state = PXX2_SETTINGS_WRITE;
    reusableBuffer.hardwareAndSettings.receiverSettings.dirty = 2;
    reusableBuffer.hardwareAndSettings.receiverSettings.timeout = 0;
    moduleState[g_moduleIdx].mode = MODULE_MODE_RECEIVER_SETTINGS;
  }
  else {
    popMenu();
  }
}

enum {
  ITEM_RECEIVER_SETTINGS_TELEMETRY,
  ITEM_RECEIVER_SETTINGS_PWM_RATE,
  ITEM_RECEIVER_SETTINGS_SPORT_FPORT,
  ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED1,
  ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED2,
  ITEM_RECEIVER_SETTINGS_PINMAP_FIRST
};


#define IF_RECEIVER_CAPABILITY(capability, count) uint8_t((reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.capabilities & (1 << capability)) ? count : HIDDEN_ROW)

void menuModelReceiverOptions(event_t event)
{
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;
  uint8_t wbar = LCD_W / 2 - 20;
  auto outputsCount = min<uint8_t>(16, reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount);

  if (event == EVT_ENTRY) {
    memclear(&reusableBuffer.hardwareAndSettings, sizeof(reusableBuffer.hardwareAndSettings));
#if defined(SIMU)
    reusableBuffer.hardwareAndSettings.receiverSettings.state = PXX2_SETTINGS_OK;
    reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount = 8;
#endif
  }
  else if (menuEvent) {
    killEvents(KEY_EXIT);
    moduleState[g_moduleIdx].mode = MODULE_MODE_NORMAL;
    if (reusableBuffer.hardwareAndSettings.receiverSettings.dirty) {
      abortPopMenu();
      POPUP_CONFIRMATION(STR_UPDATE_RX_OPTIONS, onRxOptionsUpdateConfirm);
    }
    else {
      return;
    }
  }

  uint8_t receiverId = reusableBuffer.hardwareAndSettings.receiverSettings.receiverId;
  uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.modelID;

  SUBMENU_NOTITLE(ITEM_RECEIVER_SETTINGS_PINMAP_FIRST + outputsCount, {
    0, // Telemetry
    0, // PWM rate
    IF_RECEIVER_CAPABILITY(RECEIVER_CAPABILITY_FPORT, 0),
    uint8_t(reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.capabilityNotSupported ? READONLY_ROW : HIDDEN_ROW),
    uint8_t(reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.capabilityNotSupported ? READONLY_ROW : HIDDEN_ROW),
    0 // channels ...
  });

  if (reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_HARDWARE_INFO && moduleState[g_moduleIdx].mode == MODULE_MODE_NORMAL) {
    if (modelId)
      moduleState[g_moduleIdx].readReceiverSettings(&reusableBuffer.hardwareAndSettings.receiverSettings);
    else
      moduleState[g_moduleIdx].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[g_moduleIdx], receiverId, receiverId);
  }

  if (event == EVT_KEY_LONG(KEY_ENTER) && reusableBuffer.hardwareAndSettings.receiverSettings.dirty) {
    killEvents(event);
    reusableBuffer.hardwareAndSettings.receiverSettings.dirty = 0;
    moduleState[g_moduleIdx].writeReceiverSettings(&reusableBuffer.hardwareAndSettings.receiverSettings);
  }

  if (reusableBuffer.hardwareAndSettings.receiverSettings.dirty == 2 && reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_SETTINGS_OK) {
    popMenu();
    return;
  }

  if (modelId != 0 && mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
    menuVerticalPosition = 0;
    while (menuVerticalPosition < ITEM_RECEIVER_SETTINGS_PINMAP_FIRST && mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
      ++menuVerticalPosition;
    }
  }

  int8_t sub = menuVerticalPosition;
  lcdDrawTextAlignedLeft(0, STR_RECEIVER_OPTIONS);
  drawReceiverName(FW * 13, 0, g_moduleIdx, reusableBuffer.hardwareAndSettings.receiverSettings.receiverId);
  lcdInvertLine(0);

  if (reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_SETTINGS_OK) {
    for (uint8_t k=0; k<LCD_LINES-1; k++) {
      coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
      uint8_t i = k + menuVerticalOffset;
      for (int j=0; j<=i; ++j) {
        if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
          ++i;
        }
      }
      LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

      switch (i) {
        case ITEM_RECEIVER_SETTINGS_TELEMETRY:
          reusableBuffer.hardwareAndSettings.receiverSettings.telemetryDisabled = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.telemetryDisabled, RECEIVER_OPTIONS_2ND_COLUMN, y, STR_TELEMETRY_DISABLED, attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = true;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_PWM_RATE:
          reusableBuffer.hardwareAndSettings.receiverSettings.pwmRate = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.pwmRate, RECEIVER_OPTIONS_2ND_COLUMN, y, isModuleR9MAccess(g_moduleIdx) ? "6.67ms PWM": "9ms PWM", attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = true;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_SPORT_FPORT:
          reusableBuffer.hardwareAndSettings.receiverSettings.fport = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.fport, RECEIVER_OPTIONS_2ND_COLUMN, y, "F.Port", attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = true;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED1:
          lcdDrawText(LCD_W/2, y+1, STR_MORE_OPTIONS_AVAILABLE, SMLSIZE|CENTERED);
          break;

        case ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED2:
          lcdDrawText(LCD_W/2, y+1, STR_OPENTX_UPGRADE_REQUIRED, SMLSIZE|CENTERED);
          break;

        default:
        // Pin
        {
          uint8_t pin = i - ITEM_RECEIVER_SETTINGS_PINMAP_FIRST;
          if (pin < reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount) {
            uint8_t & mapping = reusableBuffer.hardwareAndSettings.receiverSettings.outputsMapping[pin];
            uint8_t channel = g_model.moduleData[g_moduleIdx].channelsStart + mapping;
            int32_t channelValue = channelOutputs[channel];
            lcdDrawText(0, y, STR_PIN);
            lcdDrawNumber(lcdLastRightPos + 1, y, pin + 1);
            putsChn(7 * FW, y, channel + 1, attr);

            // Channel
            if (attr) {
              mapping = checkIncDec(event, mapping, 0, sentModuleChannels(g_moduleIdx) - 1);
              if (checkIncDec_Ret) {
                reusableBuffer.hardwareAndSettings.receiverSettings.dirty = true;
              }
            }

            // Bargraph
#if !defined(PCBX7) // X7 LCD doesn't like too many horizontal lines
            lcdDrawRect(RECEIVER_OPTIONS_2ND_COLUMN, y + 2, wbar + 1, 4);
#endif
            const uint8_t lenChannel = limit<uint8_t>(1, (abs(channelValue) * wbar / 2 + lim / 2) / lim, wbar / 2);
            const coord_t xChannel = (channelValue > 0) ? RECEIVER_OPTIONS_2ND_COLUMN + wbar / 2 : RECEIVER_OPTIONS_2ND_COLUMN + wbar / 2 + 1 - lenChannel;
            lcdDrawHorizontalLine(xChannel, y + 3, lenChannel, SOLID, 0);
            lcdDrawHorizontalLine(xChannel, y + 4, lenChannel, SOLID, 0);
          }
          break;
        }
      }
    }
  }
  else {
    lcdDrawCenteredText(LCD_H/2, STR_WAITING_FOR_RX);
  }
}
