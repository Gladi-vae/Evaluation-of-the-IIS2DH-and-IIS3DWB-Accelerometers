#ifndef IIS2DH_H
#define IIS2DH_H

#include <Arduino.h>
#include <SPI.h>
#include "iis2dh_config.h"

#define IIS2DH_CS_PIN 4
#define IIS2DH_INT1 15

#define IIS2DH_CTRL_REG1 0x20
#define IIS2DH_CTRL_REG4 0x23
#define IIS2DH_WHO_AM_I 0x0F
#define IIS2DH_EXPECTED_ID 0x33
#define IIS2DH_OUT_X_L 0x28
#define IIS2DH_STATUS_REG 0x27


typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} iis2dh_data_t;

typedef struct {
    float x;
    float y;
    float z;
} iis2dh_data_mg_t;

void iis2dh_write_register(uint8_t reg_addr, uint8_t value);
void iis2dh_init(const IIS2DH_Config& config);
bool iis2dh_is_available();
uint8_t iis2dh_read_register(uint8_t reg_addr);
void iis2dh_read_acceleration(iis2dh_data_t* data);
void iis2dh_read_acceleration_mg(iis2dh_data_mg_t* data_mg);
float iis2dh_get_sensitivity_mg_per_digit(IIS2DH_Mode mode, IIS2DH_Scale scale);
void print_iis2dh_config();
void iis2dh_configure_interrupt_drdy();
bool iis2dh_data_ready();

#endif

