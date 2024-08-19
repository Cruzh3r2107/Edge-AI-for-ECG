#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>
#include "sensor_model.h"

#define PLOTT_DATA 
#define MAX_BUFFER 100
#define NUMBER_OF_INPUTS 17
#define NUMBER_OF_OUTPUTS 1
#define TENSOR_ARENA_SIZE 2 * 1024 

uint32_t prevData[MAX_BUFFER];
uint32_t sumData = 0;
uint32_t maxData = 0;
uint32_t avgData = 0;
uint32_t roundrobin = 0;
uint32_t countData = 0;
uint32_t period = 0;
uint32_t lastperiod = 0;
uint32_t millistimer = millis();
double frequency;
double beatspermin = 0;
uint32_t newData;

Eloquent::TinyML::TensorFlow::TensorFlow<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> tf;

void freqDetec()
{
  if (countData == MAX_BUFFER)
  {
    if (prevData[roundrobin] < avgData * 1.5 && newData >= avgData * 1.5)
    {
      period = millis() - millistimer;
      millistimer = millis();
      maxData = 0;
    }
  }
  roundrobin++;
  if (roundrobin >= MAX_BUFFER)
  {
    roundrobin = 0;
  }
  if (countData < MAX_BUFFER)
  {
    countData++;
    sumData += newData;
  }
  else
  {
    sumData += newData - prevData[roundrobin];
  }
  avgData = sumData / countData;
  if (newData > maxData)
  {
    maxData = newData;
  }

  #ifdef PLOTT_DATA
  Serial.print(newData);
  Serial.print("\t");
  Serial.print(avgData);
  Serial.print("\t");
  Serial.print(avgData * 1.5);
  Serial.print("\t");
  Serial.print(maxData);
  Serial.print("\t");
  Serial.println(beatspermin);
  #endif

  prevData[roundrobin] = newData;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("start");
  Serial.println("Starting the model");
  tf.begin(sensor_model);
  Serial.println("Model started");
}

void loop()
{
  newData = analogRead(34);
  freqDetec();
  if (period != lastperiod)
  {
    frequency = 1000 / (double)period;
    if (frequency * 60 > 20 && frequency * 60 < 200)
    {
      beatspermin = frequency * 60;

      #ifndef PLOTT_DATA
      Serial.print(frequency);
      Serial.print(" hz");
      Serial.print(" ");
      Serial.print(beatspermin);
      Serial.println(" bpm");
      #endif
      
      lastperiod = period;
    }
  }
  
  if (Serial.available() > 0)
  {
    float sensorReadings[NUMBER_OF_INPUTS];
    for (int i = 0; i < NUMBER_OF_INPUTS; i++)
    {
      sensorReadings[i] = Serial.parseFloat();
    }
    
    float predicted = tf.predict(sensorReadings);
    
    Serial.print("Input: ");
    for (int i = 0; i < NUMBER_OF_INPUTS; i++)
    {
      Serial.print(sensorReadings[i]);
      Serial.print("\t");
    }
    Serial.print("\tPredicted: ");
    Serial.println(predicted);
    
    if (predicted > 0.50)
    {
      Serial.print("\tThere is SIGNAL: ");
    }
    else
    {
      Serial.print("\tNO SIGNAL: ");
    }
  }
  
  delay(5);
}
