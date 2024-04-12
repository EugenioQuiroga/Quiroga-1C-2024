/*! @mainpage Proyecto 1- Ejercicio 3
 *
 * @section El Proyecto 1 
 *
 * Consignas:
 * Realice un función que reciba un puntero a una estructura LED como la que se muestra a continuación: 
 *	struct leds
 *	{
 *  	uint8_t mode;       ON, OFF, TOGGLE
 *	    uint8_t n_led;        indica el número de led a controlar 
 *   	uint8_t n_ciclos;   indica la cantidad de ciclos de ncendido/apagado
 *	    uint16_t periodo;    indica el tiempo de cada ciclo
 *	 } my_leds; 
 *
 *  Para ver las condiciones del mapa a seguir para el desarrollo de la actividad ver imagen de actidad 3 de  https://docs.google.com/document/d/1f4OtorkZ1hOFu-jOo0Uhmz-cJnUvV05Z-eNhs19IVZM/edit
 *
 *
 * |   Fecha	    | Descripcion                                    |
 * |:----------:|:-----------------------------------------------|
 * | 4/04/2024 | Ejercicio 3 proyecto 1	                         |
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
/** @fn my_leds 
 * @brief Estructura que modela un led  
 * @param mode uint8_t modo 
 * @param n_led uint8_t numero de led
 * @param n_ciclos uint8_t numero de cicloes de encendido/ apagado
 * @param periodo uint8_t indica el tiempo de cada ciclo
 */
struct leds
{
    uint8_t mode;     //  ON, OFF, TOGGLE
	uint8_t n_led;      //  indica el número de led a controlar
	uint8_t n_ciclos;  // indica la cantidad de ciclos de encendido/apagado
	uint8_t periodo;   // indica el tiempo de cada ciclo
} my_leds;
/** @def MODO_1 
* @brief define el modo en ON 
*/

#define MODO_1 1
/** @def MODO_0 
* @brief define el modo en OFF 
*/
#define MODO_0 0
/** @def MODO_T 
* @brief define el modo en TOGGLE 
*/
#define MODO_T 3

/** @def LED1 
* @brief define el led 1 con el numero 1 
*/
#define LED1 1
/** @def LED2 
* @brief define el led 2 con el numero 2 
*/
#define LED2 2
/** @def LED3 
* @brief define el led 3 con el numero 3 
*/
#define LED3 3
/** @def CONFIG_BLINK_PERIOD 
* @brief define el tiempo del periodo  
*/

#define CONFIG_BLINK_PERIOD 100

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
/** @fn void controlar_led(struct leds *ptr_led)
 * @brief Fucion que a traves de un puntero del tipo struct leds controla que hacer con 3 leds distintos y combinacion de los mismos a traves de 3 modos distintos. 
 * Modo ON o 1 ,toma el numero de led y lo enciende. 
 * Modo OFF o 0, toma el numero del led y lo apaga o pone en 0.
 * Modo 3 o toggle mira el led que tiene cargado lo hace parpadear y le aplica un retardo.  
 * @param ptr_led puntero del tipo struct leds 
 * @return 
 */
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
/** @fn void app_main(void)
 * @brief Funcion principal del programa. 
 * @return 
 */
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