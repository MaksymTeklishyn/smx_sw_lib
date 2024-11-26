#ifndef SMX_CONSTANTS_H
#define SMX_CONSTANTS_H

/**
 * @file smxConstants.h
 * @brief Defines constants used across the smxPscan project.
 */

/**
 * @brief Number of channels per ASIC.
 */
constexpr int smxNCh = 128;

/**
 * @brief Number of ADC comparators.
 */
constexpr int smxNAdc = 31;

/**
 * @brief Maximum test pulse amplitude in arbitrary units.
 */
constexpr int smxNApmCalU = 255;

/**
 * @brief Conversion factor from elementary charge (e) to femtoCoulombs (fC).
 * 
 * One electron charge is approximately equal to 0.16 fC.
 */
constexpr double smxEtoFC = 1/0.1602176634;

/**
 * @brief Conversion factor from amplitude calibration units (a.u.) to elementary charge (e).
 */
constexpr double smxAmCaltoE = 342.7;

#endif // SMX_CONSTANTS_H

