#include "application.h"
#include "PietteTech_DHT.h"
#include "DS18B20.h"

#define DHTTYPE  DHT11             // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   3         	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   4000  // Sample Air Temp every 4 seconds

DS18B20 ds18b20 = DS18B20(D2);

void dht_wrapper();
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

bool firstPass = true;
char szInfo[75];
char status[64];
byte dsAddr;
float pubTemp;
double celsius;
double fahrenheit;
float airtemp = 0;
float airhumidity = 0;
unsigned long metric_Publish_Rate = 30000;
unsigned long metricNextPublishTime = 35000;
unsigned long tempNextSampleTime = 30000;
unsigned long temp_Sample_Rate = 10000;
unsigned long dsNextSampleTime = 20000;
unsigned long ds_Sample_Rate = 2000;
unsigned long syncDHTNextSampleTime = 10000;
unsigned long syncDHT_Sample_Rate = 10000;
unsigned long wireSearchTime = 20000;
unsigned long wire_Search_Rate = 10000;
uint8_t dsAttempts = 0;
bool bDHTstarted;
bool online;
bool radioON;
int failedAttempts;
int successAttempts;

void getairtemp();
void getwatertemp();
void publishData();
void radioCallbackHandler(bool radio_active);
/* executes once at startup */
void setup() {
  Serial1.begin(38400);
  Serial1.println("Hello World");
  pinMode(D3, INPUT);
  pinMode(D2, INPUT);
  BLE.registerNotifications(radioCallbackHandler);
}

void dht_wrapper() {
    DHT.isrCallback();
    }

/* executes continuously after setup() runs */
void loop() {
  // if (millis() > tempNextSampleTime && !radioON){
  //   Serial1.println("Air Sample Time!");
  //   getairtemp();
  // }

  if (millis() > dsNextSampleTime && !radioON){
    getwatertemp();
    }

  // if (millis() > metricNextPublishTime && Particle.connected()){
  //   Serial1.println("Publish Time!");
  //   publishData();
  //   }

  if (Particle.connected()){
      System.sleep(SLEEP_MODE_DEEP);
    }
}

void radioCallbackHandler(bool radio_active){
  radioON = radio_active;
}

void publishData(){
//if(!ds18b20.crcCheck()){
  //  return;
  //}
  sprintf(szInfo, "Air Temp is: %2.2f and Humidity is: %2.2f. Water Temp is: %2.2f" , airtemp, airhumidity, fahrenheit);
  Particle.publish("bluzTmp", szInfo, PRIVATE);
  metricNextPublishTime = millis() + metric_Publish_Rate;
  Serial1.print("Next Publish in: ");
  Serial1.println(metricNextPublishTime);
}

void getwatertemp(){
  if(firstPass){
    ds18b20.setResolution(9);
    firstPass = false;
  }
  if(!ds18b20.search()){
    ds18b20.resetsearch();
    Serial1.print("Power Supply mode is: ");
    if(ds18b20.readPowerSupply()){
      Serial1.println("Parasite.");
    }
    else {Serial1.println("NOT Parasite.");}
    celsius = ds18b20.getTemperature();
    Serial1.print("Temp returned is: ");
    Serial1.println(celsius);
    if (!ds18b20.crcCheck()){
      Serial1.print("Device asserted: ");
      Serial1.println(ds18b20.dsreset());
      failedAttempts++;
      Serial1.print("Failed Attempts: ");
      Serial1.println(failedAttempts);
    }
    else{
    successAttempts++;
    Serial1.print("Successful Attempts: ");
    Serial1.println(successAttempts);
    }
    fahrenheit = ds18b20.convertToFahrenheit(celsius);
    dsNextSampleTime = millis() + ds_Sample_Rate;
    Serial1.print("Fahrenheit returned is: ");
    Serial1.println(fahrenheit);
    int totalAttempts = successAttempts + failedAttempts;
    Serial1.print("Successful Temp Reading at: ");
    int percentage = ((float)successAttempts /  (float)totalAttempts) * 100.0;
    Serial1.println(percentage);
  }

}

void getairtemp(){
  tempNextSampleTime = millis() + temp_Sample_Rate;  // set the time for next sample
  if (!bDHTstarted) {		// start the sample
      Serial1.println("Acquiring from DHT.");
      DHT.acquire();
      bDHTstarted = true;
      Serial1.print("bDHTstarted is: ");
      Serial1.println(bDHTstarted);
      } //End of DHTstarted IF
      Serial1.println("Checking for Sample Completion.");
      if (!DHT.acquiring())
     {		// has sample completed?
      Serial1.println("Not acquiring.");
      Serial1.println("Getting Status");
    	int result = DHT.getStatus();
      switch (result) {
    case DHTLIB_OK:
        Serial1.println("OK");
        break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial1.println("Error\n\r\tChecksum error");
        //status = "Checksum error.";
        break;
    case DHTLIB_ERROR_ISR_TIMEOUT:
      Serial1.println("Error\n\r\tISR time out error");
        //status = "ISR Timeout.";
        break;
    case DHTLIB_ERROR_RESPONSE_TIMEOUT:
      Serial1.println("Error\n\r\tResponse time out error");
        //status = "Response Timeout.";
        break;
    case DHTLIB_ERROR_DATA_TIMEOUT:
       Serial1.println("Error\n\r\tData time out error");
        //status = "Data Timeout error.";
        break;
    case DHTLIB_ERROR_ACQUIRING:
        Serial1.println("Error\n\r\tAcquiring");
        //status = "Error Acquiring";
        break;
    case DHTLIB_ERROR_DELTA:
       Serial1.println("Error\n\r\tDelta time to small");
      //status = "Delta time too small.";
        break;
    case DHTLIB_ERROR_NOTSTARTED:
        Serial1.println("Error\n\r\tNot started");
      //status = "Error Not Started.";
        break;
    default:
        Serial1.println("Unknown error");
    //  status = "Unknown Error.";
        break;
        }
      }
      Serial1.println("Getting Fahrenheit.");
      airtemp = (DHT.getFahrenheit());
      Serial1.println(airtemp);
      Serial1.println("Getting Humidity.");
      airhumidity = (DHT.getHumidity());
      Serial1.println(airhumidity);
      //n++;  // increment DHT Sample counter
      bDHTstarted = false;  // reset the sample flag so we can take another
      //tempNextSampleTime = millis() + temp_Sample_Rate;  // set the time for next sample
      Serial1.print("Next Temp Sample in: ");
      Serial1.println(tempNextSampleTime);

}
