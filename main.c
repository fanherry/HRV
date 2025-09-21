#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

#define MAX30102_I2C_ADDR (0x57 << 1)

// MAX30102 Register Addresses
#define MAX30102_FIFO_WR_PTR 0x04
#define MAX30102_FIFO_DATA 0x07
#define MAX30102_FIFO_CONFIG 0x08
#define MAX30102_MODE_CONFIG 0x09
#define MAX30102_SPO2_CONFIG 0x0A
#define MAX30102_LED1_PA 0x0C
#define MAX30102_LED2_PA 0x0D
#define MAX30102_MULTI_LED_CTRL1 0x11

// Function prototypes
void SystemClock_Config(void);
void Error_Handler(void);
HAL_StatusTypeDef MAX30102_WriteRegister(uint8_t reg, uint8_t value);
void MAX30102_ReadFIFO(uint32_t *ir);
void MAX30102_Init(void);

HAL_StatusTypeDef MAX30102_WriteRegister(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return HAL_I2C_Master_Transmit(&hi2c1, MAX30102_I2C_ADDR, data, 2, 100);
}

void MAX30102_ReadFIFO(uint32_t *ir) {
    uint8_t buffer[6];
    uint8_t reg = MAX30102_FIFO_DATA;
    
    HAL_I2C_Master_Transmit(&hi2c1, MAX30102_I2C_ADDR, &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, MAX30102_I2C_ADDR, buffer, 6, 100);
    
    *ir = (buffer[3] << 16) | (buffer[4] << 8) | buffer[5];
    *ir &= 0x03FFFF;  // Keep only 18 bits
}

void MAX30102_Init(void) {
    MAX30102_WriteRegister(MAX30102_MODE_CONFIG, 0x40); // Reset
    HAL_Delay(10);
    
    MAX30102_WriteRegister(MAX30102_FIFO_CONFIG, 0x4F); // FIFO config
    MAX30102_WriteRegister(MAX30102_SPO2_CONFIG, 0x27); // SpO2 config
    MAX30102_WriteRegister(MAX30102_LED1_PA, 0x24);     // IR LED current
    MAX30102_WriteRegister(MAX30102_LED2_PA, 0x24);     // Red LED current
    MAX30102_WriteRegister(MAX30102_MULTI_LED_CTRL1, 0x21); // LED mode
    MAX30102_WriteRegister(MAX30102_MODE_CONFIG, 0x03); // SpO2 mode
}

void Error_Handler(void) {
    while(1) {
        // Error handling loop
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    
    MAX30102_Init();
    
    uint32_t ir_value;
    
    while (1) {
        MAX30102_ReadFIFO(&ir_value);
        
        // Print IR value only
        char buffer[20];
        sprintf(buffer, "IR: %lu\r\n", ir_value);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        
        HAL_Delay(10);
    }
}

void SystemClock_Config(void) {
    // System clock configuration
}
