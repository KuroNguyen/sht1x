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

#define _dataPin 21
#define _clockPin 22
#define MSBFIRST 0
#define LSBFIRST 1

void gpio_conf() {
   gpio_config_t gp_conf;
   gp_conf.mode = GPIO_MODE_OUTPUT;
   gp_conf.pin_bit_mask = ((1ULL << _dataPin) | (1ULL << _clockPin));
   gp_conf.pull_down_en = 0;
	gp_conf.pull_up_en = 1;
	gpio_config(&gp_conf);
}

void shiftOut(int _dataPin,int _clockPin, int edian, int _command) {
   //int l_command = command;
   switch(edian) {
      case 0:
         int shiftbit = 0x80;
         for (int i = 0; i < 8; i++) {
            if (shiftbit&_command !0) {
               gpio_set_level(_dataPin,1);
               gpio_set_level(_clockPin,1); 
            }
            else {
               gpio_set_level(_dataPin,0);
               gpio_set_level(_clockPin,1);
            }
            vTaskDeylay(1/portTICK_PERIOD_MS);
            gpio_set_level(_clockPin,0);
            shiftbit = shiftbit >> 1;
         }
         break;
      case 1:
         int shiftbit = 0x01;
         for (int i = 0;i < 8; i++) {
            if (shiftbit&_command !0) {
               gpio_set_level(_dataPin,1);
               gpio_set_level(_clockPin,1);
            }
            else {
               gpio_set_level(_dataPin,1);
               gpio_set_level(_clockPin,1);
            }
            vTaskDeylay(1/portTICK_PERIOD_MS);
            gpio_set_level(_clockPin,0);
            shiftbit = shiftbit << 1;
         }
         break;
   }
}

void sendCommandSHT(int _command, int _dataPin, int _clockPin) {
   int ack;
   gpio_set_direction(_dataPin, output_only);
   gpio_set_direction(_clockPin, output_only);
   gpio_set_level(_dataPin, 1);
   gpio_set_level(_clockPin,1);
   gpio_set_level(_dataPin,0);
   gpio_set_level(_clockPin,0);
   gpio_set_level(_clockPin,1);
   gpio_set_level(_dataPin,1);
   gpio_set_level(_clockPin,0);
   
   shiftOut(_dataPin,_clockPin,edian,_command);
   
   //Verify we get correct ack
   gpio_set_level(_clockPin, 1);
   gpio_set_direction(_dataPin, input_only);
   ack = gpio_get_level(_dataPin);
   if (ack != 0) {
      printf("ACK Error 1");
   }
}

void waitForResultSHT(int _dataPin) {
   int i;
   int ack;
   gpio_set_direction(_dataPin, input_only);
   
   for(i = 0; i < 100; ++i) {
      vTaskDeylay(10/portTICK_PERIOD_MS);
      ack = gpio_get_level(_dataPin);
      
      if (ack == 0) {
         break;
      }
   }
   if (ack == 1) {
      printf("ACK Error 2");
   }
}

int shiftIn(int _dataPin,int _clockPin, int _numBits) {
   int ret = 0;
   int i;
   
   for (i = 0; i<_numBits; i++) {
      gpio_set_level(_clockPin, 1);
      vTaskDeylay(10/portTICK_PERIOD_MS);
      ret = ret*2 + gpio_get_level(_dataPin);
      gpio_set_level(_clockPin, 0);
   }
   return(ret);
}

int getData16SHT(int _dataPin, int _clockPin) {
   int val;
   
   // Get the most significant bits
   gpio_set_direction(_dataPin, input_only);
   gpio_set_direction(_clockPin, output_only);
   val = shiftIn(_dataPin, _clockPin, 8);
   val = val << 8;
   
   //Send the required ack
   gpio_set_direction(_dataPin, output_only);
   gpio_set_level(_dataPin, 0);
   gpio_set_level(_clockPin, 1);
   gpio_set_level(_clockPin, 0);
   
   //Get the least significant bits
   gpio_set_direction(_dataPin, input_only);
   val |= shiftIn(_dataPin,_clockPin, 8);
   
   return val;   
}

void skipCrcSHT(int _dataPin, int _clockPin) {
   //Skip CRC
   gpio_set_direction(_dataPin, output_only);
   gpio_set_level(_dataPin, 0);
   gpio_set_level(_clockPin, 1);
   gpio_set_level(_clockPin, 0);
}

void app_main()
{
   printf("Hello World");
   int _val;
   int _gTempCmd = 0b00000011;
   gpio_conf();
   sendCommandSHT(_gTempCmd, _dataPin, _clockPin);
   waitForResultSHT(_dataPin);
   _val = getData16SHT(_dataPin, _clockPin);
   skipCrcSHT(_dataPin, _clockPin);
   printf("Value: %04x", _val);
}
