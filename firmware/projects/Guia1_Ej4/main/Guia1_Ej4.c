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
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include <led.h>
#include <switch.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (uint8_t i = digits;i>0; i--)
	{

		bcd_number[i-1] = data % 10;
		data = data / 10;
	}
	
}
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;




void modificar_estado(uint32_t dato, gpioConf_t *vector_gpios  )
{	
	uint32_t debug_var;
	for(uint8_t i=0;i<4;i++)
	{
		debug_var = dato&(1<<i);
		if(debug_var)
		{
			GPIOOn(vector_gpios[i].pin);
		}
		else{ GPIOOff(vector_gpios[i].pin);}
	}
}
	




/*==================[external functions definition]==========================*/
void app_main(void)
{


	gpioConf_t vectorgpios [4];
	vectorgpios[0].pin=GPIO_20;
	vectorgpios[1].pin=GPIO_21;
	vectorgpios[2].pin=GPIO_22;
	vectorgpios[3].pin=GPIO_23;
	vectorgpios[0].dir=GPIO_OUTPUT;
	vectorgpios[1].dir=GPIO_OUTPUT;
	vectorgpios[2].dir=GPIO_OUTPUT;
	vectorgpios[3].dir=GPIO_OUTPUT;

	uint32_t data=356;
	uint8_t digitos=3;
	uint8_t vector_bcd[digitos]; 

	convertToBcdArray(data, digitos, vector_bcd);

	

	modificar_estado(vector_bcd[1], vectorgpios);


	
}
/*==================[end of file]============================================*/