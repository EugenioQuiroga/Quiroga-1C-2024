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
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	gpio_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (uint8_t i = digits; i > 0; i--)
	{

		bcd_number[i - 1] = data % 10;
		data = data / 10;
	}
}

void modificar_estado(uint32_t dato, gpioConf_t *vector_gpios)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		if (dato & (1 << i))
		{
			GPIOOn(vector_gpios[i].pin);
		}
		else
		{
			GPIOOff(vector_gpios[i].pin);
		}
	}
}

void display_leds(uint32_t dato, uint digitos, gpioConf_t *vectorgpios, gpioConf_t *vectorgpio_map)
{

	uint8_t arreglo[digitos];
	convertToBcdArray(dato, digitos, arreglo);
	for (int i = 0; i < 3; i++)
	{ // defino digitos=3 en on/off(pulso)
		modificar_estado(arreglo[i], vectorgpios);

		GPIOOn(vectorgpio_map[i].pin);
		GPIOOff(vectorgpio_map[i].pin);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t data = 543;
	uint8_t digitos = 3;
	uint8_t vector_bcd[digitos];

	convertToBcdArray(data, digitos, vector_bcd);

	for (int i = 0; i < digitos; i++)
	{

		printf("valor : %d", vector_bcd[i]);
	}

	gpioConf_t vectorgpios[4];
	vectorgpios[0].pin = GPIO_20;
	vectorgpios[1].pin = GPIO_21;
	vectorgpios[2].pin = GPIO_22;
	vectorgpios[3].pin = GPIO_23;
	vectorgpios[0].dir = GPIO_OUTPUT;
	vectorgpios[1].dir = GPIO_OUTPUT;
	vectorgpios[2].dir = GPIO_OUTPUT;
	vectorgpios[3].dir = GPIO_OUTPUT;

	for (uint h = 0; h < digitos; h++)
	{

		GPIOInit(vectorgpios[h].pin, vectorgpios[h].dir);
	}

//	for (uint j = 0; j < digitos; j++)
//	{

//		modificar_estado(vector_bcd[j], vectorgpios);
//	}

	// creo vector de mapeo
	gpioConf_t vectorgpio_map[3];

	
	
	vectorgpio_map[0].pin = GPIO_19;
	vectorgpio_map[1].pin = GPIO_18;
	vectorgpio_map[2].pin = GPIO_9;
	
	vectorgpio_map[0].dir = GPIO_OUTPUT;
	vectorgpio_map[1].dir = GPIO_OUTPUT;
	vectorgpio_map[2].dir = GPIO_OUTPUT;
	
	 
	 for (uint r = 0; r < digitos; r++)
	{

		GPIOInit(vectorgpio_map[r].pin, vectorgpio_map[r].dir);
	}
	 
	 
	 display_leds(data,digitos,vectorgpios,vectorgpio_map);




}
/*==================[end of file]============================================*/