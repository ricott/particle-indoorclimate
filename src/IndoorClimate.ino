#include "SparkFunRHT03/SparkFunRHT03.h"
#include "blynk.h"
//Blynk token kept in file listed in .gitignore
#include "AUTH_TOKEN.h"

const String event_prefix = "house/climate/";
const int RHT03_DATA_PIN = D1;

BlynkTimer timer;
RHT03 rht;

float humidity;
float celsius;
int tempStrike;
int humStrike;
float offset = -0.2;

void setup()
{
  Particle.publish(event_prefix + "setup", "starting", PRIVATE);
  delay(5000); // Allow board to settle
  Time.zone(+1);

  Blynk.begin(BLYNK_AUTH_TOKEN);

  rht.begin(RHT03_DATA_PIN);

  // Setup a function to be called every second
  timer.setInterval(15000L, getClimate);
  timer.setInterval(60000L, publishData);
}

void loop()
{
  Blynk.run();
  timer.run();
}

void getClimate()
{
  float _temp;
  float _hum;
  int i = 0;
  int MAXRETRY = 4;

  int updateRet = 0;
  do
  {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    updateRet = rht.update();
    delay(2000L);
  } while ((updateRet != 1) && MAXRETRY > i++);

  _temp = rht.tempC();
  _hum = rht.humidity();

  if (i < MAXRETRY)
  {
    //If new reading is >10% or <10% vs previous then ignire it
    //At most ignore 5 readings in a row, then set it anyways
    if (celsius == 0 || tempStrike > 5 ||
        (_temp > celsius * 0.9 && _temp < celsius * 1.1))
    {
      celsius = _temp + offset;
      tempStrike = 0;
    }
    else
    {
      tempStrike++;
    }

    if (humidity == 0 || humStrike > 5 ||
        (_hum > humidity * 0.9 && _hum < humidity * 1.1))
    {
      humidity = _hum;
      humStrike = 0;
    }
    else
    {
      humStrike++;
    }
  }
  else
  {
    //celsius = humidity = 0.0;
    Particle.publish(event_prefix + "reading", "failed", PRIVATE);
  }
}

void publishData()
{
  char val[16];

  Blynk.virtualWrite(V106, humidity);
  Blynk.virtualWrite(V107, celsius);

  sprintf(val, "%.2f", celsius);
  Particle.publish(event_prefix + "temperature", val, PRIVATE);
  sprintf(val, "%.2f", humidity);
  Particle.publish(event_prefix + "humidity", val, PRIVATE);
}
