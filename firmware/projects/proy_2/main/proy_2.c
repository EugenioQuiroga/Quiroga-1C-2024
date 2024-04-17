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
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"


/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1 1000
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;
TaskHandle_t led4_task_handle = NULL;
gpio_t ECHO = GPIO_3;
gpio_t TRIGGER = GPIO_2;
bool TEC1=false, TEC2=false;
uint8_t teclas;
 
/*==================[internal data definition]===============================*/
uint16_t distancia;//defino variable global
/*==================[internal functions declaration]=========================*/

static void Medir(void * p)
{
    while (1)
    {
        if (TEC1 == false && TEC2 == false)
        {
            distancia = HcSr04ReadDistanceInCentimeters(); // modifico la variable global distancia
        }
        if (TEC1 == false && TEC2 == true)
        {
        }

        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    }
}

static void cargarLCD(void * p)
{

    while (1)
    {
        //printf("%d\n", distancia);
        if (TEC1 == true)
        {
            LcdItsE0803Off();
            LedsOffAll();
        }
        if (TEC1 == false && TEC2 == false)
        {
            LcdItsE0803Write(distancia);
        }
        if (TEC1 == false && TEC2 == true)
        {
            LcdItsE0803Write(distancia);
        }
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    }
}

static void cargar_teclas(void * p)
{

    while (1)
    {
        teclas = SwitchesRead();
        switch (teclas)
        {
        case 1:
            printf("tecla1\n");
            TEC1 = !TEC1;
            break;

        case 2:
            printf("tecla2\n");
            TEC2 = !TEC2;
            break;
        }

        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    }
}
static void modificar_leds(void * p){
    while(1){ 
        
     if(TEC1==false){

        if(distancia<10){
            printf("LED_1 OFF\n");
            LedOff(LED_1);        
        

            printf("LED_2 OFF\n");
            LedOff(LED_2);
        

            printf("LED_3 OFF\n");
            LedOff(LED_3);
            
        }
        if(distancia>10 && distancia<20){
            printf("LED_1 ON\n");
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);

            
        
        }
        if(distancia>20 && distancia<30){
            printf("LED_1, LED_2 ON\n");
            LedOn(LED_2);
            LedOn(LED_1);
            LedOff(LED_3);
        
        }
        if(distancia>30){
            printf("LED_1, LED_2,LED_3 ON\n");
            LedOn(LED_1);
            LedOn(LED_2);
            LedOn(LED_3);
               

        } 
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
     }
        if(TEC1==true){
            LedsOffAll();
        }
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    HcSr04Init(ECHO, TRIGGER);
    LcdItsE0803Init();
    SwitchesInit();

    xTaskCreate(cargar_teclas, "Cargar estado de switch", 1024, NULL, 5, &led1_task_handle);
    xTaskCreate(Medir, "Medir", 1024, NULL, 5, &led2_task_handle);
    xTaskCreate(cargarLCD, "cargar_LCD", 1024, NULL, 5, &led3_task_handle);
    xTaskCreate(modificar_leds, "Modifica_Estados", 1024, NULL, 5, &led4_task_handle);
  
           
        
                       
            

        
    
}