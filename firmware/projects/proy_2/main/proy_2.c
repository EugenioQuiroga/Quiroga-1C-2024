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


/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1 1000
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;
gpio_t ECHO = GPIO_2;
gpio_t TRIGGER = GPIO_3;
bool TEC1=false, TEC2=false;
uint8_t teclas;
 
/*==================[internal data definition]===============================*/
uint16_t distancia;//defino variable global
/*==================[internal functions declaration]=========================*/



static void Medir(){
    while(1){
        
      distancia=HcSr04ReadDistanceInCentimeters();//modifico la variable global distancia
      vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
}
    
}



static void cargarLCD(){

    while(1){
         if(TEC1==true && TEC2==false){
             LcdItsE0803Off();
             LedsOffAll();
        }
        if(TEC1==false && TEC2==false){
            LcdItsE0803Write(distancia); 
        }
        if(TEC1==0 && TEC2==1){
            LcdItsE0803Write(distancia);
        }
        if(TEC2==1){

        }
     vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    }
   
}


static void cargar_teclas(void){

teclas  = SwitchesRead();

 while(1)
	{
		
		switch(teclas)
		{
		    case 1:
			    TEC1=!TEC1;
			    break;

		    case 2:
			    TEC2=!TEC2;
			    break;
		}
        
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);

    } 
    
}
static void modificar_leds(){
    while(1){ 
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
            
        
        }
        if(distancia>20 && distancia<30){
            printf("LED_1 ON\n");
            LedOn(LED_1);
            
        
        
        }
        if(distancia>30){
            printf("LED_1 ON\n");
            LedOn(LED_1);
               

        } 
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    HcSr04Deinit();
    HcSr04Init(ECHO, TRIGGER);

    xTaskCreate(&cargar_teclas, "Cargar estado de switch", 512, NULL, 5, &led1_task_handle);
    xTaskCreate(&Medir, "Medir", 512, NULL, 5, &led1_task_handle);
    xTaskCreate(&cargarLCD, "cargar_LCD", 512, NULL, 5, &led2_task_handle);
    xTaskCreate(&modificar_leds, "Modifica_Estados", 512, NULL, 5, &led3_task_handle);
  
   
           
        
                       
            

        
    
}