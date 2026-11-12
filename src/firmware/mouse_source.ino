#include <Mouse.h>
#include <Adafruit_MPU6050.h>
#define outputA 4
#define outputB 5 // encoder channels
#define X_PIN A3
#define Y_PIN A2 // joystick analog outputs
#define SWITCH_PIN 15 // joystick button pin
#define MotorPin 9 // "in" vibration motor pin
#define pressed false 
#define g 9.5 // mpu6050 reading when measuring gravitational acceleration
#define ENCODER_SWITCH 7 // encoder "switch" pin
#define Button1Pin 8 // left button pin
#define Button2Pin 10 //right button pin
#define X_POS_MAX 505
#define X_NEG_MAX -518
#define Y_POS_MAX 519
#define Y_NEG_MAX -504 // joystick readings across all axes and directions
#define MOUSE_MAX 6 // bounds of cursor movement at once (joystick)
#define ACTUAL_SWITCH_PIN 14 // slide switch middle pin
Adafruit_MPU6050 mpu;
bool button=true,prev_button=true;
bool button1=true;
bool button2=true; // variables to save all the buttons' states
uint8_t move_cursor_max=11;
const uint16_t x_offset=518;
const uint16_t y_offset=519; // joystick resting position analog readings
const float acc_x_offset=0.65;
const float acc_y_offset=0.3;
uint8_t count=0; // counter variable
int32_t sumx_joy=0,sumy_joy=0; // sums of consecutive joystick readings
int16_t avgx_joy=0,avgy_joy=0; // averages of consecutive joystick readings
float sumx = 0,avgx=0; 
float sumy = 0,avgy=0; // same for accelerometer readings
int16_t aState;
int16_t aLastState; // current and previous states of encoder outputA
int8_t increment=0; // scroll value
sensors_event_t accel, gyro, temp; // accelerometer, gyro and temp sensor structures 
void move_motor(uint16_t period, float duty_cycle, uint16_t number_of_cycles) // a function that moves the motor 
{
  int i;
  uint16_t high_time =(uint16_t)(duty_cycle * (float)period);
  for (i = 0; i < number_of_cycles; i++)
  {
    digitalWrite(MotorPin, HIGH);
    delay(high_time);
    digitalWrite(MotorPin, LOW);
    delay(period - high_time);
  }
}
inline void Init_pins (void) // function that initializes the pins
{
  pinMode(X_PIN,INPUT);
  pinMode(Y_PIN,INPUT);
  pinMode(SWITCH_PIN,INPUT_PULLUP);
  pinMode (outputA,INPUT);
  pinMode (outputB,INPUT);
  pinMode(MotorPin, OUTPUT);
  pinMode(Button1Pin, INPUT_PULLUP);
  pinMode(Button2Pin, INPUT_PULLUP);
  pinMode(ACTUAL_SWITCH_PIN,INPUT_PULLUP);
  pinMode(ENCODER_SWITCH,INPUT_PULLUP);
}
void setup() {
  // put your setup code here, to run once:
  Init_pins(); // initialization of all the pins in use, except for I2C and power pins
  Serial.begin(9600);
  mpu.begin(); // initiates the MPU6050 module
  Mouse.begin(); // initiates the mouse module
  aLastState = digitalRead(outputA); // reads the initial state of the outputA
}
void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()>0) // if data is sent over serial communication
    move_cursor_max=Serial.read(); // // receive data and use it to adjust sensitivity
  button1 = digitalRead(Button1Pin);
  button2 = digitalRead(Button2Pin); // read states of both buttons
  if (button1 == pressed)
  {
    move_motor(140,0.8,2);
    Mouse.click(); 
  } // generate vibrations and activate left click if left button is pressed
  if (button2 == pressed)
  {
    move_motor(10,0.8,28);
    Mouse.click(MOUSE_RIGHT); 
  } // generate vibrations and activate right click if right button is pressed
  if(digitalRead(ENCODER_SWITCH)==pressed)
  {
    move_motor(70,0.9,4);
    Mouse.click(MOUSE_MIDDLE);
  } // generate vibrations and activate middle click if encoder button is pressed
  button=digitalRead(SWITCH_PIN); // read joystick button state
  if(button!=prev_button) 
  {
    if(button==pressed && Mouse.isPressed()==false)
    {
      move_motor(140,0.8,2);
      Mouse.press();
    } // generate vibrations and hold left click if joystick button is pressed
    else if(button==pressed && Mouse.isPressed()==true)
    {
      move_motor(10,0.8,28);
      Mouse.release();
    } // generate vibrations and release left click if joystick button is pressed and left click is held 
  }
  mpu.getEvent(&accel, &gyro, &temp); // read values from mpu: acceleration, angular velocity and temperature, respectively
  float gx = (float)accel.acceleration.x-acc_x_offset; // store acceleration value across x-axis inside a variable and compensate its offset
  float gy = (float)accel.acceleration.y-acc_y_offset; // store acceleration value across y-axis inside a variable and compensate its offset
  int16_t x_value=analogRead(X_PIN)-x_offset;
  int16_t y_value=analogRead(Y_PIN)-y_offset; // read joystick values and compensate their offset
  count++; // increment the counter variable
  sumx = sumx + gx;
  sumy = sumy + gy;
  sumx_joy=sumx_joy+x_value;
  sumy_joy=sumy_joy+y_value; // sum the accelerometer and joystick readings with their previous readings
  if(count==5)
  {
    if(digitalRead(ACTUAL_SWITCH_PIN)==false) // if the switch is in position 1, use the accelerometer readings as input
    {
      avgx = sumx / 5.0; 
      avgy = sumy / 5.0; // calculate the average of 5 consecutive sensor readings to get a better idea of object's actual movement
      if(abs(avgx)<2)
        avgx=0;
      if(abs(avgy)<2)
        avgy=0;
      if(avgx>g)
        avgx=g; 
      if(avgx<-g)
        avgx=-g; // bound accelerometer readings across x-axis to [-g,g] 
      if(avgy>g)
        avgy=g;
      if(avgy<-g)
        avgy=-g; // bound accelerometer readings across y-axis to [-g,g] 
      avgx=map(avgx,-g,g,move_cursor_max,-move_cursor_max-1); // configure mouse sensitivity across x-axis
      avgy=map(avgy,-g,g,move_cursor_max,-move_cursor_max); // configure mouse sensitivity across y-axis
      Mouse.move((int8_t)avgx,(int8_t)avgy,0); // function that does cursor movement
    }
    else
    { // if switch is in position 2, use joystick readings as input
      avgx_joy=sumx_joy/5;
      avgy_joy=sumy_joy/5; // calculate the average of 5 consecutive readings
      avgx_joy=map(avgx_joy,X_NEG_MAX,X_POS_MAX,-move_cursor_max,move_cursor_max);
      avgy_joy=map(avgy_joy,Y_NEG_MAX,Y_POS_MAX,-move_cursor_max,move_cursor_max); // map the values to a certain range to configure sensitivity
      if(avgy_joy>0)
        avgy_joy++; // increment joystick readings when handle is upwards to make it symmetrical with downwards position
      if(abs(avgx_joy)<2)
        avgx_joy=0; 
      if(abs(avgy_joy)<2)
        avgy_joy=0; // filter out very small readings across both axes
      Mouse.move((int8_t)avgx_joy,(int8_t)avgy_joy,0); // function that does cursor movement
    }
    sumx = 0; 
    sumy = 0;
    sumx_joy=0;
    sumy_joy=0;
    count=0; // reset all the relevant values so the average of the next 5 readings can be calculated properly
  }
  aState = digitalRead(outputA); // reads the "current" state of the outputA
  if (aState != aLastState){  // if the outputA state is different to its previous state, the encoder is rotating     
    if (digitalRead(outputB) != aState) // if the outputB state is different to the outputA state, the encoder is rotating CW, otherwise CCW
      increment=-1; 
    else 
      increment=1;
  }
  else 
    increment=0; // set the increment value according to encoder rotation 
  Mouse.move(0,0,increment); // scroll downwards if encoder is  rotating CW, scroll upwards if it is rotating CCW
  prev_button=button; // updates the previous state of the button with the current one, used for software debouncing
  aLastState = aState; // updates the previous state of the outputA with the current state
}
