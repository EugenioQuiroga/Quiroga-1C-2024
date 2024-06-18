/*! @mainpage Proyecto 2
 *
 * \section genDesc General Description
 *
 *  Este codigo es una aplicacion para sensar la temperatura basado en un sensor de distancia y de temperatura con un sistema de informado de la misma mediante UART y buzzer
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 16/04/2024 | Document creation		                         |
 *
 * @author Quiroga Eugenio (eugenioquirogabio@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "gpio_mcu.h"
#include "analog_io_mcu.h"
#include "switch.h"
#include "uart_mcu.h"
#include "buzzer.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
/**
 * @def PERIODO_MEDICION_Distancia Periodo de medicion distancia  
*/
#define PERIODO_MEDICION_DISTANCIA 1000000  // lee cada un segundo
/**
 * @def PERIODO_MEDICION_Temperatura Periodo de medicion temperatura  
*/
#define PERIODO_MEDICION_TEMPERATURA 100000 // lee cada 100 ms ya que son 10 mediciones de 100ms o sea durante 1 seg


/**
 * @brief Atributo para tarea de medir distancia
*/
TaskHandle_t medirD_task_handle = NULL;

/**
 * @brief Atributo para tarea de medir Temperatura
*/
TaskHandle_t  medirT_task_handle = NULL;

// Defino los GPIOS para el sensado de la distancia
/**
 * @brief Atributo GPIO para el sensor de distancia
*/
gpio_t ECHO = GPIO_3;
/**
 * @brief Atributo GPIO para el sensor de distancia
*/
gpio_t TRIGGER = GPIO_2;

/**
 * @brief Atributo bool para guardar como condicion la distancia optima
*/

bool distancia_optima = false;  // para definir si la distancia s la optima
/**
 * @brief Atributo vector flotante para almacenar 10 valores de temperatura
*/
float temperatura[10];
/**
 * @brief Atributo bool para reinicio
*/
bool reiniciar=false;

int indice = 0; // para recorrer temperatura
/**
 * @brief valor analogico para almacenar el valor de entrada analogico
*/
uint16_t aux_valor_analogico;
/**
 * @brief Atributo flotante para guardar la temperatura promedio
 * */
float promedio_temperatura;

/**
 * @brief Atributo flotante para guardar la distancia
 * */
float distancia; // defino variable global la distncia sensada

/**
 * @fn void sensado_distancia(void *param)
 * @brief Sensa la distancia , encendiendo un led correspondiente a 3 tipos de distancias y ademas actualiza unavariable global que determina si la distancia es la optima para medir  
*/
void sensado_distancia(void *param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        
        distancia = HcSr04ReadDistanceInCentimeters(); // modifico la variable global distancia 
                if(distancia>140){
                    reiniciar=true;}
        
        if (distancia < 8 )
        {
            printf("LED_1 ON\n");
            LedOn(LED_1);

            printf("LED_2 OFF\n");
            LedOff(LED_2);

            printf("LED_3 OFF\n");
            LedOff(LED_3);
            distancia_optima = false;
        }

        if (distancia > 12)
        {
            printf("LED_1 OFF\n");
            LedOff(LED_1);

            printf("LED_2 OFF\n");
            LedOff(LED_2);

            printf("LED_3 ON\n");
            LedOn(LED_3);
            distancia_optima = false;
        }

        if (distancia > 8 && distancia < 12)
        {
            printf("LED_1 OFF\n");
            LedOff(LED_1);

            printf("LED_2 ON\n");
            LedOn(LED_2);

            printf("LED_3 OFF\n");
            LedOff(LED_3);

            distancia_optima = true;
        }
    }
}
/**
 * @fn void sensado_distancia(void *param)
 * @brief Sensa la temperatura , encendiendo un buzzer para promedios altos, tambien calcula el promedio de temperatura y la envia bajo formato por la uart  
*/
void sensado_temperatura(void *param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (distancia_optima == true && reiniciar==true)
        {
            // Guardo en vector temperatura (10 valores entre 0-3300 mv)
            AnalogInputReadSingle(CH1, &aux_valor_analogico);
            temperatura[indice] = 20 + (((50 - 20) / 3300) * aux_valor_analogico);
            indice++;

        }
        if (indice == 9)
        {
            promedio_temperatura = 0;
            // calculo el promedio y lo guardo en variable GLOBAL promedio
            for (int i = 0; i < 10; i++)
            {
                promedio_temperatura = ((promedio_temperatura + temperatura[i]) / i);

                // mando el valor de temperatura por uart con su distancia
                UartSendString(UART_PC, (char *)UartItoa(promedio_temperatura, 10));
                UartSendString(UART_PC, (char *)"°C persona a ");
                UartSendString(UART_PC, (char *)"\r\n");
                UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
                UartSendString(UART_PC, (char *)"\r\n");
               // Enciendo el buzzer si el promedio es alto; 
                if(promedio_temperatura>37.5){
                    BuzzerOn();
                    BuzzerPlayTone(50, 1000);}
                else {BuzzerOff();}
            }
        }
    }
}


void sensar_T(void *param)
{
    vTaskNotifyGiveFromISR(medirT_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}
void sensar_D(void *param)
{
    vTaskNotifyGiveFromISR(medirD_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    // Definimos los timers
    // timer para sensar distancia
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = PERIODO_MEDICION_DISTANCIA,
        .func_p = sensar_D,
        .param_p = NULL};
    TimerInit(&timer_1);
    TimerStart(timer_1.timer);

    // timer para sensar temperatura
    timer_config_t timer_2 = {
        .timer = TIMER_B,
        .period = PERIODO_MEDICION_TEMPERATURA,
        .func_p = sensar_T,
        .param_p = NULL};
    TimerInit(&timer_2);
    TimerStart(timer_2.timer);

    // entrada analogica de la temperatura
    analog_input_config_t entrada_analoga = {
        .input = CH1,       // Inputs: CH1
        .mode = ADC_SINGLE, // Mode: single read or continuous read
        .func_p = NULL,     // Pointer to callback function for convertion end (only for continuous mode)
        .param_p = NULL,    // Pointer to callback function parameters (only for continuous mode)
        .sample_frec = 0,
    };

    AnalogInputInit(&entrada_analoga);

    // para el sensor de ultrasonido
    HcSr04Init(ECHO, TRIGGER);
    // Inicializo los leds
    LedsInit();
    //inicializo el buzzer
    BuzzerInit(GPIO_18);
    // inicializacion de UART
    serial_config_t my_UART = {
        .port = UART_PC,
        .baud_rate = 115200, // unidad de transmision de datos, vel de l a se;al
        .func_p = NULL,
        .param_p = NULL};

    UartInit(&my_UART);

    xTaskCreate(sensado_distancia, "sensa la distancia", 4096, NULL, 5, &medirD_task_handle);
    xTaskCreate(sensado_temperatura, "sensa la distancia", 4096, NULL, 5, &medirT_task_handle);

   
}