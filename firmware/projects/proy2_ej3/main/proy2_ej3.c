/*! @mainpage Proyecto 2
 *
 * \section genDesc General Description
 *
 * Este codigo es un detector de proximidad dando respuestas diferentes ante una magnitud definda por rangos de distancias
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
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
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"



/*==================[macros and definitions]=================================*/
#define PERIODO_MEDICION 1000000

/*==================[internal data definition]===============================*/

TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;
TaskHandle_t led4_task_handle = NULL;
gpio_t ECHO = GPIO_3;
gpio_t TRIGGER = GPIO_2;
bool TEC1 = false, TEC2 = false;
uint8_t teclas;
uint16_t distancia; // defino variable global
/*==================[internal functions declaration]=========================*/


static void Escribir(){
UartSendString(UART_PC,"distancia");
UartSendString(UART_PC,(char *)UartItoa(distancia,10));
UartSendString(UART_PC,"cm\r\n");

}
void FuncTimermedir(void *param)
{
    vTaskNotifyGiveFromISR(led2_task_handle, pdFALSE);
    vTaskNotifyGiveFromISR(led3_task_handle, pdFALSE);
    vTaskNotifyGiveFromISR(led4_task_handle, pdFALSE);
}

static void Medir(void *p)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto, ya que sacamos la vttaskdelay y colocamos un timer*/

        if (TEC1 == false)
        {
            distancia = HcSr04ReadDistanceInCentimeters(); // modifico la variable global distancia
        }
    }
}

static void cargarLCD(void *p)
{

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // printf("%d\n", distancia);
        if (TEC1 == true)
        {
            LcdItsE0803Off();
            LedsOffAll();
        }
        if (TEC1 == false && TEC2 == true)
        {
            LcdItsE0803Write(distancia);
        }
        
         Escribir();
    }
}

static void cargar_TCL1(void *p)
{
    TEC1 = !TEC1;
}
static void cargar_TCL2(void *p)
{
    TEC2 = !TEC2;
}

static void modificar_leds(void *p)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (TEC1 == false)
        {

            if (distancia < 10)
            {
                printf("LED_1 OFF\n");
                LedOff(LED_1);

                printf("LED_2 OFF\n");
                LedOff(LED_2);

                printf("LED_3 OFF\n");
                LedOff(LED_3);
            }
            if (distancia > 10 && distancia < 20)
            {
                printf("LED_1 ON\n");
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            if (distancia > 20 && distancia < 30)
            {
                printf("LED_1, LED_2 ON\n");
                LedOn(LED_2);
                LedOn(LED_1);
                LedOff(LED_3);
            }
            if (distancia > 30)
            {
                printf("LED_1, LED_2,LED_3 ON\n");
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        }
        if (TEC1 == true)
        {
            LedsOffAll();
        }
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(ECHO, TRIGGER);
    LcdItsE0803Init();
    SwitchesInit();

    timer_config_t medir = {
        .timer = TIMER_A,
        .period = PERIODO_MEDICION,
        .func_p = FuncTimermedir,
        .param_p = NULL,
    };
    TimerInit(&medir);
    // timer_config_t cargar_lcd = {
    //     .timer = TIMER_A,
    //     .period = PERIODO_MEDICION,
    //     .func_p = FuncTimerlcd,
    //     .param_p = NULL,
    // };
    // TimerInit(&cargar_lcd);

    // timer_config_t mod_led = {
    //     .timer = TIMER_A,
    //     .period = PERIODO_MEDICION,
    //     .func_p = FuncTimermodled,
    //     .param_p = NULL,
    // };
    // TimerInit(&mod_led);

    // si hay una interrupcion con algun swtitch se ejecuta esta linea
    SwitchActivInt(SWITCH_1, &cargar_TCL1, NULL);
    SwitchActivInt(SWITCH_2, &cargar_TCL2, NULL);

    // xTaskCreate(cargar_teclas, "Cargar estado de switch", 1024, NULL, 5, &led1_task_handle);
    xTaskCreate(Medir, "Medir", 1024, NULL, 5, &led2_task_handle);
    xTaskCreate(cargarLCD, "cargar_LCD", 1024, NULL, 5, &led3_task_handle);
    xTaskCreate(modificar_leds, "Modifica_Estados", 1024, NULL, 5, &led4_task_handle);

    TimerStart(medir.timer);

    serial_config_t my_uart={
        .port=UART_PC,
	    .baud_rate=115200,					
        .func_p=Escribir,
        .param_p=NULL,
};

UartInit(&my_uart);
}