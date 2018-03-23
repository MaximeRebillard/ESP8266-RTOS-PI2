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

#include "esp_common.h"
#include "pwm.h"
#include "math.h"


//PIN SETTINGS FOR PWM
//12 => OK
#define PWM_12_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_12_OUT_IO_NUM 12
#define PWM_12_OUT_IO_FUNC  FUNC_GPIO12

//13 => OK
#define PWM_13_OUT_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define PWM_13_OUT_IO_NUM 13
#define PWM_13_OUT_IO_FUNC  FUNC_GPIO13

//14 => OK
#define PWM_14_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_14_OUT_IO_NUM 14
#define PWM_14_OUT_IO_FUNC  FUNC_GPIO14

//15 => OK
#define PWM_15_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_15_OUT_IO_NUM 15
#define PWM_15_OUT_IO_FUNC  FUNC_GPIO15

//4 => OK
#define PWM_4_OUT_IO_MUX PERIPHS_IO_MUX_GPIO4_U
#define PWM_4_OUT_IO_NUM 4
#define PWM_4_OUT_IO_FUNC  FUNC_GPIO4

//0 => OK
#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_GPIO0_U
#define PWM_0_OUT_IO_NUM 0
#define PWM_0_OUT_IO_FUNC FUNC_GPIO0

//5 => OK
#define PWM_5_OUT_IO_MUX PERIPHS_IO_MUX_GPIO5_U
#define PWM_5_OUT_IO_NUM 5
#define PWM_5_OUT_IO_FUNC FUNC_GPIO5

//2 => OK
#define PWM_2_OUT_IO_MUX PERIPHS_IO_MUX_GPIO2_U
#define PWM_2_OUT_IO_NUM 2
#define PWM_2_OUT_IO_FUNC FUNC_GPIO2




#define PWM_NUM_CHANNEL_NUM  8  //number of PWM Channels

#define input_start 0
#define input_end 1024
#define output_start 0
#define output_end 152

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

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
 /*
   flex sensor value are between 0 and 1024
   according to sparkfun : https://learn.sparkfun.com/tutorials/pulse-width-modulation
   Frequency/period are specific to controlling a specific servo. A typical servo motor expects to be updated every 20 ms with a pulse between 1 ms and 2 ms,
   or in other words, between a 5 and 10% duty cycle on a 50 Hz waveform. With a 1.5 ms pulse, the servo motor will be at the natural 90 degree position.
   With a 1 ms pulse, the servo will be at the 0 degree position, and with a 2 ms pulse, the servo will be at 180 degrees. You can obtain the full range of
   motion by updating the servo with an value in between.

   Moreover the maximum duty cycle of a PWM channel can be 1023
   so 5% and 10% duty cycle of 1023 is respectively 52 (for 0 degree) and 102 (for 90 degrees)
 */
 /******************************************************************************/

 //function that takes a flex sensor value and return the corresponding servo value (here that's not the angle)


void user_init(void)
{
    int i = 0;
    printf("SDK version:%s\n", system_get_sdk_version());
    printf("PWM TEST !!!!\r\n");

    //need to declare all the pin here
    uint32 io_info[][3] = {
            { PWM_12_OUT_IO_MUX, PWM_12_OUT_IO_FUNC, PWM_12_OUT_IO_NUM }, //Channel 0
            { PWM_13_OUT_IO_MUX, PWM_13_OUT_IO_FUNC, PWM_13_OUT_IO_NUM }, //Channel 1
            { PWM_14_OUT_IO_MUX, PWM_14_OUT_IO_FUNC, PWM_14_OUT_IO_NUM }, //Channel 2
            { PWM_15_OUT_IO_MUX, PWM_15_OUT_IO_FUNC, PWM_15_OUT_IO_NUM }, //Channel 3
            { PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC, PWM_0_OUT_IO_NUM }, //Channel 4
            { PWM_4_OUT_IO_MUX, PWM_4_OUT_IO_FUNC, PWM_4_OUT_IO_NUM }, // Channel 5
            { PWM_5_OUT_IO_MUX, PWM_5_OUT_IO_FUNC, PWM_5_OUT_IO_NUM }, //Channel 6
            { PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC, PWM_2_OUT_IO_NUM }, //Channel 7
    };

	u32 duty[1] = {15}; //Max duty cycle is 1023
	pwm_init(20000, duty,PWM_NUM_CHANNEL_NUM,io_info);
	while(1){

    for(int i = 0; i <102 ; i += 10){

      if(i + 10 > 102)break;
      		pwm_set_duty(i, 0);
          pwm_set_duty(i, 2);
          pwm_start();
          printf("duty : %d\n",pwm_get_duty(0));
          vTaskDelay(10);
}

    for(int i = 102; i > 0 ; i-=10){
      if(i - 10 > 102)break;
      pwm_set_duty(i, 0);
      pwm_set_duty(i, 2);
      pwm_start();
      printf("duty : %d\n",pwm_get_duty(0));
      vTaskDelay(10);
    }
    /*
		pwm_set_duty(0, 1);
    pwm_start();
    printf("duty : %d\n",pwm_get_duty(1));
    vTaskDelay(20);


    pwm_set_duty(102,1);
    pwm_start();
    printf("duty : %d\n",pwm_get_duty(1));
    vTaskDelay(20);
    */


/*
    pwm_set_duty(102,1);
    pwm_start();   //Call this: every time you change duty/period
    printf("duty : %d\n",pwm_get_duty(1));
		vTaskDelay(20); //400 milliSec Delay

    pwm_set_duty(180,1);
    pwm_start();   //Call this: every time you change duty/period
    printf("duty : %d\n",pwm_get_duty(1));
		vTaskDelay(20); //400 milliSec Delay

*/

	}
}
