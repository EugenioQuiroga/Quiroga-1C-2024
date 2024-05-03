/*! @mainpage Proyecto 2 ejercicio 3-b
 *
 * @section m Proyecto 2
 * 
 * Este codigo es un detector de proximidad dando respuestas diferentes ante una magnitud definda por rangos de distancias
 * En el mismo se procede a mediante interrupciones y otras herramientas a leer una entrada y cambiar la posicion de las variables que modifican el estado de una medicacion mediante leds.
 * Ademas, mediante un  sensor de ultrasonido tomamos la distancia y la visualizamos en consola o terminal y un lcd.
 * 
 *
 * |   Date	    | 
 * |:----------:|
 * | 30/04/2024 | 
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
/** @def PERIODO_MEDICION 
* @brief define el periodo de medicion  
*/
#define PERIODO_MEDICION 1000000

/*==================[internal data definition]===============================*/

TaskHandle_t task_handle_1 = NULL;
TaskHandle_t task_handle_2 = NULL;
TaskHandle_t task_handle_3 = NULL;


gpio_t ECHO = GPIO_3;
gpio_t TRIGGER = GPIO_2;

bool TEC1 = false, TEC2 = false;

uint8_t teclas;
uint16_t distancia; // defino variable global

/*==================[internal functions declaration]=========================*/

/** @fn  void Escribir()
 * @brief Escribe en la uart la distancia 
 * @return 
 */
void Escribir(){
UartSendString(UART_PC,"distancia");
UartSendString(UART_PC,(char *)UartItoa(distancia,10));
UartSendString(UART_PC,"cm\r\n");

}


/** @fn void FuncTimermedir(void *param)
 * @brief envia notificacion para medir
 * @return 
 */

    void FuncTimermedir(void *param)
{
    vTaskNotifyGiveFromISR(task_handle_1, pdFALSE);
    vTaskNotifyGiveFromISR(task_handle_2, pdFALSE);
    vTaskNotifyGiveFromISR(task_handle_3, pdFALSE);
}


/** @fn void Medir(void *param)
 * @brief Mide la distancia bajo la condicion de que la tecla 1 no este activa  
 * @return 
 */

void Medir(void *p)
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
/** @fn  void consola()
 * @brief Consola lee la letra O o la letra H y cambia la posicion de la tecla uno o dos segun corresponda 
 * @return 
 */

 void consola(){
    uint8_t letras;
    UartReadByte(UART_PC, &letras);
    switch(letras){
    case 'O': 
         TEC1 = !TEC1;
         break;
    
     case 'H': 
         TEC2 = !TEC2;
         break;
    }
}

/** @fn void cargarLCD(void *p)
 * @brief Cargar lcd se encarga de mostrar  la distancia si la tecla 1 esta en alto y la tecla dos en bajo por el lcd. En el caso de estar presionada la tecla uno lo que realizara es apagar la pantalla lcd. Tambien realiza el llamado a la uart.
 *  @return 
 */
 void cargarLCD(void *p)
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
/** @fn void cargar_TCL1(void *p)
 * @brief cambia la posicion de la tecla uno 
 *  @return 
 */
 void cargar_TCL1(void *p)
{
    TEC1 = !TEC1;
}
/** @fn void cargar_TCL2(void *p)
 * @brief cambia la posicion de la tecla dos 
 *  @return 
 */
void cargar_TCL2(void *p)
{
    TEC2 = !TEC2;
}

/** @fn void modificar_leds(void *p)
 * @brief Modifica el estado de los leds leyendo la distancia. 
 *  @return 
 */

 void modificar_leds(void *p)
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
   
    // si hay una interrupcion con algun swtitch se ejecuta esta linea
    SwitchActivInt(SWITCH_1, &cargar_TCL1, NULL);
    SwitchActivInt(SWITCH_2, &cargar_TCL2, NULL);

    // xTaskCreate(cargar_teclas, "Cargar estado de switch", 1024, NULL, 5, &led1_task_handle);
    xTaskCreate(Medir, "Medir", 2024, NULL, 5, &task_handle_1);
    xTaskCreate(cargarLCD, "cargar_LCD", 2024, NULL, 5, &task_handle_2);
    xTaskCreate(modificar_leds, "Modifica_Estados", 2024, NULL, 5, &task_handle_3);

    TimerStart(medir.timer);

    serial_config_t my_uart={
        .port=UART_PC,
	    .baud_rate=115200,					
        .func_p=consola,
        .param_p=NULL,
};

UartInit(&my_uart);



}