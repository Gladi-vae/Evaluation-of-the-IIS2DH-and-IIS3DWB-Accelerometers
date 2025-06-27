#ifndef IIS2DH_CONFIG_H
#define IIS2DH_CONFIG_H

// === ÉCHELLES DISPONIBLES ===
enum IIS2DH_Scale {
    IIS2DH_SCALE_2G = 2,
    IIS2DH_SCALE_4G = 4,
    IIS2DH_SCALE_8G = 8,
    IIS2DH_SCALE_16G = 16
};

// === MODES D’OPÉRATION DISPONIBLES ===
enum IIS2DH_Mode {
    IIS2DH_MODE_LOW_POWER,   // Low-power mode (8 bits)
    IIS2DH_MODE_NORMAL,      // Normal mode (10 bits)
    IIS2DH_MODE_HIGH_RES     // High-resolution mode (12 bits)
};

// === AXES ACTIVÉS ===
enum IIS2DH_AxisConfig {
    IIS2DH_AXES_XYZ,
    IIS2DH_AXES_XY,
    IIS2DH_AXES_XZ,
    IIS2DH_AXES_YZ,
    IIS2DH_AXES_X,
    IIS2DH_AXES_Y,
    IIS2DH_AXES_Z
};

// === FRÉQUENCES D’ÉCHANTILLONNAGE ODR ===
enum IIS2DH_ODR {
    IIS2DH_ODR_POWER_DOWN = 0x00, // Power-down mode (no data)
    IIS2DH_ODR_1HZ        = 0x01, // HR / Normal / Low-power: 1 Hz
    IIS2DH_ODR_10HZ       = 0x02, // 10 Hz
    IIS2DH_ODR_25HZ       = 0x03, // 25 Hz
    IIS2DH_ODR_50HZ       = 0x04, // 50 Hz
    IIS2DH_ODR_100HZ      = 0x05, // 100 Hz
    IIS2DH_ODR_200HZ      = 0x06, // 200 Hz
    IIS2DH_ODR_400HZ      = 0x07, // 400 Hz
    IIS2DH_ODR_1620HZ_LP  = 0x08, // Low-power only: 1.620 kHz
    IIS2DH_ODR_1344HZ     = 0x09  // HR/Normal: 1.344 kHz | LP: 5.376 kHz
};

// === CONFIGURATION GLOBALE DU CAPTEUR ===
struct IIS2DH_Config {
    IIS2DH_Mode mode;
    IIS2DH_ODR odr;
    IIS2DH_Scale scale;
    IIS2DH_AxisConfig axes;
};

#endif
