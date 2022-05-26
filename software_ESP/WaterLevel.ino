// 6 dec 2020, basic open loop duty ratio
//              structured using very basic task scheduling: https://github.com/sskata/Arduino-Task-Scheduling-Template/blob/master/TaskSchedulingTemplate_v1.0.ino
// 2-3 may 2021: added functions for PI controller, OTA, mqtt, deepsleep
// 26 may 2022: added BMP280 pressure sensor over i2c

#include <Wire.h>
#include <INA219_WE.h> //https://github.com/wollewald/INA219_WE
#include <ESP8266WiFi.h>  //For ESP8266
#include <PubSubClient.h> //For MQTT
#include <ESP8266mDNS.h>  //For OTA
#include <WiFiUdp.h>      //For OTA
#include <ArduinoOTA.h>   //For OTA
#include "configuration.h" // to store passwords of mqtt and wifi
#include <BMx280I2C.h>

//Set to 1 if tasks with respective period is used, otherwise set to 0 to save memory and CPU
#define TASK_2MS 1
#define TASK_200MS 1
#define TASK_10000MS 1

#define Deepsleep 1
const int sleepSeconds = 200;

float voltage_setp_dcdc=20; //setpoint for voltage over output terminals

//mqtt
String mqtt_client_id = "WEMOSD1-"; //This text is concatenated with ChipId to get unique client_id
String mqtt_base_topic = "/sensor/" + mqtt_client_id + "/data";
#define shunt_topic "/shuntvolt"
#define voltage_topic "/voltage"
#define dutyratio_topic "/dutr"
#define airpressure_topic "/airpress"

#define I2C_ADDRESS 0x40
INA219_WE ina219(I2C_ADDRESS);

const byte D_DCDC = 15; //pin D8 on wemos D1 generates PWM for dc-converter, pin8 makes it feasible to send out zero when going in deep sleep

#define I2C_ADDRESS_BMP 0x76
//create a BMx280I2C object using the I2C interface with I2C Address 0x76
BMx280I2C bmx280(I2C_ADDRESS_BMP);

//structure for PI controller
struct PI_ctrl {
  float Kp;
  float Ti;
  float mem1;
  float mem2;
  float output;
  float samplingtime;
};
struct PI_ctrl PI_DC;


//MQTT client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

//Necesary to make Arduino Software autodetect OTA device
WiFiServer TelnetServer(8266);

void setup()
{

  Serial.begin(9600);

  setup_wifi();

  //OTA things
  Serial.print("Configuring OTA device...");
  TelnetServer.begin();   //Necesary to make Arduino Software autodetect OTA device
  ArduinoOTA.onStart([]() {
    Serial.println("OTA starting...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update finished!");
    Serial.println("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA in progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA OK");

  Serial.println("Configuring MQTT server...");
  mqtt_client_id = mqtt_client_id + ESP.getChipId();
  mqtt_base_topic = "/sensor/" + mqtt_client_id + "/data";
  mqtt_client.setServer(mqtt_server, 1883);
  Serial.printf("   Server IP: %s\r\n", mqtt_server);
  Serial.printf("   Username:  %s\r\n", mqtt_user);
  Serial.println("   Cliend Id: " + mqtt_client_id);
  Serial.println("   MQTT configured!");

  Serial.println("Setup completed! Running app...");

  // PI controller for voltage regulation
  PI_DC.Kp = 0.25;
  PI_DC.Ti = 2;
  PI_DC.mem1 = 0;
  PI_DC.mem2 = 0;
  PI_DC.output = 0;
  PI_DC.samplingtime = 0.2;


  pinMode(LED_BUILTIN, OUTPUT);
  analogWriteFreq(1000); //PWM frequency dc converter

  Wire.begin();
  if (!ina219.init()) {
    Serial.println("INA219 not connected!");
  }
  else {

    ina219.setADCMode(BIT_MODE_12);
    ina219.setMeasureMode(TRIGGERED);

    /* Set PGain
      Gain *  * Shunt Voltage Range *   * Max Current
      PG_40       40 mV                    0,4 A
      PG_80       80 mV                    0,8 A
      PG_160      160 mV                   1,6 A
      PG_320      320 mV                   3,2 A (DEFAULT)
    */
    ina219.setPGain(PG_320);

    /* Set Bus Voltage Range
      BRNG_16   -> 16 V
      BRNG_32   -> 32 V (DEFAULT)
    */
    // ina219.setBusRange(BRNG_32); // choose range and uncomment for change of default

    Serial.println("INA219 Current Sensor Example Sketch - Triggered Mode");
  }

  //begin() checks the Interface, reads the sensor ID (to differentiate between BMP280 and BME280)
  //and reads compensation parameters.
  if (!bmx280.begin())
  {
    Serial.println("begin() failed. check your BMx280 Interface and I2C Address.");
    while (1);
  }

  if (bmx280.isBME280())
    Serial.println("sensor is a BME280");
  else
    Serial.println("sensor is a BMP280");

  //reset sensor to default parameters.
  bmx280.resetToDefaults();

  //by default sensing is disabled and must be enabled by setting a non-zero
  //oversampling setting.
  //set an oversampling setting for pressure and temperature measurements. 
  bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);

  //if sensor is a BME280, set an oversampling setting for humidity measurements.
  if (bmx280.isBME280())
    bmx280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
  
}



struct INAvar {
  float busVoltage_V;
  float current_mA;
  float shuntVoltage_mV;
};
struct INAvar SensorMeas;
int counterloops = 0; //loops of slowest task rate before entering deepsleep
float voltage_zeroDR = 0; // voltage Vout-esp_ground at duty ratio of zero
bool first_run = true;

////////////////////////////////////////////////
// start of main loop
////////////////////////////
void loop()
{ //Task scheduler, schedules and executes tasks
  ArduinoOTA.handle();

#if TASK_2MS
  static long unsigned int a = micros();
  if (micros() >= (a + 2000)) { //2 ms tasks
    a = micros();
    /* Tasklist: 2 ms */

  }
  else if (micros() < a) a = 0;
#endif

#if TASK_200MS
  static long unsigned int f = micros();
  if (micros() >= (f + 200000)) { //200 ms tasks
    f = micros();
    /* Tasklist: 500 ms */
    //    firstTask();

    if (first_run) //get voltage on bus when converter is inactive
    {
      ctrl_DCDC(0);
    }

    SensorMeas = read_INA219();

    if (first_run) //get voltage on bus when converter is inactive
    {
      voltage_zeroDR = SensorMeas.busVoltage_V;
      first_run = false;
    }


//    Serial.print("struct Shunt Voltage [mV]: "); Serial.println(SensorMeas.shuntVoltage_mV);
//    Serial.print("struct bus Voltage [V]: "); Serial.println(SensorMeas.busVoltage_V);
//    Serial.print("struct current_mA: "); Serial.println(SensorMeas.current_mA); //not relevant when replacing resistor

    float setp_volt_out = voltage_setp_dcdc + voltage_zeroDR;
//    Serial.print("setp_volt_out: "); Serial.println(setp_volt_out);

    PI_DC = PI_loop(PI_DC, SensorMeas.busVoltage_V, setp_volt_out);


    //clip maximum duty ratio
    if (PI_DC.output > 30) {
      PI_DC.output = 30;
    }
    if (PI_DC.output < 0) {
      PI_DC.output = 0;
    }

    ctrl_DCDC((int)(PI_DC.output));
    Serial.print("PI_DC output: "); Serial.println(PI_DC.output);

    //    ctrl_DCDC(0);
    blinkLED();



  }
  else if (micros() < f) f = 0;
#endif

#if TASK_10000MS
  static long unsigned int g = micros();
  if (micros() >= (g + 10000000)) { 
    g = micros();
    /* Tasklist: 10000 ms */

      //start a pressure measurement
  if (!bmx280.measure())
  {
    Serial.println("could not start measurement, is a measurement already running?");
    return;
  }

    if (!mqtt_client.connected())
    {
      mqtt_reconnect();
    }
    mqtt_client.loop();

      //wait for the pressure measurement to finish
  do
  {
    delay(100);
  } while (!bmx280.hasValue());

  Serial.print("Pressure: "); Serial.println(bmx280.getPressure());
  Serial.print("Pressure (64 bit): "); Serial.println(bmx280.getPressure64());
  Serial.print("Temperature: "); Serial.println(bmx280.getTemperature());

  //important: measurement data is read from the sensor in function hasValue() only. 
  //make sure to call get*() functions only after hasValue() has returned true. 
  if (bmx280.isBME280())
  {
    Serial.print("Humidity: "); 
    Serial.println(bmx280.getHumidity());
  }

    //    Serial.print("mqtt voltage V: ");
    //    Serial.println(SensorMeas.busVoltage_V);
    //    Serial.print("mqtt current shunt mA: ");
    //    Serial.println(SensorMeas.current_mA);
    //    Serial.println(String(SensorMeas.busVoltage_V,3).c_str());
    mqtt_client.publish((mqtt_base_topic + shunt_topic).c_str(), String(SensorMeas.shuntVoltage_mV, 2).c_str(), true);
    mqtt_client.publish((mqtt_base_topic + voltage_topic).c_str(), String(SensorMeas.busVoltage_V, 2).c_str(), true);
    mqtt_client.publish((mqtt_base_topic + dutyratio_topic).c_str(), String(PI_DC.output, 2).c_str(), true);
mqtt_client.publish((mqtt_base_topic + airpressure_topic).c_str(), String(bmx280.getPressure64(), 2).c_str(), true);


#if Deepsleep
    if (counterloops > 8)
    {
      ESP.deepSleep(sleepSeconds * 1000000);
    }
    counterloops = counterloops + 1;
#endif


  }
  else if (micros() < g) g = 0;
#endif


}

////////////////////////////
// end of main loop
/////////////////////////////////////////////////////////////




bool LedStateRequest = true;
void blinkLED()
{
  digitalWrite(LED_BUILTIN, LedStateRequest);
  LedStateRequest = !LedStateRequest;
}


void ctrl_DCDC(float percentage)
{
  //duty ratio in percentage to PWM signal
  int value = 0.01 * percentage * 1024;
  analogWrite(D_DCDC, value);

}



struct INAvar read_INA219() {
  //read sensor data from INA 219 and report back in structure
  struct INAvar INAnow;

  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0;
  bool ina219_overflow = false;

  ina219.startSingleMeasurement(); // triggers single-shot measurement and waits until completed
  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();
//  current_mA = ina219.getCurrent_mA(); //not relevant when replacing 0.1ohm resistance
  //  power_mW = ina219.getBusPower();
  //  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
  ina219_overflow = ina219.getOverflow();

  //    Serial.print("Shunt Voltage [mV]: "); Serial.println(shuntVoltage_mV);
  //    Serial.print("Bus Voltage [V]: "); Serial.println(busVoltage_V);
  //    Serial.print("Load Voltage [V]: "); Serial.println(loadVoltage_V);
  //    Serial.print("Current[mA]: "); Serial.println(current_mA);
  //  Serial.print("Bus Power [mW]: "); Serial.println(power_mW);

  INAnow.busVoltage_V = busVoltage_V;
  INAnow.current_mA = current_mA;
  INAnow.shuntVoltage_mV = shuntVoltage_mV;
  return INAnow;
}



struct PI_ctrl PI_loop(struct PI_ctrl PI_in, float measurement, float setpoint) {
  struct PI_ctrl PI_out;
  float c0 = 0;
  float c1 = 0;
  float error = 0;
  float Ut1 = 0;
  float Et1 = 0;

  PI_out = PI_in;

  c0 = PI_in.Kp * (1 + PI_in.samplingtime / PI_in.Ti);
  c1 = -PI_in.Kp;

  Ut1 = PI_in.mem1;
  Et1 = PI_in.mem2;

  //anti-reset windup back clamping
  Et1 = Et1 + (PI_in.output - Ut1) / c0;
  Ut1 = PI_in.output;

  error = setpoint - measurement;
  PI_out.output = Ut1 + c0 * error + c1 * Et1;

  PI_out.mem1 = PI_out.output;
  PI_out.mem2 = error;

  return PI_out;

  //  struct PI_ctrl{
  //  float Kp;
  //  float Ti;
  //  float mem1;
  //  float mem2;
  //  float output;
  //  float samplingtime;
  //}
}



void mqtt_reconnect()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (mqtt_client.connect(mqtt_client_id.c_str(), mqtt_user, mqtt_password))
    {
      Serial.println("connected");
    } else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup_wifi()
{
  delay(10);
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("OK");
  Serial.print("   IP address: ");
  Serial.println(WiFi.localIP());
}
