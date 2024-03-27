/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Fecha	    | Descripcion                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/03/2024 | Ejercicio 3 proyecto 1	                         |
 *
 * @author Quiroga Eugenio  (eugenioquirogabio@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <led.h>
#include "switch.h"
/*==================[macros and definitions]=================================*/
struct leds
{
    uint8_t mode;     //  ON, OFF, TOGGLE
	uint8_t n_led;      //  indica el número de led a controlar
	uint8_t n_ciclos;  // indica la cantidad de ciclos de ncendido/apagado
	uint8_t periodo;   // indica el tiempo de cada ciclo
} my_leds;

#define MODO_1 1
#define MODO_0 0
#define MODO_T 3
#define LED1 1
#define LED2 2
#define LED3 3
#define PERIOD_UNO 
#define PERIOD_UNO 
#define PERIOD_UNO 
#define CONFIG_BLINK_PERIOD 100

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void controlar_led(struct leds *ptr_led){


switch(ptr_led->mode){
case  MODO_1: 
				if(ptr_led->n_led==LED1){
					LedOn(LED_1);
				}
				else if(ptr_led->n_led==LED2){
					LedOn(LED_2);
				}
				else {if(ptr_led->n_led==LED3) LedOn(LED_3);}



break;
case  MODO_0: 
					if(ptr_led->n_led==LED1){
					LedOff(LED_1);
				}
				else if(ptr_led->n_led==LED2){
					LedOff(LED_2);
				}
				else {if(ptr_led->n_led==LED2) LedOff(LED_3);}

break;
case MODO_T:
	uint8_t i = 0;
	while (i < ptr_led->n_ciclos)
	{

		if (ptr_led->n_led == LED1)
		{
			LedToggle(LED_1);
		}
		else if (ptr_led->n_led == LED2)
		{
			LedToggle(LED_2);
		}
		else
		{
			if (ptr_led->n_led == LED3)
				LedToggle(LED3);
		}

		for (uint16_t retardo = 0; retardo < ptr_led->periodo / CONFIG_BLINK_PERIOD; retardo++)
		{
			vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		}

		i++;
	}
	break;
}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	my_leds.mode = MODO_T;
	my_leds.n_ciclos = 10;
	my_leds.periodo = 500;
	my_leds.n_led = LED2;
	controlar_led(& my_leds);
	
	

}
/*==================[end of file]============================================*/