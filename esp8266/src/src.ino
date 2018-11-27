#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <Scheduler.h>
#include <DHTesp.h>

#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

static String FARMID = "C000000";
static String EQUIPID = "S0000000";

StaticJsonBuffer<300> jsonBuffer;
JsonObject& cmd   = jsonBuffer.createObject();   // Command
JsonObject& state = jsonBuffer.createObject();   // StateInfo

enum errNo {
  PUSH_CMD_ERR    = -1,
  PUSH_STATE_ERR  = -2,
  GET_CMD_ERR     = -3
};

struct command{
  int Gosensing;
  int Stop;
  int Init;
  int Up;
  int Down;
  int In;
  int Out;
  int Setpos0;
};
struct command c;

int dht_pin = D0;
int led[8] = {D1, D2, D3, D4, D5, D6, D7, D8};
DHTesp dht;

void initData() {
  int ret;
  
  // init FB-StateInfo
  state["actionFlag"]      = 0;
  state["inOutFlag"]       = 0;
  state["upDownFlag"]      = 0;
  state["currentDepth"]    = 0;
  state["measureTime"]     = 0;
  state["depth_1"]         = 0;
  state["sensorvalueDO_1"] = 0;

  // push Init Data
  if( (ret = pushData(state)) < 0 ) {
    Serial.println("error "+ret);
  }
}

int pushData(JsonObject& json) {
  String para = "StateInfo/";
  int err = PUSH_STATE_ERR;
  
  if( json == cmd ) {
    para = "Command/";
    err = PUSH_CMD_ERR;
  }

  Firebase.set(para+FARMID+"&"+EQUIPID, json);
  
  if( Firebase.failed() ) {
    Serial.println("setting /"+para+FARMID+"&"+EQUIPID);
    return err;
  }
 
  return 0;
}

class ReadSensorTask : public Task {
  public:
    void setup() {
        dht.setup(dht_pin, DHTesp::DHT11);
    }
    void loop() {
        delay(dht.getMinimumSamplingPeriod());
        float humidity = dht.getHumidity();
        float temperature = dht.getTemperature();

        state["actionFlag"]      = dht.getStatusString();
        state["inOutFlag"]       = humidity;
        state["upDownFlag"]      = temperature;
        state["currentDepth"]    = dht.toFahrenheit(temperature);
        state["measureTime"]     = dht.computeHeatIndex(temperature, humidity, false);
        state["depth_1"]         = dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true);
        state["sensorvalueDO_1"] = 0;

        pushData(state);
    }
} readSensor_task;

class GetDataTask : public Task {
  public:
    void loop() {
        c.Gosensing = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/GOSENSING");
        c.Stop      = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/STOP");
        c.Init      = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/INIT");
        c.Up        = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/UP");
        c.Down      = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/DOWN");
        c.In        = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/IN");
        c.Out       = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/OUT");
        c.Setpos0   = Firebase.getBool("Command/"+FARMID+"&"+EQUIPID+"/SETPOS0");
    }
} getData_task;

class RunCommandTask1 : public Task {
  public:
    void loop() {
        // Command/[FARMID]&[EQUIPID]/GOSENSING 을 수행할 코드 집어넣기
        c.Gosensing ? digitalWrite(led[0], HIGH) : digitalWrite(led[0], LOW);

        // Command/[FARMID]&[EQUIPID]/STOP 을 수행할 코드 집어넣기
        c.Stop ? digitalWrite(led[1], HIGH) : digitalWrite(led[1], LOW);

        // Command/[FARMID]&[EQUIPID]/INIT 을 수행할 코드 집어넣기
        c.Init ? digitalWrite(led[2], HIGH) : digitalWrite(led[2], LOW);

        // Command/[FARMID]&[EQUIPID]/UP 을 수행할 코드 집어넣기
        c.Up ? digitalWrite(led[3], HIGH) : digitalWrite(led[3], LOW);
    }
} runCommand_task1;

class RunCommandTask2 : public Task {
  public:
    void loop() {
        // Command/[FARMID]&[EQUIPID]/DOWN 을 수행할 코드 집어넣기
        c.Down ? digitalWrite(led[4], HIGH) : digitalWrite(led[4], LOW);

        // Command/[FARMID]&[EQUIPID]/IN 을 수행할 코드 집어넣기
        c.In ? digitalWrite(led[5], HIGH) : digitalWrite(led[5], LOW);

        // Command/[FARMID]&[EQUIPID]/OUT 을 수행할 코드 집어넣기
        c.Out ? digitalWrite(led[6], HIGH) : digitalWrite(led[6], LOW);

        // Command/[FARMID]&[EQUIPID]/SETPOS0 을 수행할 코드 집어넣기
        c.Setpos0 ? digitalWrite(led[7], HIGH) : digitalWrite(led[7], LOW);
    }
} runCommand_task2;

void setup() {
  Serial.begin(9600);
  int i;

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // start & connect FB
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  initData();

  for(i = 0; i < 8; i++) {
    pinMode(led[i], OUTPUT);
    digitalWrite(led[i], LOW);
  }
  dht.setup(17, DHTesp::DHT22);

  Scheduler.start(&getData_task);
  Scheduler.start(&runCommand_task1);
  Scheduler.start(&runCommand_task2);
  Scheduler.start(&readSensor_task);
  Scheduler.begin();
}

void loop() {
  //
}
