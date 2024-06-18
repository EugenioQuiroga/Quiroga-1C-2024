

/*! @mainpage Ejemplo Bluetooth - Filter
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
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "neopixel_stripe.h"
#include "ble_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"

#include "iir_filter.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1
#define SAMPLE_FREQ 100
#define T_SENIAL 10000
#define CHUNK 4
#define TAMANO 256
/*==================[internal data definition]===============================*/
float ecg[4];
static float ecg_filt[CHUNK];

static float ecg_filtrado[TAMANO];

TaskHandle_t fft_task_handle = NULL;
bool filter = false;

float ultimo_valor = 0;
int u = 0;
int aux = 0;
float Ntiempo = 1;
int p = 0;
// bool filter_cero = true;
float F_cardiaca = 1;
float Delta_t = 1;
uint16_t valorAnalogico;
float valor;
int indice=0;
// printf("valor2222 :\n");
/*==================[internal functions declaration]=========================*/

/*void maximos()
{
    char msg_chunk1[24];
    char msg1[128];
    p = 0;
    strcpy(msg1, "");
    while (p < 255)
    {

        if (p == 255)
        {
            ultimo_valor = ecg_filtrado[p];
        }

        if (ecg_filtrado[p] > 50)
        {
            if (((p == 0) && ((ecg_filtrado[p] - ultimo_valor) > 0) && ((ecg_filtrado[p] - ecg_filtrado[p + 1]) > 0)) || ((p != 0) && ((ecg_filtrado[p] - ecg_filtrado[p - 1]) > 0) && ((ecg_filtrado[p] - ecg_filtrado[p + 1]) > 0)))
            {

                Ntiempo = aux;
                aux = 0;
            }
        }
        aux++;
        p++;

        // printf("valor :%.2f\n",Ntiempo);
    }

    sprintf(msg_chunk1, "*X%.2f*", Ntiempo);
    strcat(msg1, msg_chunk1);

    // F_cardiaca = (1 / (Ntiempo * Delta_t));

    BleSendString(msg1);
}
*/
void read_data(uint8_t *data, uint8_t length)
{
    switch (data[0])
    {
    case 'A':
        filter = true;
        break;
    case 'a':
        filter = false;
        break;
        /*  case 'H':
              filter_cero = true;
              break;
          case 'h':
              filter_cero = false;
              break;
          */
    }
}

void FuncTimerSenial(void *param)
{
    xTaskNotifyGive(fft_task_handle);
}

static void FftTask(void *pvParameter)
{
    char msg[128];
    char msg_chunk[24];
    
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        
        
        AnalogInputReadSingle(CH1, &valorAnalogico);
        ecg[indice] = valorAnalogico;
        indice++;
            
        

        if (indice == 4)
        {
            if (filter)
            {
                HiPassFilter(&ecg[0], ecg_filt, CHUNK);
                LowPassFilter(ecg_filt, ecg_filt, CHUNK);
            }
            else
            {
                memcpy(ecg_filt, &ecg[0], CHUNK * sizeof(float));
            }

            /*   if(filter_cero == true){
                       for(int c=0; c<4; c++){
                           if(ecg_filt[c]<200){
                               ecg_filt[c]=0;
                           }

                       }


               }      */
            // funcion para hacer todo cero los valores leidos y visualizarlos se quedaria con el pico QRS(solo con los 4 valores de ecg_filt[])

            strcpy(msg, "");
            for (uint8_t i = 0; i < CHUNK; i++)
            {
                // if(filter == true){
                //      ecg_filtrado[u]=ecg_filt[i];    //guardo el doble de las muestras ya que en 256 tengo solo QRS
                //      u++;
                //     if(u==255){
                //         maximos();//llamar a calcular maximos
                //         u=0;}
                //    }
          
                sprintf(msg_chunk, "*G%.2f*", ecg_filt[i]);
                strcat(msg, msg_chunk);
            }
           

            BleSendString(msg);

            
            indice=0;
        }
        
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    uint8_t blink = 0;
    static neopixel_color_t color;
    ble_config_t ble_configuration = {
        "CintaDeCorrer",
        read_data};
    timer_config_t timer_senial = {
        .timer = TIMER_B,
        .period = T_SENIAL,
        .func_p = FuncTimerSenial,
        .param_p = NULL};

    NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
    NeoPixelAllOff();
    TimerInit(&timer_senial);
    LedsInit();
    LowPassInit(SAMPLE_FREQ, 30, ORDER_2);
    HiPassInit(SAMPLE_FREQ, 1, ORDER_2);
    BleInit(&ble_configuration);

    xTaskCreate(&FftTask, "FFT", 4096, NULL, 5, &fft_task_handle);
    TimerStart(timer_senial.timer);

    analog_input_config_t entrada_analoga = {
        .input = CH1,       // Inputs: CH1
        .mode = ADC_SINGLE, // Mode: single read or continuous read
        .func_p = NULL,     // Pointer to callback function for convertion end (only for continuous mode)
        .param_p = NULL,    // Pointer to callback function parameters (only for continuous mode)
        .sample_frec = 0,
    };

    AnalogInputInit(&entrada_analoga);

    while (1)
    {
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch (BleStatus())
        {
        case BLE_OFF:
            NeoPixelAllOff();
            break;
        case BLE_DISCONNECTED:
            if (blink % 2)
            {
                NeoPixelAllColor(NEOPIXEL_COLOR_BLUE);
            }
            else
            {
                NeoPixelAllOff();
            }
            blink++;
            break;
        case BLE_CONNECTED:
            NeoPixelAllColor(NEOPIXEL_COLOR_BLUE);
            break;
        }
    }
}