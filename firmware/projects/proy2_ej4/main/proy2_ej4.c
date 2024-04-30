/*! @mainpage Blinking
 *
 * \section Diseñar e implementar una aplicación, basada en el driver analog_io_mcu.h y el driver de transmisión serie uart_mcu.h, que digitalice una señal analógica y la transmita a un graficador de puerto serie de la PC. Se debe tomar la entrada CH1 del conversor AD y la transmisión se debe realizar por la UART conectada al puerto serie de la PC, en un formato compatible con un graficador por puerto serie. 


 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/04/2024 | Document creation		                         |
 *
 * @author Quiroga Eugenio (Eugeniquirogabio@gmail.com)
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
#include "analog_io_mcu.h"



/*==================[macros and definitions]=================================*/
#define PERIODO_MEDICION 1000000
#define tiempo_de_conversionDA 2000
#define tiempo_de_conversionAD 4000
#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/

TaskHandle_t handle_conversorDA = NULL;
TaskHandle_t handle_conversorAD = NULL;

uint16_t valorAnalogico; // defino variable global


const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};


/*==================[internal functions declaration]=========================*/


static void Escribir(){
UartSendString(UART_PC,"señal es:");
UartSendString(UART_PC,(char *)UartItoa(valorAnalogico,10));
UartSendString(UART_PC,"cm\r\n");

}





static void DA_conversor(){
 uint8_t aux=0;

    while (1){
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
          
          
        if(aux==BUFFER_SIZE){

            aux=0;

       }

        AnalogOutputWrite(ecg[aux]);
        aux++;
        

       

    }
}

static void AD_conversor(){
 

    while (1){
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
     AnalogInputReadSingle(CH1, valorAnalogico);
     Escribir();     
     

    }
}


void FuncConvertirAD(void *param)
{
    vTaskNotifyGiveFromISR(tiempo_de_conversionDA, pdFALSE);

    
}

void FuncConvertirDA(void *param)
{
    vTaskNotifyGiveFromISR(tiempo_de_conversionAD, pdFALSE);

    
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
   

    timer_config_t conv = {
        .timer = TIMER_A,
        .period = tiempo_de_conversionDA,
        .func_p = &FuncConvertirAD,
        .param_p = NULL,
    };
    TimerInit(&conv);


    timer_config_t conv1 = {
        .timer = TIMER_B,
        .period = tiempo_de_conversionAD,
        .func_p = &FuncConvertirAD,
        .param_p = NULL,
    };
    TimerInit(&conv1);
   
 
    xTaskCreate(AD_conversor, "conversor AD", 1024, NULL, 5, handle_conversorAD);
    xTaskCreate(DA_conversor, "Conversor DA", 1024, NULL, 5, handle_conversorDA);
    

    TimerStart(conv.timer);
    TimerStart(conv1.timer);

  serial_config_t my_uart={
        .port=UART_PC,
	    .baud_rate=tiempo_de_conversionAD,					
        .func_p=NULL,
        .param_p=NULL,
    };
    UartInit(&my_uart);

    

      
    analog_input_config_t entrada_analoga = {
        .input= CH1,			/*!< Inputs: CH0, CH1, CH2, CH3 */
	    .mode= ADC_SINGLE,		/*!< Mode: single read or continuous read */
	    .func_p = NULL,			/*!< Pointer to callback function for convertion end (only for continuous mode) */
        .param_p = NULL,			/*!< Pointer to callback function parameters (only for continuous mode) */
	    .sample_frec = NULL,
        };
    
    AnalogInputInit(&entrada_analoga);

    



}