#include "iis2dh.h"

static SPISettings spiSettings(10E6, MSBFIRST, SPI_MODE3);
static IIS2DH_Mode current_mode = IIS2DH_MODE_HIGH_RES;
static IIS2DH_Scale current_scale = IIS2DH_SCALE_2G;

void iis2dh_write_register(uint8_t reg_addr, uint8_t value) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(IIS2DH_CS_PIN, LOW);
    SPI.transfer(reg_addr & 0x7F);
    SPI.transfer(value);
    digitalWrite(IIS2DH_CS_PIN, HIGH);
    SPI.endTransaction();
}

void iis2dh_init(const IIS2DH_Config& config) {
    SPI.begin();
    pinMode(IIS2DH_CS_PIN, OUTPUT);
    digitalWrite(IIS2DH_CS_PIN, HIGH);

    current_mode = config.mode;
    current_scale = config.scale;

    if (config.odr == IIS2DH_ODR_POWER_DOWN) {
        iis2dh_write_register(IIS2DH_CTRL_REG1, 0x00); // Power-down mode
        return;
    }

    bool hr = config.mode == IIS2DH_MODE_HIGH_RES;
    bool lp = config.mode == IIS2DH_MODE_LOW_POWER;

    uint8_t ctrl_reg1 = (config.odr << 4);
    ctrl_reg1 |= (lp ? 1 : 0) << 3;

    bool enable_x = false, enable_y = false, enable_z = false;
    switch (config.axes) {
        case IIS2DH_AXES_XYZ: enable_x = enable_y = enable_z = true; break;
        case IIS2DH_AXES_XY:  enable_x = enable_y = true; break;
        case IIS2DH_AXES_XZ:  enable_x = enable_z = true; break;
        case IIS2DH_AXES_YZ:  enable_y = enable_z = true; break;
        case IIS2DH_AXES_X:   enable_x = true; break;
        case IIS2DH_AXES_Y:   enable_y = true; break;
        case IIS2DH_AXES_Z:   enable_z = true; break;
    }

    ctrl_reg1 |= (enable_x ? 1 : 0) << 2;
    ctrl_reg1 |= (enable_y ? 1 : 0) << 1;
    ctrl_reg1 |= (enable_z ? 1 : 0) << 0;

    uint8_t ctrl_reg4 = 0;
    ctrl_reg4 |= (hr ? 1 : 0) << 3;

    switch (config.scale) {
        case IIS2DH_SCALE_2G: ctrl_reg4 |= 0 << 4; break;
        case IIS2DH_SCALE_4G: ctrl_reg4 |= 1 << 4; break;
        case IIS2DH_SCALE_8G: ctrl_reg4 |= 2 << 4; break;
        case IIS2DH_SCALE_16G: ctrl_reg4 |= 3 << 4; break;
    }

    iis2dh_write_register(IIS2DH_CTRL_REG1, ctrl_reg1);
    iis2dh_write_register(IIS2DH_CTRL_REG4, ctrl_reg4);
}

uint8_t iis2dh_read_register(uint8_t reg_addr) {
    uint8_t value;
    SPI.beginTransaction(spiSettings);
    digitalWrite(IIS2DH_CS_PIN, LOW);
    SPI.transfer(reg_addr | 0x80);
    value = SPI.transfer(0x00);
    digitalWrite(IIS2DH_CS_PIN, HIGH);
    SPI.endTransaction();
    return value;
}

void iis2dh_read_acceleration(iis2dh_data_t *data) {
    if (iis2dh_read_register(IIS2DH_CTRL_REG1) == 0x00) {
        data->x = data->y = data->z = 0;
        return;
    }

    uint8_t buffer[6];
    SPI.beginTransaction(spiSettings);
    digitalWrite(IIS2DH_CS_PIN, LOW);
    SPI.transfer(IIS2DH_OUT_X_L | 0xC0);
    for (int i = 0; i < 6; i++) {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(IIS2DH_CS_PIN, HIGH);
    SPI.endTransaction();

    int shift = 0;
    switch (current_mode) {
        case IIS2DH_MODE_HIGH_RES: shift = 4; break; // 12 bits left-sided
        case IIS2DH_MODE_NORMAL:   shift = 6; break; // 10 bits left-sided
        case IIS2DH_MODE_LOW_POWER:shift = 0; break; // 8 bits left-sided
    }

    data->x = ((int16_t)(buffer[1] << 8 | buffer[0])) >> shift;
    data->y = ((int16_t)(buffer[3] << 8 | buffer[2])) >> shift;
    data->z = ((int16_t)(buffer[5] << 8 | buffer[4])) >> shift;
}

// --- Nouvelle fonction pour obtenir la valeur en mg ---
void iis2dh_read_acceleration_mg(iis2dh_data_mg_t* data_mg) {
    iis2dh_data_t raw;
    iis2dh_read_acceleration(&raw);

    float sens = iis2dh_get_sensitivity_mg_per_digit(current_mode, current_scale);

    data_mg->x = raw.x * sens;
    data_mg->y = raw.y * sens;
    data_mg->z = raw.z * sens;
}

bool iis2dh_is_available() {
    uint8_t id;
    SPI.beginTransaction(spiSettings);
    digitalWrite(IIS2DH_CS_PIN, LOW);
    SPI.transfer(IIS2DH_WHO_AM_I | 0x80);
    id = SPI.transfer(0x00);
    digitalWrite(IIS2DH_CS_PIN, HIGH);
    SPI.endTransaction();
    return (id == IIS2DH_EXPECTED_ID);
}

void print_iis2dh_config() {
    uint8_t ctrl_reg1 = iis2dh_read_register(IIS2DH_CTRL_REG1);
    uint8_t ctrl_reg4 = iis2dh_read_register(IIS2DH_CTRL_REG4);

    Serial.print("CTRL_REG1: 0x");
    Serial.println(ctrl_reg1, HEX);
    Serial.print("CTRL_REG4: 0x");
    Serial.println(ctrl_reg4, HEX);
}

float iis2dh_get_sensitivity_mg_per_digit(IIS2DH_Mode mode, IIS2DH_Scale scale) {
    switch (mode) {
        case IIS2DH_MODE_HIGH_RES:
            switch (scale) {
                case IIS2DH_SCALE_2G: return 0.98f;
                case IIS2DH_SCALE_4G: return 1.95f;
                case IIS2DH_SCALE_8G: return 3.91f;
                case IIS2DH_SCALE_16G: return 11.72f;
            }
            break;
        case IIS2DH_MODE_NORMAL:
            switch (scale) {
                case IIS2DH_SCALE_2G: return 3.9f;
                case IIS2DH_SCALE_4G: return 7.82f;
                case IIS2DH_SCALE_8G: return 15.63f;
                case IIS2DH_SCALE_16G: return 46.9f;
            }
            break;
        case IIS2DH_MODE_LOW_POWER:
            switch (scale) {
                case IIS2DH_SCALE_2G: return 15.63f;
                case IIS2DH_SCALE_4G: return 31.26f;
                case IIS2DH_SCALE_8G: return 62.52f;
                case IIS2DH_SCALE_16G: return 187.58f;
            }
            break;
    }
    return 0.0f;
}

void iis2dh_configure_interrupt_drdy() {
  pinMode(IIS2DH_INT1, INPUT);                 // INT1 pin

  iis2dh_write_register(0x24, 0x00); // CTRL_REG5 : disable interrupt latching
  iis2dh_write_register(0x25, 0x00); // CTRL_REG6 : disable other interrupts
  iis2dh_write_register(0x22, 0x10); // CTRL_REG3 : DRDY1 on INT1
  iis2dh_write_register(0x30, 0x00); // INT1_CFG : no interrupt latched
}


bool iis2dh_data_ready() {
    uint8_t status = iis2dh_read_register(IIS2DH_STATUS_REG);
    return (status & 0x08); // Bit ZYXDA (bit 3) : 1 si nouvelle donnée prête
}




// ------------------------------------------------------//TEST SETUP + LOOP LECTURE CAPTEUR
// #include "iis2dh.h"
// #include "iis2dh_config.h"

// #include <Arduino.h>
// #include "wifi_manager.h"
//// #include <WebServer.h> // conflit avec ESPAsyncWeb, choisir
// void setup() {
//   Serial.begin(115200);
//   delay(500);

//   // Configuration : mode haute résolution, 1.344kHz, ±2g, axes XYZ
//   IIS2DH_Config config = {
//     .mode = IIS2DH_MODE_HIGH_RES,
//     .odr = IIS2DH_ODR_1344HZ,
//     .scale = IIS2DH_SCALE_4G,
//     .axes = IIS2DH_AXES_XYZ
//   };

//   iis2dh_init(config);

//   if (iis2dh_is_available()) {
//     Serial.println("IIS2DH détecté !");
//     print_iis2dh_config();
//   } else {
//     Serial.println("Erreur : capteur non détecté.");
//   }
// }

// void loop() {
//   iis2dh_data_t raw_data;
//   iis2dh_data_mg_t data_mg;

//   // Lecture brute
//   iis2dh_read_acceleration(&raw_data);
//   // Conversion en mg
//   iis2dh_read_acceleration_mg(&data_mg);

//   // Affichage
//   Serial.print("Brut  -> X: "); Serial.print(raw_data.x);
//   Serial.print(" | Y: "); Serial.print(raw_data.y);
//   Serial.print(" | Z: "); Serial.print(raw_data.z);

//   Serial.print("   ||   Converti -> X: "); Serial.print(data_mg.x, 2); Serial.print(" mg");
//   Serial.print(" | Y: "); Serial.print(data_mg.y, 2); Serial.print(" mg");
//   Serial.print(" | Z: "); Serial.print(data_mg.z, 2); Serial.println(" mg");

//   delay(5000);
// }


// //----------------------------------------//TEST INTERRUPTION OK---------------------------------------------------
// #include <Arduino.h>
// #include "iis2dh.h"

// // Flag d'interruption
// volatile bool data_ready = false;

// // ISR sur front montant INT1
// void IRAM_ATTR on_data_ready() {
//   data_ready = true;
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   // Configuration capteur 10Hz, high-res, 2G, XYZ
//   IIS2DH_Config config = {
//     .mode = IIS2DH_MODE_HIGH_RES,
//     .odr = IIS2DH_ODR_1HZ,
//     .scale = IIS2DH_SCALE_2G,
//     .axes = IIS2DH_AXES_XYZ
//   };
//   iis2dh_init(config);

//   // Configuration interruption DRDY
//   iis2dh_configure_interrupt_drdy();

//   // Purge premier DRDY latent en lisant une fois
//   iis2dh_data_t dummy;
//   iis2dh_read_acceleration(&dummy);

//   pinMode(IIS2DH_INT1, INPUT);

//   attachInterrupt(digitalPinToInterrupt(IIS2DH_INT1), on_data_ready, RISING);

//   Serial.println("=== Capteur prêt, interruption DRDY à 1 Hz ===");
// }

// void loop() {
//   if (data_ready) {
//     data_ready = false;

//     // Vérifie que nouvelle donnée est bien dispo
//     uint8_t status = iis2dh_read_register(0x27);
//     if (status & 0x08) { // bit ZYXDA = 1 => data ready
//       iis2dh_data_mg_t acc;
//       iis2dh_read_acceleration_mg(&acc);

//       Serial.print("Acc (mg) X: "); Serial.print(acc.x, 2);
//       Serial.print(" | Y: "); Serial.print(acc.y, 2);
//       Serial.print(" | Z: "); Serial.println(acc.z, 2);
//     } else {
//       Serial.println("Interruption reçue mais pas de nouvelle donnée !");
//     }
//   }
//   Serial.print("INT1 = ");
//   Serial.println(digitalRead(IIS2DH_INT1));
//   delay(500);

// }

