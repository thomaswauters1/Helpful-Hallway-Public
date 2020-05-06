/***************************************************************************
  This is a library for the AMG88xx GridEYE 8x8 IR camera
  This sketch tries to read the pixels from the sensor
  Designed specifically to work with the Adafruit AMG88 breakout
  ----> http://www.adafruit.com/products/3538
  These sensors use I2C to communicate. The device's I2C address is 0x69
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
//================================================
// Library
//================================================
// Adruino paltform
#include <Arduino.h>
// I2C
#include <Wire.h>
// IR sensor
#include <C:\Users\Twinky\Desktop\Helpful-Hallway\Code\IR_sensor\Test_IR_pixels\lib\Adafruit_AMG88xx\Adafruit_AMG88xx.h>
// Watchdog
#include <esp_system.h>
#include <esp_event.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

//================================================
// Global variables
//================================================
//=============================
// IR sensor
//=============================
// Adafruit object
Adafruit_AMG88xx amg;

// Address of the IR sensor
uint8_t IR_address = 0;

// Amount of pixels
uint8_t pixel_amount = 64;
// Raw values of the pixels of the IR sensor
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
// Average value of all the pixels together
uint16_t average = 0;
// // Enum of population levels
// enum level
// {   
//     very_high = 1,
//     high      = 2, 
//     medium    = 3, 
//     low       = 4,
//     very_low  = 5
// };
// // Population
// level population;
// Population
uint8_t population = 0;

// Packet size
// uint8_t packet_size = AMG88xx_PIXEL_ARRAY_SIZE + 1;
// const uint8_t packet_size = 2;
const uint8_t packet_size = 65;
// Packet for the wireless communication part
// uint8_t packet[packet_size];
uint16_t packet[packet_size];

//=============================
// Watchdog
//=============================
// Timer
const int wdtTimeout = 2;


//=============================
// Test
//=============================
// Tester
bool start = true;
bool tester = true;
bool no_write8 = false;

//================================================
// Methodes
//================================================
//=============================
// IR sensor
//=============================
// https://www.robot-electronics.co.uk/files/grideyeappnote.pdf
//===============
// Sleep mode
//===============
void sleep_mode()
{
  if (no_write8 == true){
    uint8_t rBuffer[1];
    uint8_t wBuffer[2];
    wBuffer[0] = AMG88xx_PCTL;
    wBuffer[1] = AMG88xx_SLEEP_MODE;

    amg.write(AMG88xx_PCTL,wBuffer,2);
  }
  else{
    amg.write8(AMG88xx_PCTL,AMG88xx_SLEEP_MODE);
  }
}
//===============
// Standby mode
//===============
void standby_mode(uint8_t time)
{
  if (no_write8 == true){
    uint8_t wBuffer[2];
    wBuffer[0] = AMG88xx_PCTL;

    if (time == 10)
    {
      wBuffer[1] = AMG88xx_STAND_BY_10;
      amg.write(AMG88xx_PCTL,wBuffer,2);
    }
    else if (time == 60)
    {
      wBuffer[1] = AMG88xx_STAND_BY_60;
      amg.write(AMG88xx_PCTL,wBuffer,2);
    }
  }
  else{
    if (time == 10)
    {
      amg.write8(AMG88xx_PCTL,AMG88xx_STAND_BY_10);
    }
    else if (time == 60)
    {
      amg.write8(AMG88xx_PCTL,AMG88xx_STAND_BY_60);
    }
  }
}
//===============
// Normal mode
//===============
void normal_mode()
{
  if (no_write8 == true){
    uint8_t rBuffer[1];
    uint8_t wBuffer[2];
    wBuffer[0] = AMG88xx_PCTL;
    wBuffer[1] = AMG88xx_NORMAL_MODE;

    amg.write(AMG88xx_PCTL,wBuffer,2);
  }
  else{
    amg.write8(AMG88xx_PCTL,AMG88xx_NORMAL_MODE);
  }
}
//===============
// IR address
//===============
// Scanning the IR sensor address and print the address
// then save the address for the packet
void if_address() {
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  IR_address = (uint8_t)address-58;
}

//===============
// Data pixels
//===============
//===============
// print Data pixels
//===============
// Printing all the pixel values to test pixel reading works
void data_pixels(){
  amg.readPixels(pixels);

  for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
    Serial.print(pixels[i-1]);
    Serial.print(", ");
    if(i%8 == 0) Serial.println();
  }
}

//===============
// option 1
//===============
void all_pixel_values(){
  amg.readPixels(pixels);

  for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
    packet[i] = (uint16_t)pixels[i-1];
  }

  esp_task_wdt_reset();
}

//===============
// option2
//===============

// Calculate the average value of all the pixels
void average_pixels(){
  amg.readPixels(pixels);

  average = 0;

  for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
    average += (uint16_t)pixels[i-1];
  }

  average = average / pixel_amount;

  // Serial.println("!!!!!!!!!!!!");
  // Serial.print("Average value: " + average);
  // Serial.println("!!!!!!!!!!!!");

  esp_task_wdt_reset();
}

// Determine the population from the calculated average value of all the pixels
void population_level(){
  population = 0;

  if ((50 <= average) && (average <= 59))
  {
    population = 1;
  }
  else if  ((40 <= average) && (average <= 49))
  {
    population = 2;
  }
  else if ((30 <= average) && (average <= 39))
  {
    population = 3;
  }
  else if ((20 <= average) && (average <= 29))
  {
    population = 4;
  }
  else if ((10 <= average) && (average <= 19))
  {
    population = 5;
  }
  else
  {
    population = 0;
  }
  
  // Serial.println("!!!!!!!!!!!!");
  // Serial.println("Population level: " + population);
  // Serial.println("!!!!!!!!!!!!");
  packet[1] = population;

  esp_task_wdt_reset();
}

//===============
// IR setup
//===============
// Setting up the IR sensor
void init_ir_program() {
  Serial.println();
  Serial.begin(9600);
  Serial.println(F("IR sensor: AMG88xx pixels"));

  bool status;
  
  // default settings
  status = amg.begin();
  if (!status) {
      Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }
  
  Serial.println();
  if_address();
  packet[0] = IR_address;

  Serial.println();

  Serial.println("(test) Get IR pixels:");
  data_pixels();
  Serial.println("done\n");

  delay(100); // let sensor boot up
}


//===============
// Packet
//===============
// Print the data that the packet contain
void print_packet_data(){
  amg.readPixels(pixels);

  for(int i=1; i<=packet_size; i++){
    if (i<=packet_size-1) {
      Serial.print(packet[i-1]);
      Serial.print(", ");
      // if(i == 10 || i == 20 || i == 30 || i == 40 || i == 50 || i == 60){
      //   Serial.println();
      // }
    }
    else{
      Serial.print(packet[i-1]);
    }
  }
  Serial.println();
  esp_task_wdt_reset();
}

// Reset packet to zeros
void reset_packet_to_zeros(){
  for(int i=1; i<=packet_size; i++){
    packet[i-1] = 0;
  }
}

// Make a packet with first the address and sencond the population
void ir_packet() {
  Serial.println("=====================================");
  Serial.println("Packet");
  Serial.println("=====================================");
  Serial.println("Refresh IR pixel array:");
  all_pixel_values();
  // average_pixels();
  // population_level();
  Serial.println("done\n");
  Serial.println("Show IR packet data:");
  Serial.print("Packet data: ");
  print_packet_data();
  Serial.print("Packet size: ");
  Serial.println(sizeof(packet), DEC);
  Serial.println("done");
  Serial.println("=====================================");
}

//=======================================
// Watchdog
//=======================================
/*
* esp32 watchdog:
* https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/wdts.html#interrupt-watchdog-api-reference
* https://github.com/nkolban/esp32-snippets/blob/master/tasks/watchdogs/main.cpp
*/
//=============================
// Watchdog test
//=============================
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void highPriorityTask(void *myData)
{
    printf("High priority task started and now looping for 10 seconds.  Our priority is %d.\n", uxTaskPriorityGet(nullptr));
    TickType_t startTicks = xTaskGetTickCount();
    while (xTaskGetTickCount() - startTicks < (10 * 1000 / portTICK_PERIOD_MS))
    {
        // Do nothing but loop
    }
    printf("High priority task ended\n");
    vTaskDelete(nullptr);
}

void hardLoopTask(void *myData)
{
    printf("Hard loop task started ...\n");
    while (1)
    {
        // do nothing but burn CPU
    }
}

void hardLoopTaskNoInterrupts(void *myData)
{
    printf("Hard loop task disabling interrupts started ...\n");
    taskDISABLE_INTERRUPTS();
    while (1)
    {
        // do nothing but burn CPU
    }
}

void test_myTask(void *myData)
{
    printf("# Running in myTask\n");
    printf("# Registering our new task with the task watchdog.\n");
    esp_task_wdt_add(nullptr);

    printf("# Looping 5 times with a delay of 1 second and not feeding the watchdog.\n");
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
    }

    printf("# Looping 5 times with a delay of 1 second and positively feeding the watchdog.\n");
    esp_task_wdt_reset();
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
        esp_task_wdt_reset();
    }

    printf("# Removing our watchdog registration so we can do something expensive.\n");
    esp_task_wdt_delete(nullptr);

    printf("# Looping 5 times with a delay of 1 second and not feeding the watchdog.\n");
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
    }

    printf("# Re-registering our task with the task watchdog.\n");
    esp_task_wdt_add(nullptr);
    printf("# Looping 5 times with a delay of 1 second and not feeding the watchdog.\n");
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
    }

    printf("# Our current task priority is %d.\n", uxTaskPriorityGet(nullptr));
    printf("# Spwaning a higher priority task\n");
    xTaskCreate(highPriorityTask, // Task code
                "Priority task",  // Name of task
                16 * 1024,        // Stack size
                nullptr,          // Task data
                5,                // Priority
                nullptr           // task handle
    );

    printf("# Looping 5 times with a delay of 1 second and positively feeding the watchdog.\n");
    esp_task_wdt_reset();
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
        esp_task_wdt_reset();
    }

    printf("Spawning a hard-loop function!\n");
    xTaskCreate(hardLoopTaskNoInterrupts, // Task code
                "Hard Loop",  // Name of task
                16 * 1024,    // Stack size
                nullptr,      // Task data
                5,            // Priority
                nullptr       // task handle
    );

    printf("# Looping 5 times with a delay of 1 second and positively feeding the watchdog.\n");
    esp_task_wdt_reset();
    for (int i = 0; i < 5; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Tick\n");
        esp_task_wdt_reset();
    }


    printf("# Removing our watchdog registration before we end the task.\n");
    esp_task_wdt_delete(nullptr);

    printf("# Ending myTask\n");
    vTaskDelete(nullptr);
} // myTask

void test_init_watchdog_program(){
  // Watchdog
  xTaskHandle handle;
  printf("App starting\n");
  printf("Initializing the task watchdog subsystem with an interval of 2 seconds.\n");
  esp_task_wdt_init(wdtTimeout, false);

  printf("Creatign a new task.\n");
  // Now let us create a new task.
  xTaskCreate(test_myTask,    // Task code
              "My Task", // Name of task
              16 * 1024, // Stack size
              nullptr,   // Task data
              0,         // Priority
              &handle    // task handle
  );
}

//===============
// System reset
//===============
void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

//=============================
// Watchdog programm
//=============================
void watchdog_Task(void *myData)
{
  printf("######################################################################\n");
  printf("Watchdog\n");
  printf("######################################################################\n");
  printf("# Running in myTask\n");
  printf("# Registering our new task with the task watchdog.\n");
  esp_task_wdt_add(nullptr);

  printf("# Resetting watchdog timer\n");
  esp_task_wdt_reset();
  printf("# Packet\n");
  ir_packet();
  printf("# For loop\n");
  for (int i = 0; i < 10; i++)
  {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      printf("Tick\n");
      printf("Resetting watchdog timer\n");
      esp_task_wdt_reset();
      if (i == 4)
      {
        printf("Test watchdog firing\n");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }      
  }

  // printf("# Test reset system\n");
  // resetModule();

  printf("# Removing our watchdog registration\n");
  esp_task_wdt_delete(nullptr);
  printf("# Ending Watchdog_Task\n");
  printf("######################################################################\n");
  vTaskDelete(nullptr);
}

void init_watchdog_program(){
  // Watchdog
  xTaskHandle handle;
  printf("App starting\n");
  printf("Initializing the task watchdog subsystem with an interval of 2 seconds.\n");
  esp_task_wdt_init(wdtTimeout, false);

  printf("Creatign a new task.\n");
  // Now let us create a new task.
  xTaskCreate(watchdog_Task,    // Task code
              "Watchdog_Task", // Name of task
              16 * 1024, // Stack size
              nullptr,   // Task data
              0,         // Priority
              &handle    // task handle
  );
}

//================================================
// Main
//================================================
// void setup() {
//   esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

//   if (cause != ESP_SLEEP_WAKEUP_ULP) {
//     printf("Not ULP wakeup\n");
//     // IR init
//     init_ir_program();
//     // Watchdog init
//     // init_watchdog_program();

//   } else {
//     // IR packet creater
//     // ir_packet();
//     // Watchdog init
//     init_watchdog_program();
//   }
//   printf("Entering deep sleep\n\n");
//   delay(100);

//   esp_deep_sleep_start();
// }

// void setup(){
//   init_ir_program();
//   // init_watchdog_program();
//   Serial.println("Try making pakket");
//   ir_packet();
//   print_packet_data();
//   reset_packet_to_zeros();
//   Serial.println("Go to sleep");
//   sleep_mode();
//   Serial.println("Try making pakket");
//   ir_packet();
//   print_packet_data();
//   Serial.println("Wake up");
//   normal_mode();
//   Serial.println("Try making pakket");
//   ir_packet();
//   print_packet_data();
// }

void setup(){
  init_ir_program();
  init_watchdog_program();
}

void loop() { 
}
