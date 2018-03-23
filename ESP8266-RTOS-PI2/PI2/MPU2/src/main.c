/*
/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

//Pin SDA 2
//Pin SCL 14

#include "esp_common.h"
#include "gpio.h"
#include "i2c.h"

#define mpu6050 0x68

uint8 Acknowledge;
uint8 pData[14];
uint8 count;

uint16 Acx;
uint8 Accelerometer_X1;
uint8 Accelerometer_X2;

uint16 Acy;
uint8 Accelerometer_Y1;
uint8 Accelerometer_Y2;

uint16 Acz;
uint8 Accelerometer_Z1;
uint8 Accelerometer_Z2;

uint16 Tem;
uint8 Temperature_1;
uint8 Temperature_2;

uint16 Gyx;
uint8 Gyroscope_X1;
uint8 Gyroscope_X2;

uint16 Gyy;
uint8 Gyroscope_Y1;
uint8 Gyroscope_Y2;

uint16 Gyz;
uint8 Gyroscope_Z1;
uint8 Gyroscope_Z2;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void i2c_test_task()
{
  int ack1 = 0;
  int ack2 = 0;
  int ack3 = 0;
  i2c_init();
  i2c_start();

  ack1 = i2c_write(0x68 << 1 | 0);
  ack2 = i2c_write(0x6B);
  ack3 = i2c_write(0x00);
  i2c_stop();
  i2c_start();

  while(1){

    printf("ack1 : %d, ack2 : %d, ack3 : %d",ack1,ack2,ack3);
    Acknowledge = i2c_write(0x68 << 1 | 0);
    printf("%d\n",Acknowledge);
    Acknowledge = i2c_write(0x3B);
    printf("%d\n",Acknowledge);
    i2c_start();
    Acknowledge = i2c_write(0x68 << 1 | 1);
    printf("%d\n",Acknowledge);

    printf("start MPU6050");
    Accelerometer_X1 = i2c_read();
    i2c_set_ack(0);
    Accelerometer_X2 = i2c_read();
    i2c_set_ack(0);
    Accelerometer_Y1 = i2c_read();
    i2c_set_ack(0);
    Accelerometer_Y2 = i2c_read();
    i2c_set_ack(0);
    Accelerometer_Z1 = i2c_read();
    i2c_set_ack(0);
    Accelerometer_Z2 = i2c_read();
    i2c_set_ack(0);
    Temperature_1 = i2c_read();
    i2c_set_ack(0);
    Temperature_2 = i2c_read();
    i2c_set_ack(0);

    Gyroscope_X1 = i2c_read();
    i2c_set_ack(0);
    Gyroscope_X2 = i2c_read();
    i2c_set_ack(0);
    Gyroscope_Y1 = i2c_read();
    i2c_set_ack(0);
    Gyroscope_Y2 = i2c_read();
    i2c_set_ack(0);
    Gyroscope_Z1 = i2c_read();
    i2c_set_ack(0);
    Gyroscope_Z2 = i2c_read();
    i2c_set_ack(1);

    Acx = Accelerometer_X1 << 8 | Accelerometer_X2;
    Acy = Accelerometer_Y1 << 8 | Accelerometer_Y2;
    Acz = Accelerometer_Z1 << 8 | Accelerometer_Z2;

    Tem = Temperature_1 << 8 | Temperature_2;
    Gyx = Gyroscope_X1 << 8 | Gyroscope_X2;
    Gyy = Gyroscope_Y1 << 8 | Gyroscope_Y2;
    Gyz = Gyroscope_Z1 << 8 | Gyroscope_Z2;

    printf("stop MPU6050\n");
    printf("Acx :%d\n", Acx);
    printf("Acy :%d\n", Acy);
    printf("Acz :%d\n", Acz);
    printf("Tem :%d\n", Tem);
    printf("Gyx :%d\n", Gyx);
    printf("Gyy :%d\n", Gyy);
    printf("Gyz :%d\n", Gyz);
    vTaskDelay(20);  //Delay for 200milli seconds
  }


}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
  wifi_softap_free_station_info();

	xTaskCreate(i2c_test_task, "user_start_tx_task", 200, NULL, 3, NULL);

}
