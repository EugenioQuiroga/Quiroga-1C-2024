/*! @mainpage Proyecto 1- Ejercicios 4, 5 y 6
 *
 * @section El Proyecto 1
 * Consignas:
 * 4)Escribir una función que reciba un dato de 32 bits, la cantidad de dígitos de salida 
 * y un puntero a un arreglo donde se almacenen los n dígitos. 
 * La función convierte el dato recibido a BCD, guardando cada uno de los dígitos de salida
 * en el arreglo pasado como puntero.
 * 5) Escribir una función que reciba como parámetro un dígito BCD y un vector de estructuras
 *  del tipo gpioConf_t.
 * 6)Escribir una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida y 
 * dos vectores de estructuras del tipo  gpioConf_t. Uno  de estos vectores es igual al 
 * definido en el punto anterior y el otro vector mapea los puertos con el dígito del LCD a 
 * donde mostrar un dato.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_20	 	| 	GPIO_OUTPUT	|
 * | 	PIN_21	 	| 	GPIO_OUTPUT	|
 * | 	PIN_22	 	| 	GPIO_OUTPUT	|
 * | 	PIN_23	 	| 	GPIO_OUTPUT	|
 * | 	PIN_9	 	| 	GPIO_OUTPUT |
 * | 	PIN_18	 	| 	GPIO_OUTPUT	|
 * | 	PIN_19	 	| 	GPIO_OUTPUT	|
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Fecha	    | Descripcion                                    |
 * |:----------:|:-----------------------------------------------|
 * | 3/04/2024 | Ejercicio 4 proyecto 1	                         |
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

/** @fn gpioConf_t 
 * @brief Estructura que contiene un pin y una direccion 
 * @param pin GPIO numero de pin
 * @param dir GPIO direccion  '0' entrada ;  '1' salida
 * @return 
 */
// Estructura del ejercicio 4
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	gpio_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/** @fn void convertToBcdArray(uint32_t data, uint8_t digits , uint8_t *bcd_number)
 * @brief Funcion que convierte un entero sin signo de 32 bits a un arreglo enteros sin signo de 8 bits en BCD
 * @param data numero del elemento que quiero convertir a bcd
 * @param digits numero de digitos del elemento data
 * @param bcd_number puntero a vector donde voy a guardar los bcd
 * @return 
 */

void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (uint8_t i = digits; i > 0; i--)
	{

		bcd_number[i - 1] = data % 10;
		data = data / 10;
	}
}
/** @fn void modificar_estado(uint32_t dato, gpioConf *vector_gpios)
 * @brief Funcion que a traves de un entero sin signo de 32 bits y una mascara, modifica el estado del pin de un vector de gpioConfig_t
 * @param dato entero de 32 bits
 * @param vector_gpios puntero a vector de elementos del tipo gpioConfig_t 
 * @return 
 */
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
/** @fn void display_leds(uint32_t dato, uint digitos ,gpioConf *vectorgpios,gpioConf *vectorgpio_map )
 * @brief  La función que recibe un dato de 32 bits, la cantidad de dígitos de salida y dos vectores de estructuras
 *  del tipo  gpioConf_t. El puntero al vector (gpioConf_t *vectorgpio_map) mapea los puertos con el dígito del LCD 
 * a donde se desea mostrar un dato 
 * @param dato entero de 32 bits
 * @param digitos entero sin signo contiene el numero de digitos  
 * @param vectorgipios puntero a vector de elementos del tipo gpioConfig_t
 * @param vectorgipio_map puntero a vector de elementos del tipo gpioConfig_t para mapear los puertos  
 * 
 * @return 
 */

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
/**
 * @brief Funcion principal del programa.
 * @return 0 
 */
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

		GPIOInit(vectorgpios[h].pin, vectorgpios[h].dir);//inicializo
	}
//(Ejercicio 5-instancia modificar_estado)
	for (uint j = 0; j < digitos; j++)
	{

		modificar_estado(vector_bcd[j], vectorgpios);
	}

//Para ejercicio 6
	
	gpioConf_t vectorgpio_map[3];// creo vector de mapeo

	
	
	vectorgpio_map[0].pin = GPIO_19;
	vectorgpio_map[1].pin = GPIO_18;
	vectorgpio_map[2].pin = GPIO_9;
	
	vectorgpio_map[0].dir = GPIO_OUTPUT;
	vectorgpio_map[1].dir = GPIO_OUTPUT;
	vectorgpio_map[2].dir = GPIO_OUTPUT;
	
	 
	 for (uint r = 0; r < digitos; r++)
	{

		GPIOInit(vectorgpio_map[r].pin, vectorgpio_map[r].dir);  //inicializo
	}
	 
	 
	 display_leds(data,digitos,vectorgpios,vectorgpio_map);




}
/*==================[end of file]============================================*/