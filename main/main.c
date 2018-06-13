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

#define DATAPIN 18
#define CLOCKPIN 19
#define MSBFIRST 0
#define LSBFIRST 1

void shiftOut(int dataPin, int clockPin, int edian, int command);
void sendCommandSHT(int command, int dataPin, int clockPin);
void waitForResultSHT(int dataPin);
int shiftIn(int dataPin, int clockPin, int numbits);

void gpio_conf() {
   gpio_config_t gp_conf;
   gp_conf.mode = GPIO_MODE_OUTPUT;
   gp_conf.pin_bit_mask = ((1ULL << DATAPIN) | (1ULL << CLOCKPIN));
   gp_conf.pull_down_en = 0;
   gp_conf.pull_up_en = 1;
   gpio_config(&gp_conf);
}

void shiftOut(int dataPin,int clockPin, int edian, int command) {
   
   //int l_command = command;
   int shiftbitMSB = 0x80;
   int shiftbitLSB = 0x01;
   switch(edian) {
      case 0:
         for (int i = 0; i < 8; i++) {
            if ((shiftbitMSB&command) != 0) {
               gpio_set_level(dataPin,1);
               gpio_set_level(clockPin,1); 
            }
            else {
               gpio_set_level(dataPin,0);
               gpio_set_level(clockPin,1);
            }
            //vTaskDelay(1/portTICK_PERIOD_MS);
            gpio_set_level(clockPin,0);
            shiftbitMSB = shiftbitMSB >> 1;
         }
         break;
      case 1:
         for (int i = 0;i < 8; i++) {
            if ((shiftbitLSB&command) != 0) {
               gpio_set_level(dataPin,1);
               gpio_set_level(clockPin,1);
            }
            else {
               gpio_set_level(dataPin,1);
               gpio_set_level(clockPin,1);
            }
            vTaskDelay(1/portTICK_PERIOD_MS);
            gpio_set_level(clockPin,0);
            shiftbitLSB = shiftbitLSB << 1;
         }
         break;
   }
}

void sendCommandSHT(int command, int dataPin, int clockPin) {
   int ack;
   gpio_set_direction(dataPin, GPIO_MODE_OUTPUT);
   gpio_set_direction(clockPin, GPIO_MODE_OUTPUT);
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
   gpio_set_direction(dataPin, GPIO_MODE_INPUT);
   ack = gpio_get_level(dataPin);
   if (ack != 0) {
      printf("ACK Error 0");
   }
   gpio_set_level(clockPin,0);
   ack = gpio_get_level(dataPin);
   if (ack != 1) {
      printf("ACK Error 1");
   }
}

void waitForResultSHT(int dataPin) {
   int i;
   int ack;
   gpio_set_direction(dataPin, GPIO_MODE_INPUT);
   
   for(i = 0; i < 100; ++i) {
      vTaskDelay(10/portTICK_PERIOD_MS);
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
   
   for (i = 0; i<numBits; ++i) {
      gpio_set_level(clockPin, 1);
      vTaskDelay(10/portTICK_PERIOD_MS);
      ret = ret*2 + gpio_get_level(dataPin);
      gpio_set_level(clockPin, 0);
   }
   return(ret);
}

int getData16SHT(int dataPin, int clockPin) {
   int val;
   
   // Get the most significant bits
   gpio_set_direction(dataPin, GPIO_MODE_INPUT);
   gpio_set_direction(clockPin, GPIO_MODE_OUTPUT);
   val = shiftIn(dataPin, clockPin, 8);
   val = val*256;
   printf("1st byte: %04x\n",val);
   //Send the required ack
   gpio_set_direction(dataPin, GPIO_MODE_OUTPUT);
   gpio_set_level(dataPin, 0);
   gpio_set_level(clockPin, 1);
   gpio_set_level(clockPin, 0);
   
   //Get the least significant bits
   gpio_set_direction(dataPin, GPIO_MODE_INPUT);
   val |= shiftIn(dataPin, clockPin, 8);
   
   return val;   
}

void skipCrcSHT(int dataPin, int clockPin) {
   //Skip CRC
   gpio_set_direction(dataPin, GPIO_MODE_OUTPUT);
   gpio_set_level(dataPin, 0);
   gpio_set_level(clockPin, 1);
   gpio_set_level(clockPin, 0);
}

void app_main()
{
   printf("Hello World");
   const float c1 = -4.0;
   const float c2 = 0.0405;
   const float c3 = -0.0000028;
   const float d1 = -39.7;
   const float d2 = 0.01;
   const float T1 = 0.01;
   const float T2 = 0.00008;

   int gHval=0;
   int gTval=0;
   int gHudCmd = 0b00000101;
   int gTempCmd = 0b00000011;
   float compTval=0;
   float compLHval=0;
   float compRHval=0;
   gpio_conf();
   while(1) {
	printf("Send temperature reading command\n");
   	sendCommandSHT(gTempCmd, DATAPIN, CLOCKPIN);
	printf("wait for result\n");
   	waitForResultSHT(DATAPIN);
	printf("get data\n");
  	gTval = getData16SHT(DATAPIN, CLOCKPIN);
   	skipCrcSHT(DATAPIN, CLOCKPIN);
   	printf("Value: %04x\n", gTval);
	compTval = (gTval * d2)+d1;
	printf("Temperature: %.4f 'C\n", compTval);
	//Read humidity value
	vTaskDelay(10/portTICK_PERIOD_MS);
	printf("Send humidity reading command\n");
	sendCommandSHT(gHudCmd, DATAPIN, CLOCKPIN);
	printf("wait for result\n");
	waitForResultSHT(DATAPIN);
	printf("get data\n");
	gHval = getData16SHT(DATAPIN, CLOCKPIN);
	skipCrcSHT(DATAPIN, CLOCKPIN);
	printf("Value: %04x\n", gHval);
	compLHval = c1 + c2*gHval + c3*gHval*gHval;
	printf("Compensated Humidity: %.4f\n", compLHval);
	compRHval = (compTval - 25.0) * (T1 + T2*gHval) + compLHval;
	printf("Corrected Humidity Value: %.4f\n\n",compRHval);
	vTaskDelay(2000/portTICK_PERIOD_MS);

   }
}
