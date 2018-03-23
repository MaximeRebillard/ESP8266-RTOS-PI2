
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
#include "lwip/udp.h"
#include "gpio.h"

#define LED_GPIO 2
#define LED_GPIO_MUX PERIPHS_IO_MUX_GPIO2_U
#define LED_GPIO_FUNC FUNC_GPIO2

xTaskHandle blink;



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

void blink_task(void){
uint8_t state = 0;
  while (1)
  {
       state = 1 - state;
       GPIO_OUTPUT_SET (LED_GPIO, state);
       vTaskDelay (20);
  }
}
/*
  The payload contained in the pbuf object is stored as a byte array but its actual type is void, which is to say it’s typeless.
  There are two reasons for that : LwIP can’t know what format your payload uses (since it’s your own custom protocol) and
  it also can’t know if the machines on each end of the transmission use the same endianness. This is common to all network
  stacks and it means you need to decode yourself the format your payload is coded in. If necessary, you also need to reorder
  its bytes yourself. That’s not really fun or glamorous, but it gots to be done.
*/
void nefastor_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  // we know there’s a payload (non-NULL pbuf pointer)
  if (p != NULL)
  {
  //cast the payload into its proper type
   uint8_t data = (uint8_t) ((uint8_t*)p->payload)[0];
   /*
   Once we’ve got our data byte, we can (must, really) free the pbuf we got it from.
   This could wait until the end of the function. Where you choose to do it may be
   based on how quickly your callback needs to process the data, but that pbuf still
    needs to be deallocated before the callback returns.
   */
   pbuf_free(p);

   if (data == 1)
     vTaskResume (blink);
     printf("receive 1");
   if (data == 0)
     vTaskSuspend (blink);
     printf("receive 0");
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
   //we must tell the ESP8266 how we want to use that pin. At the start of the function, paste :
  PIN_FUNC_SELECT(LED_GPIO_MUX, LED_GPIO_FUNC);
/*
    printf("Please get the RTOS version of SDK.%s\n", system_get_sdk_version());
    espconn_init();
    */
    xTaskCreate(blink_task, "blink_task", 256, NULL, 2, &blink);
//    sprintf(config->ssid, "Livebox-087C_2.4GHz");
  //  sprintf(config->password, "ZcZkfDCpftPKuHWFSU");


    struct station_config *config = (struct station_config *)zalloc(sizeof(struct station_config));
    sprintf(config->ssid, "cc");
    sprintf(config->password, "12345678");
    wifi_station_set_config_current(config);
    free(config);

    struct udp_pcb *nefastor_pcb;
    nefastor_pcb = udp_new();
    udp_bind(nefastor_pcb, IP_ADDR_ANY, 8888);
    udp_recv(nefastor_pcb, nefastor_udp_recv, NULL);
}
