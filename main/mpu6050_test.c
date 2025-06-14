#include <stdio.h>
#include "driver/i2c.h"
#include "mpu6050.h"
#include "esp_system.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 22    /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

static const char *TAG = "mpu6050";
static mpu6050_handle_t mpu6050 = NULL;

static void i2c_bus_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL
    };

    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C config failed");
        return;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed");
    }
}

static void i2c_sensor_mpu6050_init(void)
{
    esp_err_t ret;

    i2c_bus_init();

    mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
    if (mpu6050 == NULL) {
        ESP_LOGE(TAG, "MPU6050 create failed");
        return;
    }

    ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 config failed");
        return;
    }

    ret = mpu6050_wake_up(mpu6050);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 wake up failed");
        return;
    }
}

void app_main(void)
{
    esp_err_t ret;
    uint8_t mpu6050_deviceid;
    mpu6050_acce_value_t acce;
    mpu6050_gyro_value_t gyro;
    mpu6050_temp_value_t temp;

    i2c_sensor_mpu6050_init();

    ret = mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);
    if (ret != ESP_OK || mpu6050_deviceid != MPU6050_WHO_AM_I_VAL) {
        ESP_LOGE(TAG, "MPU6050 not found or wrong device ID (0x%02x)", mpu6050_deviceid);
        return;
    }

    while (1) {
        ret = mpu6050_get_acce(mpu6050, &acce);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Accel -> X: %.2f, Y: %.2f, Z: %.2f", acce.acce_x, acce.acce_y, acce.acce_z);
        }

        ret = mpu6050_get_gyro(mpu6050, &gyro);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Gyro  -> X: %.2f, Y: %.2f, Z: %.2f", gyro.gyro_x, gyro.gyro_y, gyro.gyro_z);
        }

        ret = mpu6050_get_temp(mpu6050, &temp);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Temp  -> %.2f °C", temp.temp);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }

    // These lines won't run; delete if you don't plan to stop the loop
    // mpu6050_delete(mpu6050);
    // i2c_driver_delete(I2C_MASTER_NUM);
}
