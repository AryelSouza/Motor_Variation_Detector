#include "mpu9250.h"

// Default I2C addresses (AD0 = GND)
#define MPU9250_ADDR (0x68 << 1)
#define AK8963_ADDR  (0x0C << 1)

uint8_t MPU9250_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t check = 0;

    // Tenta ler o WHO_AM_I (0x75)
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, MPU9250_ADDR, 0x75, 1, &check, 1, 100);

    if (status != HAL_OK) {
        return 1; // Erro 1: Sem comunicação (fio solto/morto)
    }

    // Se o valor lido não for o padrão do MPU9250 (0x71)
    if (check != 0x73) {
        return check; // <--- RETORNA O ID EXATO DO CHIP!
    }

    // Configuração básica
    uint8_t data = 0x00; // Reset
    if (HAL_I2C_Mem_Write(hi2c, MPU9250_ADDR, 0x6B, 1, &data, 1, 100) != HAL_OK) {
        return 3; // Erro 3: Falha ao enviar comando de reset
    }
    HAL_Delay(100);

    return 0; // 0 = SUCESSO!
}

void MPU9250_Read_All(I2C_HandleTypeDef *hi2c, MPU9250_t *DataStruct) {
    uint8_t rec_data[14];
    int16_t raw_accel_x, raw_accel_y, raw_accel_z;
    int16_t raw_gyro_x, raw_gyro_y, raw_gyro_z;
    int16_t raw_mag_x, raw_mag_y, raw_mag_z;

    // Read 14 bytes starting from ACCEL_XOUT_H (0x3B)
    if (HAL_I2C_Mem_Read(hi2c, MPU9250_ADDR, 0x3B, 1, rec_data, 14, 25) == HAL_OK) {
        raw_accel_x = (int16_t)(rec_data[0] << 8 | rec_data[1]);
        raw_accel_y = (int16_t)(rec_data[2] << 8 | rec_data[3]);
        raw_accel_z = (int16_t)(rec_data[4] << 8 | rec_data[5]);

        raw_gyro_x = (int16_t)(rec_data[8] << 8 | rec_data[9]);
        raw_gyro_y = (int16_t)(rec_data[10] << 8 | rec_data[11]);
        raw_gyro_z = (int16_t)(rec_data[12] << 8 | rec_data[13]);

        // Convert to standard units (Gs and Degrees/s)
        DataStruct->Ax = raw_accel_x / 16384.0f;
        DataStruct->Ay = raw_accel_y / 16384.0f;
        DataStruct->Az = raw_accel_z / 16384.0f;
        DataStruct->Gx = raw_gyro_x / 131.0f;
        DataStruct->Gy = raw_gyro_y / 131.0f;
        DataStruct->Gz = raw_gyro_z / 131.0f;
    }

    // Read 7 bytes from AK8963 starting from HXL (0x03)
    if (HAL_I2C_Mem_Read(hi2c, AK8963_ADDR, 0x03, 1, rec_data, 7, 25) == HAL_OK) {
        // Note: AK8963 stores data as Little-Endian (LSB first)
        raw_mag_x = (int16_t)(rec_data[1] << 8 | rec_data[0]);
        raw_mag_y = (int16_t)(rec_data[3] << 8 | rec_data[2]);
        raw_mag_z = (int16_t)(rec_data[5] << 8 | rec_data[4]);

        // Convert to micro-Teslas (uT)
        DataStruct->Mx = raw_mag_x * 0.15f;
        DataStruct->My = raw_mag_y * 0.15f;
        DataStruct->Mz = raw_mag_z * 0.15f;
    }
}
