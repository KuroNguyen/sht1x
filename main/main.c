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

#define DATAPIN 21
#define CLOCKPIN 22
#define MSBFIRST 0
#define LSBFIRST 1

void gpio_conf() {
   gpio_config_t gp_conf;
   gp_conf.mode = GPIO_MODE_OUTPUT;
   gp_conf.pin_bit_mask = ((1ULL << dataPin) | (1ULL << clockPin));
   gp_conf.pull_down_en = 0;
   gp_conf.pull_up_en = 1;
   gpio_config(&gp_conf);
}

void shiftOut(int dataPin,int clockPin, int edian, int command) {
   //int l_command = command;
   switch(edian) {
      case 0:
         int shiftbit = 0x80;
         for (int i = 0; i < 8; i++) {
            if ((shiftbit&command) != 0) {
               gpio_set_level(dataPin,1);
               gpio_set_level(clockPin,1); 
            }
            else {
               gpio_set_level(dataPin, 0);
               gpio_set_level(clockPin, 1);
            }
            vTaskDeylay(1/portTICK_PERIOD_MS);
            gpio_set_level(clockPin, 0);
            shiftbit = shiftbit >> 1;
         }
         break;
      case 1:
         int shiftbit = 0x01;
         for (int i = 0;i < 8; i++) {
            if ((shiftbit&command) != 0) {
               gpio_set_level(dataPin,1);
               gpio_set_level(clockPin,1);
            }
            else {
               gpio_set_level(dataPin,1);
               gpio_set_level(clockPin,1);
            }
            vTaskDeylay(1/portTICK_PERIOD_MS);
            gpio_set_level(clockPin,0);
            shiftbit = shiftbit << 1;
         }
         break;
   }
}

void sendCommandSHT(int command, int dataPin, int clockPin) {
   int ack;
   gpio_set_direction(dataPin, output_only);
   gpio_set_direction(clockPin, output_only);
   gpio_set_level(dataPin, 1);
   gpio_set_level(clockPin,1);
   gpio_set_level(dataPin,0);
   gpio_set_level(clockPin,0);
   gpio_set_level(clockPin,1);
   gpio_set_level(dataPin,1);
   gpio_set_level(clockPin,0);
   
   shiftOut(dataPin,clockPin,MSBFIRST,command);
   
   //Verify we get correct ack
   gpio_set_level(clockPin, 1);
   gpio_set_direction(dataPin, input_only);
   ack = gpio_get_level(dataPin);
   if (ack != 0) {
      printf("ACK Error 1");
   }
}

void waitForResultSHT(int dataPin) {
   int i;
   int ack;
   gpio_set_direction(dataPin, input_only);
   
   for(i = 0; i < 100; ++i) {
      vTaskDeylay(10/portTICK_PERIOD_MS);
      ack = gpio_get_level(dataPin);
      
      if (ack == 0) {
         break;
      }
   }
   if (ack == 1) {
      printf("ACK Error 2");
   }
}

int shiftIn(int dataPin,int clockPin, int numBits) {
   int ret = 0;
   int i;
   
   for (i = 0; i<numBits; i++) {
      gpio_set_level(clockPin, 1);
      vTaskDeylay(10/portTICK_PERIOD_MS);
      ret = ret*2 + gpio_get_level(dataPin);
      gpio_set_level(clockPin, 0);
   }
   return(ret);
}

int getData16SHT(int dataPin, int clockPin) {
   int val;
   
   // Get the most significant bits
   gpio_set_direction(dataPin, input_only);
   gpio_set_direction(clockPin, output_only);
   val = shiftIn(dataPin, clockPin, 8);
   val = val << 8;
   
   //Send the required ack
   gpio_set_direction(dataPin, output_only);
   gpio_set_level(dataPin, 0);
   gpio_set_level(clockPin, 1);
   gpio_set_level(clockPin, 0);
   
   //Get the least significant bits
   gpio_set_direction(dataPin, input_only);
   val |= shiftIn(dataPin,clockPin, 8);
   
   return val;   
}

void skipCrcSHT(int dataPin, int clockPin) {
   //Skip CRC
   gpio_set_direction(dataPin, output_only);
   gpio_set_level(dataPin, 0);
   gpio_set_level(clockPin, 1);
   gpio_set_level(clockPin, 0);
}

void app_main()
{
   printf("Hello World");
   int gval;
   int gTempCmd = 0b00000011;
   gpio_conf();
   sendCommandSHT(gTempCmd, dataPin, clockPin);
   waitForResultSHT(dataPin);
   val = getData16SHT(dataPin, clockPin);
   skipCrcSHT(dataPin, clockPin);
   printf("Value: %04x", gval);
}
