/* Device Sketch for hand sanitizer station
 *
 * */

#include "LoRaWan_APP.h"
#include "Arduino.h"

/*
 * set LoraWan_RGB to 1,the RGB active in loraWan
 * RGB red means sending;
 * RGB green means received done;
 */
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

//define LoRa constaints
#define RF_FREQUENCY                                915000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // 125 kHz                                                                                                                         
#define LORA_SPREADING_FACTOR                       7         
#define LORA_CODINGRATE                             1                                                                                                                         
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

//Define variables for deep sleep and timer wakeup
#define timetillsleep 5000
#define timetillwakeup 5000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;
double counter;

//Define storage containers for communication packets
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

double txNumber;

int16_t rssi,rxSize;
void  DoubleToString( char *str, double double_num,unsigned int len);

char sensorLocation[] = "monech";

void setup() {
    boardInitMcu( );
    Serial.begin(115200); //DBG

    txNumber=0;
    rssi=0;
    counter=0;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
    //Radio.Sleep( );
    //TimerInit( &sleep, onSleep );
    //TimerInit( &wakeUp, onWakeUp );
    //onSleep();
   }



void loop()
{
  if(lowpower){
    //note that lowPowerHandler() runs six times before the mcu goes into lowpower mode;
    lowPowerHandler();
  }
  //delay(1000);
  //txNumber += 0.01;
  //sendMessage();
}

void sendMessage()
{
  sprintf(txpacket,"%s", sensorLocation);
//sprintf(txpacket,"%s","Hello world number");  //start a package
//sprintf(txpacket+strlen(txpacket),"%d",txNumber); //add to the end of package
  
  DoubleToString(txpacket,txNumber,3);     //add to the end of package
  
  turnOnRGB(COLOR_SEND,0); //change rgb color

  Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

  Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out  
}//end sendMessage

/*
 * Sets the development board into the low power deep
 * sleep while the device is not in use. It utilizes the
 * RTC clock to set off a timerInterupt to wake up the device
 * after the timer period defined in timetillwakeup.
 * 
 * @param void
 * @return N/A
 */
void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}//end onSleep


void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}//end onWakeUp


/**
  * @brief  Double To String
  * @param  str: Array or pointer for storing strings
  * @param  double_num: Number to be converted
  * @param  len: Fractional length to keep
  * @retval None
  */
void  DoubleToString( char *str, double double_num,unsigned int len) { 
  double fractpart, intpart;
  fractpart = modf(double_num, &intpart);
  fractpart = fractpart * (pow(10,len));
  sprintf(str + strlen(str),"%d", (int)(intpart)); //Integer part
  sprintf(str + strlen(str), ".%d", (int)(fractpart)); //Decimal part
}
