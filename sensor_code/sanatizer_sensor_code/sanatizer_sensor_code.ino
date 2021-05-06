/* Heltec Automation send communication test example
 *
 * Function:
 * 1. Send data from a CubeCell device over hardware 
 * 
 * 
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/ASR650x-Arduino
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

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

double txNumber;

int16_t rssi,rxSize;
void  DoubleToString( char *str, double double_num,unsigned int len);

char deviceId[] = "1";
#define SENSOR_DATA GPIO0

//Define variables for deep sleep and timer wakeup
#define timetillsleep 10000
#define timetillwakeup 10000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;
double counter;

void setup() {
    boardInitMcu( );
    pinMode(SENSOR_DATA,INPUT);
    //Setup pins for the liquid sensor,Vext=PWR and Port30=Data
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext,HIGH); //Start with sensor off 
    
    //delay due to board reading inactive pins high during startup
    delay(5000);
    
    Serial.begin(115200);;
    txNumber=0;
    rssi=0;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
    //Start the device with the radio off
    Radio.Sleep( );
    //Initialize the timers
    TimerInit( &sleep, onSleep );
    TimerInit( &wakeUp, onWakeUp );
    onSleep();
    
   }//end setup


/*
 * onSleep puts the device into low power mode and will 
 * go to sleep until the time indicated in timetillwakeup
 * (ms) passes and onWakeUp is called
 * 
 * @param N/A
 */
void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}//end onSleep

/*
 * onWakeUp wakes up the device from deep sleep, and then
 * powers the sensor and reads its data, if the container 
 * is empty it will send a LoRa packet with sendMessage()
 * 
 * @param N/A
 */
void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;
  digitalWrite(Vext,LOW); //Turn sensor on
  delay(1000); //Allow time for sensor to power on
  //If tank is empty, send message
  if(digitalRead(SENSOR_DATA)==LOW){
    sendMessage();
    delay(1000); //allow message to have time to send
    Radio.Sleep();
    }//end if

  digitalWrite(Vext,HIGH); //Turn sensor off
  
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}//end onWakeUp

void loop()
{
   if(lowpower){
    //note that lowPowerHandler() runs six times before the mcu goes into lowpower mode;
    lowPowerHandler();
  }//end if
}//end loop


/*
 * sendMessage()
 * 
 * Sends a LoRa packet with the deviceId to tell
 * the base station that "deviceId" is out of sanatizer.
 * 
 * @param N/A
 */
void sendMessage(){
  txNumber += 0.01;
  sprintf(txpacket,"%s", deviceId);  //start a package
  sprintf(txpacket+strlen(txpacket),"%s","%is_empty"); //add to the end of package
  
  //turnOnRGB(COLOR_SEND,0); //change rgb color to red, off to save power
  Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
  
  Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out
  }

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
