/*
 *
 */

#pragma once

#define SETTINGS_HOME_PAGE         0
#define SETTINGS_BLINDSIGNING_PAGE 1

/**
 * @brief Initialize settings screen for nano devices. Displays status of
 * Expert-mode and Blind Signing.
 *
 * @param page Current page to display among all the pages available under
 * Settings.
 */
void ui_settings_init(int16_t page);
