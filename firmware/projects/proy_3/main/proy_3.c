

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
 * @author Quiroga Eugenio (Eugenioquirogabio@gmail.com)
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
#include "gpio_mcu.h"
#include "pwm_mcu.h"
#include "iir_filter.h"
/*==================[macros and definitions]=================================*/
/** @def CONFIG_BLINK_PERIOD
 *  @brief configuracion de parpadeo del led Bluetooth
 */
#define CONFIG_BLINK_PERIOD 500
/** @def LED_BT LED_1
*  @brief Led del bluetooth
 */
#define LED_BT LED_1
/** @def SAMPLE_FREQ 100
 *  @brief Frecuencia utilizada par filtros en base a cada cuanto se toma la medicion de la señal ECG
 */
#define SAMPLE_FREQ 100
/** @def T_SENIAL
 *  @brief Tiempo (us) que establece el periodo de muestreo de la señal ECG
 */
#define T_SENIAL 10000
/** @def CHUNK 
 *  @brief Tamaño de muestas para filtrado 
 */
#define CHUNK 8
/** @def MINUTO
 *  @brief Tiempo (us) de un segundo
 */
#define MINUTO 60000000
/** @def T_INFORMAR
 *  @brief Tiempo para en el que se llama a la tarea de mostrar la lectura de lo que se quiere muestrear
 */
#define T_INFORMAR 3000000


// *****Cuando lo hago por pwm****
// #define vel_motor_FC GPIO_22

//**** cuando lo hago con reles****

/** @defvel_motor_FC_baja GPIO_19
 *  @brief Salida para rele que activa motor para frecuencias bajas
 */
#define vel_motor_FC_baja GPIO_19
/** @def vel_motor_FC_estandar GPIO_21
 *  @brief Salida para rele que activa motor para frecuencias estandar
 */
#define vel_motor_FC_estandar GPIO_21
/** @def vel_motor_FC_alta GPIO_18
 *  @brief Salida  para rele que activa motor para frecuencias altas
 */
#define vel_motor_FC_alta GPIO_18
/*==================[internal data definition]===============================*/
/** @brief  estado variable para encendido y apagado de la aplicacion*/
bool estado = false;

/** @brief ecg[CHUNK] Variable para almacenar 8 lecturas que seran procesadas */
float ecg[CHUNK];

/** @brief ecg_filt[CHUNK] Variable para guardar las muestaras fitradas */
static float ecg_filt[CHUNK];

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea de procesar */
TaskHandle_t fft_task_handle = NULL;
/** @brief filter Variable para decidir si filtrar la selñal o no */
bool filter = false;

/** @brief Objeto de tipo TaskHandle_t que se asocia con la tarea de informar */
TaskHandle_t Informar_task_handle = NULL;

/** @brief FC_INF Variable para setear y fijar la variable de frecuencia inferior */
float FC_INF = 65.0;

/** @brief  FC_SUP Variable para setear y fijar la variable de frecuencia superior*/
float FC_SUP = 85.0;

/** @brief  Ntiempo numero de veces que se tomo la lectura entre picos R*/
float Ntiempo = 1;

/** @brief  F_cardiaca Variable para guardar la frecuencia cardiaca calculada */
float F_cardiaca = 1;

/** @brief  cond_Frec Variable  */
bool cond_Frec = false;

/** @brief RR veces que no toma un pico E una vez detectado uno (tiempo de espera para comenzar una nueva lectura) */
int RR = 0;

int u = 0;   //variable auxiliar para contador  
int aux = 0;    //variable auxiliar para contador
int p = 0;          //variable auxiliar para contador
int indice = 0; //Indice para tarea de Procesado de la señal

/** @brief valorAnalogico Variable que se utiliza para almacenar los valores que se convierten de analgico a digital */
uint16_t valorAnalogico;


//**********Para la opcion de PWM ya que la fncion recibe un entero de 8 bits********
//uint8_t velocidad_aux = 0;
/*==================[internal functions declaration]=========================*/

/** @fn maximos()
 * @brief Funcion que bucas los Maximos carga la frecuencia en la variable global F_cardiaca
 */
void maximos()
{

    char msg1[128];

    strcpy(msg1, ""); // probar comentar
    for (int i = 0; i < CHUNK; i++)
    {
        if (ecg_filt[i] > 250 && RR > 25)
        {
            Ntiempo = aux + 25;
            aux = 0;
            RR = 0;
            F_cardiaca = MINUTO / (Ntiempo * T_SENIAL);
            sprintf(msg1, "*X%.2f*", F_cardiaca);
            BleSendString(msg1);
        }

        aux++;
    }
}

/** @fn read_data(uint8_t *data, uint8_t length)
 * @brief Funcion que recibe las ordenes de bluetooth y toma decisiones sobre que modificar 
 */
void read_data(uint8_t *data, uint8_t length)
{
    if (data[0] == 'O')
    {
        estado = true;
    }
    if (data[0] == 'o')
    {
        estado = false;
        GPIOOn(vel_motor_FC_baja);
        GPIOOn(vel_motor_FC_estandar);
        GPIOOn(vel_motor_FC_alta);

        LedOff(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
    if (estado == true)
    {
        char msg2[128];

        char msg3[128];

        switch (data[0])
        {
        case 'A':
            filter = true;
            break;
        case 'a':
            filter = false;
            break;

        case 'B':

            cond_Frec = true;

            break;
        case 'b':
            cond_Frec = false;
            break;
        case 'C':

            strcpy(msg2, "");
            if (cond_Frec == true)
            {
                FC_SUP = FC_SUP + 10;
            }
            sprintf(msg2, "*V%.2f*", FC_SUP);
            BleSendString(msg2);

            break;

        case 'D':
            strcpy(msg2, "");
            if (cond_Frec == true)
            {
                FC_SUP = FC_SUP - 10;
            }
            sprintf(msg2, "*V%.2f*", FC_SUP);
            BleSendString(msg2);

            break;
        case 'E':
            strcpy(msg2, "");
            if (cond_Frec == true)
            {
                FC_SUP = FC_SUP + 1;
            }
            sprintf(msg2, "*V%.2f*", FC_SUP);
            BleSendString(msg2);

            break;

        case 'F':
            strcpy(msg2, "");
            if (cond_Frec == true)
            {
                FC_SUP = FC_SUP - 1;
            }
            sprintf(msg2, "*V%.2f*", FC_SUP);
            BleSendString(msg2);

            break;

        case 'G':
            strcpy(msg3, "");
            if (cond_Frec == true)
            {
                FC_INF = FC_INF + 10;
            }
            sprintf(msg3, "*M%.2f*", FC_INF);
            BleSendString(msg3);

            break;
        case 'H':
            strcpy(msg3, "");
            if (cond_Frec == true)
            {
                FC_INF = FC_INF - 10;
            }
            sprintf(msg3, "*M%.2f*", FC_INF);
            BleSendString(msg3);

            break;
        case 'I':
            strcpy(msg3, "");
            if (cond_Frec == true)
            {
                FC_INF = FC_INF + 1;
            }
            sprintf(msg3, "*M%.2f*", FC_INF);
            BleSendString(msg3);
            printf("%.2f\n", FC_INF);
            break;
        case 'J':
            strcpy(msg3, "");
            if (cond_Frec == true)
            {
                FC_INF = FC_INF - 1;
            }
            sprintf(msg3, "*M%.2f*", FC_INF);
            BleSendString(msg3);

            break;
        }
    }
}


void FuncTimerSenial(void *param)
{
    xTaskNotifyGive(fft_task_handle);
}
/** @fn FftTask(void *pvParameter)
 * @brief Tarea para realizar el procesamiento de la señal 
 */
static void FftTask(void *pvParameter)
{
    char msg[128];
    char msg_chunk[24];

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (estado == true)
        {
            AnalogInputReadSingle(CH1, &valorAnalogico);
            ecg[indice] = valorAnalogico;
            indice++;

            if (indice == 8)
            {

                HiPassFilter(&ecg[0], ecg_filt, CHUNK);
                LowPassFilter(ecg_filt, ecg_filt, CHUNK);

                if (filter == false)
                {
                    for (int c = 0; c < CHUNK; c++)
                    {
                        if (ecg_filt[c] < 300)
                        {
                            ecg_filt[c] = 0;
                        }
                    }
                }
                // funcion para hacer todo cero los valores leidos y visualizarlos se quedaria con el pico QRS(solo con los 4 valores de ecg_filt[])

                strcpy(msg, "");
                for (uint8_t i = 0; i < CHUNK; i++)
                {

                    RR++;

                    if (RR > 25 && i == 7)
                    {
                        maximos();
                    }
                    sprintf(msg_chunk, "*G%.2f*", ecg_filt[i]);
                    strcat(msg, msg_chunk);
                }

                BleSendString(msg);

                indice = 0;
            }
        }
    }
}
void FuncTimerInformar(void *param)
{
    xTaskNotifyGive(Informar_task_handle);
}
/** @fn Informar(void *param)
 * @brief Tarea para informar las acciones tomadas 
 */
void Informar(void *param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (estado == true)
        {
            if (F_cardiaca < FC_INF)
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
                GPIOOff(vel_motor_FC_baja); // velocidad mas alta
                GPIOOn(vel_motor_FC_estandar);
                GPIOOn(vel_motor_FC_alta);

                // seteo de velocidad en FC_baja
                /*    velocidad_aux=velocidad_aux + 10;
                    PWMSetDutyCycle(PWM_1, velocidad_aux);
                    PWMOn(PWM_0);
                */
            }
            if (F_cardiaca > FC_INF && F_cardiaca < FC_SUP)
            {
                LedOff(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
                GPIOOn(vel_motor_FC_baja); // velocidad
                GPIOOff(vel_motor_FC_estandar);
                GPIOOn(vel_motor_FC_alta);
                // seteo de velocidad en FC_estandar no cambio nada
                /*  PWMSetDutyCycle(PWM_1, velocidad_aux);
                  PWMOn(PWM_0);
              */
            }
            if (F_cardiaca > FC_SUP)
            {
                LedOff(LED_1);
                LedOff(LED_2);
                LedOn(LED_3);
                GPIOOn(vel_motor_FC_baja); // velocidad
                GPIOOn(vel_motor_FC_estandar);
                GPIOOff(vel_motor_FC_alta);
                // seteo de velocidad en FC_alta
                /*   velocidad_aux=velocidad_aux - 10;
                   PWMSetDutyCycle(PWM_1, velocidad_aux);
                   PWMOn(PWM_0);
               */
            }
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
    // Timer para la tarea de procesar
    timer_config_t timer_senial = {
        .timer = TIMER_B,
        .period = T_SENIAL,
        .func_p = FuncTimerSenial,
        .param_p = NULL};

    TimerInit(&timer_senial);

    // Timer para la tarea de informar
    timer_config_t timer_informar = {
        .timer = TIMER_A,
        .period = T_INFORMAR,
        .func_p = FuncTimerInformar,
        .param_p = NULL};

    TimerInit(&timer_informar);
    /*------------------Dejo comentado porque no tenfgo los driver para motor------------*/
    // Inicializo los GPIO en salida para los motores.
    // GPIOInit(vel_motor_FC, GPIO_OUTPUT);

    // Inicializo PWM
    // PWMInit(PWM_0, vel_motor_FC, 0);
    /*------------------------------------------------------------------------------------*/
    // Para cuando lo hago por rele
    GPIOInit(vel_motor_FC_baja, GPIO_OUTPUT);
    GPIOInit(vel_motor_FC_estandar, GPIO_OUTPUT);
    GPIOInit(vel_motor_FC_alta, GPIO_OUTPUT);

    NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
    NeoPixelAllOff();

    LedsInit();
    LowPassInit(SAMPLE_FREQ, 20, ORDER_4);
    HiPassInit(SAMPLE_FREQ, 1, ORDER_4);
    BleInit(&ble_configuration);

    xTaskCreate(&Informar, "Prende las salidas segun la FC", 4096, NULL, 5, &Informar_task_handle);
    xTaskCreate(&FftTask, "FFT", 4096, NULL, 5, &fft_task_handle);
    TimerStart(timer_senial.timer);
    TimerStart(timer_informar.timer);
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