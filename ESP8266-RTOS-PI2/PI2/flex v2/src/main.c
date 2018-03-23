#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "gpio.h"
//#include "i2c_master.h"
#include "math.h"
#include "uart.h"

// size of the array (the array that contains S2 S1 S0)
#define arraySize 3

// number of flex sensor (that will be changed)
#define flexNumber 5

// Pin definition
// all the pin used to control : S1, S2 and S3 :

// S0
#define PIN_S0 5
#define PIN_GPIO_MUX5 PERIPHS_IO_MUX_GPIO5_U
#define PIN_GPIO_FUNC5 FUNC_GPIO5

// S1
#define PIN_S1 0
#define PIN_GPIO_MUX0 PERIPHS_IO_MUX_GPIO0_U
#define PIN_GPIO_FUNC0 FUNC_GPIO0

// S2
#define PIN_S2 4
#define PIN_GPIO_MUX4 PERIPHS_IO_MUX_GPIO4_U
#define PIN_GPIO_FUNC4 FUNC_GPIO4

// Pin Number 2 and 14 (only used for testing Servo with PWM library)

#define PWM_2_OUT_IO_MUX PERIPHS_IO_MUX_GPIO2_U
#define PWM_2_OUT_IO_NUM 2
#define PWM_2_OUT_IO_FUNC FUNC_GPIO2

#define PWM_14_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_14_OUT_IO_NUM 14
#define PWM_14_OUT_IO_FUNC FUNC_GPIO14

// Number of channel that need to be initialized when calling "pwm_init"
// function
#define PWM_NUM_CHANNEL_NUM 2

// analog values scale : from 1024 to 0
// when no flex => 1024
// when flex 1024 decrease to 0
#define input_start 1024
#define input_end 0

// PWM values scale : from 0 to 152
#define output_start 0
#define output_end 152

#define delay_time portTICK_RATE_MS

// Task Handlers
xTaskHandle xHandle_get_flex_values;
xTaskHandle xHandle_display_values;
;

// Define some array to organize the Pin (and simplify the code too)
const static uint8_t SxPinArray[3] = {PIN_S0, PIN_S1, PIN_S2};
const static uint SxPinMux[3] = {PERIPHS_IO_MUX_GPIO5_U, PERIPHS_IO_MUX_GPIO0_U,
                                 PERIPHS_IO_MUX_GPIO4_U};
const static char *SxPinFunction[3] = {PIN_GPIO_FUNC5, PIN_GPIO_FUNC0,
                                       PIN_GPIO_FUNC4};

// Some pointers
int *FlexValueArray = NULL;
int *FlexValueArrayCopy = NULL;
int *ServoValueArray = NULL;

// all the S0 S1 S2 code for the multiplexer (that correspond to the code for S0
// S1 S2)

const static int Code_y0[3] = {0, 0, 0};
const static int Code_y1[3] = {1, 0, 0};
const static int Code_y2[3] = {0, 1, 0};
const static int Code_y3[3] = {1, 1, 0};
const static int Code_y4[3] = {0, 0, 1};

// do not modified the function below
uint32 user_rf_cal_sector_set(void) {
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
//***************************************************

// do not modified the function below
void ICACHE_FLASH_ATTR user_rf_pre_init(void) {}

//***************************************************
// display all the  analogic value (use of a task)
void ICACHE_FLASH_ATTR displayValues(void *pvParameters) {

  int *array = NULL;
  int choice = (int)pvParameters;
  if (choice == 1)
    array = (int *)FlexValueArray;
  else
    array = (int *)ServoValueArray;

  while (1) {
    printf("Y0\tY1\tY2\tY3\tY4\t");
    printf("\n");
    printf("---\t---\t---\t---\t---");
    printf("\n");

    for (int i = 0; i < flexNumber; i++) {
      printf("%d\t", array[i]);
    }
    printf("\n");
    vTaskDelay(10);
  }
  vTaskDelete(NULL);
}

//*****************************************************

void ICACHE_FLASH_ATTR displayValuesForProcesssing(void){

  int* array = NULL;
  //no choice the value that we are going to display is the final value (the PWM value)
  array = (int*)ServoValueArray;

  while(1){
    printf("data ");
    for (int i = 0; i < flexNumber; i++) {
      printf("%d ", array[i]);
    }
    printf("\n");
    vTaskDelay(10);
  }
}

//***************************************************

int ICACHE_FLASH_ATTR convert_flex_to_servo(int input) {

  if (input >= 0 && input <= 1024) {
    // mapping from (1024 => 0) to (0 => 102)
    double slope =
        1.0 * (output_end - output_start) / (input_end - input_start);
    int output = output_start + slope * (input - input_start);
    return floor(output);
  }
  return -1;
}

//*******************************************************************************

// function that copy all the value of array2 into array1
void ICACHE_FLASH_ATTR copy_array(int *array1, int *array2) {
  for (int i = 0; i < flexNumber; i++) {
    array1[i] = array2[i];
  }
}

//**********************************************************************************
// get the correct analogic value from the multiplexer
void ICACHE_FLASH_ATTR value_yx(void *pvParameters) {

  // array that will point to the "code" in function of the pvParameters value
  int *tab = NULL;
  // switch with all the case (Y0, Y1, Y2, Y3, Y4)
  switch ((int)pvParameters) {

  case 0:
    tab = (int *)Code_y0;
    break;
  case 1:
    tab = (int *)Code_y1;
    break;
  case 2:
    tab = (int *)Code_y2;
    break;
  case 3:
    tab = (int *)Code_y3;
    break;
  case 4:
    tab = (int *)Code_y4;
    break;
  default:
    break;
  }

  // apply the combination in Pin 5, 0, 4 (respectively: S0 S1 S2)
  GPIO_OUTPUT_SET(SxPinArray[0], tab[0]);
  GPIO_OUTPUT_SET(SxPinArray[1], tab[1]);
  GPIO_OUTPUT_SET(SxPinArray[2], tab[2]);

  // get the analogic value
  FlexValueArray[(int)pvParameters] = system_adc_read();

}

//***************************************************************************************************
// convert each analog value
void ICACHE_FLASH_ATTR get_servo_values(void) {
  for (int i = 0; i < flexNumber; i++) {
    ServoValueArray[i] = convert_flex_to_servo(FlexValueArrayCopy[i]);
  }
}

//**************************************************************************************************
// function that get the final servo values (used by task )
void ICACHE_FLASH_ATTR get_flex_values(void) {

  while (1) {

    for (int i = 0; i < flexNumber; i++) {
      //assign the right code and get the value
      value_yx((void *)i);
    }
    // make a copy of the flex sensors values
    copy_array(FlexValueArrayCopy, FlexValueArray);
    // get the new values (for the servo)
    get_servo_values();
  }
  vTaskDelete(NULL);
}

//************************************************
// function used to turn servo
// to be deleted ...
void ICACHE_FLASH_ATTR turn_servo(void) {
  while (1) {
    // set the duty cycle
    vTaskSuspend(xHandle_get_flex_values);
    vTaskSuspend(xHandle_display_values);
    printf("\ntask1 suspend\n");
    printf("task2 suspend\n");

    // void pwm_set_duty ( uint32 duty, uint8 channel )
    // the duty is the value inside ServoValueArray and the Pin declared in the
    // user_init is the corresponding channel
    pwm_set_duty(ServoValueArray[3], 1);
    pwm_set_duty(ServoValueArray[1], 0);
    // it is only where calling pwm_start() that the 2 line above are setted
    pwm_start();

    // wait 200ms
    vTaskDelay(200 / delay_time);

    printf("task1 resume\n");
    vTaskResume(xHandle_get_flex_values);
    printf("task2 resume\n");
    vTaskResume(xHandle_display_values);
  }
  vTaskDelete(NULL);
}

//***************************************************

void ICACHE_FLASH_ATTR user_init(void) {

  // Pin declaration used for pwm_init
  uint32 io_info[][3] = {
      {PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC, PWM_2_OUT_IO_NUM},
      {PWM_14_OUT_IO_MUX, PWM_14_OUT_IO_FUNC, PWM_14_OUT_IO_NUM}};

  // doesn't matter the value below
  u32 duty[1] = {15};

  /*we initalise the PWM for a period of 20ms (so 20 000 micro-seconds),
    we specify the duty (irrelevant here because we will change it multiple
    times when we will set the value for the servo) we specify the number of
    channel (here only one) and all the information about the pin (that is an
    array of array of size n (the number of channel used and a constant[3])
  */
  pwm_init(20000, duty, PWM_NUM_CHANNEL_NUM, io_info);

  // dynamic allocation of arrays

  // FlexValueArray used to save the flex sensors values (input)
  FlexValueArray = (int *)malloc(flexNumber * sizeof(int));
  // copy of FlexValueArray
  FlexValueArrayCopy = (int *)malloc(flexNumber * sizeof(int));
  // ServoValueArray used to save the servo values (output)
  ServoValueArray = (int *)malloc(flexNumber * sizeof(int));

  // change this value will change values displayed in the Serial Monitor
  // 1 for flex sensors
  // otherwise servo sensors
  int n = 1;

  // Pin definition (we use Digital Pin 5, 0, 4)
  PIN_FUNC_SELECT(PIN_GPIO_MUX5, PIN_GPIO_FUNC5);
  PIN_FUNC_SELECT(PIN_GPIO_MUX0, PIN_GPIO_FUNC0);
  PIN_FUNC_SELECT(PIN_GPIO_MUX4, PIN_GPIO_FUNC4);

  // Tasks

  // one task for all  the flex sensors values
  xTaskCreate(get_flex_values, (signed char *)"updateflexValues", 256, NULL, 2,
              &xHandle_get_flex_values);
              vTaskDelay(10);
  xTaskCreate(displayValuesForProcesssing, (signed char *)"display", 256, (void *)n, 2,
             &xHandle_display_values);
  //  xTaskCreate(ex_servo,(signed char*)"servo",256,NULL,2,NULL);

  //enable the function below to "test" the servo
  //  xTaskCreate(turn_servo,(signed char*)"turnservo",256,NULL,2,NULL);

  /*
//create all the tasks for each
  xTaskCreate(&value_yx, (signed char*)"y0", 256, (void*) 0, 2, NULL);
  xTaskCreate(&value_yx, (signed char*)"y1", 256, (void*) 1, 2, NULL);
  xTaskCreate(&value_yx, (signed char*)"y2", 256, (void*) 2, 2, NULL);
  xTaskCreate(&value_yx, (signed char*)"y3", 256, (void*) 3, 2, NULL);
  xTaskCreate(&value_yx, (signed char*)"y4", 256, (void*) 4, 2, NULL);

  */

  // delay de 500ms
  //  xTaskCreate(&displayValues ,(signed char*)"display" ,256 , NULL ,2 ,NULL);
}
