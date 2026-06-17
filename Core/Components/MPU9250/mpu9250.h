#ifndef MPU9250_H_
#define MPU9250_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Struct to store sensor data
typedef struct {
    float Ax, Ay, Az;
    float Gx, Gy, Gz;
    float Mx, My, Mz;
} MPU9250_t;

// Initialization and read functions
uint8_t MPU9250_Init(I2C_HandleTypeDef *hi2c);
void MPU9250_Read_All(I2C_HandleTypeDef *hi2c, MPU9250_t *DataStruct);

#ifdef __cplusplus
}
#endif

#endif /* MPU9250_H_ */
