/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define DataPin 21
#define ClockPin 22
#define MSBFIRST 0
#define LSBFIRST 1

void gpio_conf() {
   gpio_config_t gp_conf;
   gp_conf.mode = GPIO_MODE_OUTPUT;
   gp_conf.pin_bit_mask = ((1ULL << DataPin) | (1ULL << clockPin));
   gp_conf.pull_down_en = 0;
   gp_conf.pull_up_en = 1;
   gpio_config(&gp_conf);
}

void shiftOut(int DataPin,int ClockPin, int edian, int command) {
   
   //int l_command = command;
   int shiftbitMSB = 0x80;
   int shiftbitLSB = 0x01;
   switch(edian) {
      case 0:
         for (int i = 0; i < 8; i++) {
            if ((shiftbitMSB&command) != 0) {
               gpio_set_level(DataPin,1);
               gpio_set_level(ClockPin,1); 
            }
            else {
               gpio_set_level(DataPin,0);
               gpio_set_level(ClockPin,1);
            }
            vTaskDelay(1/portTICK_PERIOD_MS);
            gpio_set_level(ClockPin,0);
            shiftbitMSB = shiftbitMSB >> 1;
         }
         break;
      case 1:
         for (int i = 0;i < 8; i++) {
            if ((shiftbitLSB&command) != 0) {
               gpio_set_level(DataPin,1);
               gpio_set_level(ClockPin,1);
            }
            else {
               gpio_set_level(DataPin,1);
               gpio_set_level(ClockPin,1);
            }
            vTaskDeylay(1/portTICK_PERIOD_MS);
            gpio_set_level(ClockPin,0);
            shiftbitLSB = shiftbitLSB << 1;
         }
         break;
   }
}

void sendCommandSHT(int command, int DataPin, int ClockPin) {
   int ack;
   gpio_set_direction(DataPin, GPIO_MODE_OUTPUT);
   gpio_set_direction(ClockPin, GPIO_MODE_OUTPUT);
   gpio_set_level(DataPin, 1);
   gpio_set_level(ClockPin,1);
   gpio_set_level(DataPin,0);
   gpio_set_level(ClockPin,0);
   gpio_set_level(ClockPin,1);
   gpio_set_level(DataPin,1);
   gpio_set_level(ClockPin,0);
   
   shiftOut(DataPin,ClockPin,MSBFIRST,command);
   
   //Verify we get correct ack
   gpio_set_level(ClockPin, 1);
   gpio_set_direction(DataPin, GPIO_MODE_INPUT);
   ack = gpio_get_level(DataPin);
   if (ack != 0) {
      printf("ACK Error 1");
   }
}

void waitForResultSHT(int dataPin) {
   int i;
   int ack;
   gpio_set_direction(DataPin, GPIO_MODE_INPUT);
   
   for(i = 0; i < 100; ++i) {
      vTaskDeylay(10/portTICK_PERIOD_MS);
      ack = gpio_get_level(DataPin);
      
      if (ack == 0) {
         break;
      }
   }
   if (ack == 1) {
      printf("ACK Error 2");
   }
}

int shiftIn(int DataPin,int ClockPin, int numBits) {
   int ret = 0;
   int i;
   
   for (i = 0; i<numBits; i++) {
      gpio_set_level(ClockPin, 1);
      vTaskDeylay(10/portTICK_PERIOD_MS);
      ret = ret*2 + gpio_get_level(DataPin);
      gpio_set_level(ClockPin, 0);
   }
   return(ret);
}

int getData16SHT(int DataPin, int ClockPin) {
   int val;
   
   // Get the most significant bits
   gpio_set_direction(DataPin, GPIO_MODE_INPUT);
   gpio_set_direction(ClockPin, GPIO_MODE_OUTPUT);
   val = shiftIn(DataPin, ClockPin, 8);
   val = val << 8;
   
   //Send the required ack
   gpio_set_direction(DataPin, GPIO_MODE_OUTPUT);
   gpio_set_level(DataPin, 0);
   gpio_set_level(ClockPin, 1);
   gpio_set_level(ClockPin, 0);
   
   //Get the least significant bits
   gpio_set_direction(DataPin, GPIO_MODE_INPUT);
   val |= shiftIn(DataPin,ClockPin, 8);
   
   return val;   
}

void skipCrcSHT(int DataPin, int ClockPin) {
   //Skip CRC
   gpio_set_direction(DataPin, GPIO_MODE_OUTPUT);
   gpio_set_level(DataPin, 0);
   gpio_set_level(ClockPin, 1);
   gpio_set_level(ClockPin, 0);
}

void app_main()
{
   printf("Hello World");
   int gval;
   int gTempCmd = 0b00000011;
   gpio_conf();
   sendCommandSHT(gTempCmd, DataPin, ClockPin);
   waitForResultSHT(DataPin);
   val = getData16SHT(DataPin, ClockPin);
   skipCrcSHT(DataPin, ClockPin);
   printf("Value: %04x", gval);
}
