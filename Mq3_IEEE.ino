#include <EasyBuzzer.h>

// Consts
const int MQ_PIN = A0;
const int RANDOM_SEED_PIN = A5;
const int BUZZER_PIN = 5;
const int RED_PIN_RGBD = 8;
const int GREEN_PIN_RGBD = 9;
const int BLUE_PIN_RGBD = 10;
const int BUTTON_R2_PIN = 2;
const int BUTTON_R3_PIN = 3;
const int CYCLE_DELAY = 300;
const int ACTIVE_TRESHOLD = 5;
const int RECOVERING_COUNT = 5;
const bool DEBUG = true;

int random_range1[] = {0, 10};
int random_range2[] = {10, 30};
int random_range3[] = {30, 80};

// State Machine
enum { StandBy = 0,
       Active = 1, 
       Recovering = 2};

// Random button ranges
enum { Range0 = 0,
       Range1 = 1, 
       Range2 = 2}; 

// Global Vars
int mq_value = 0;
int last_mq_value = -1;
int status = StandBy;
bool active_side = NULL;
int active_count = 0;
bool sound_active = true;
int *selected_range = random_range1;
float alcoholic_val = -1;


// Sound functions
void buzz_callback(){
  Serial.println("Mute");
}

void custom_beep(){
  EasyBuzzer.beep(
    2000,        
    100,           
    100,           
    2,             
    300,          
    1,             
    buzz_callback
  );
}

// Random ranges functions
float random_range(int range[]){
  float randNumber = random(range[0], range[1]);

  return randNumber/100;
}


void R2_IRS()
{
  Serial.println("R2_IRS");
  selected_range = random_range2;
}

void R3_IRS()
{
  Serial.println("R3_IRS");
  selected_range = random_range3;
}


void setup()
{
  Serial.begin(9600);
  pinMode(MQ_PIN,INPUT);
  EasyBuzzer.setPin(BUZZER_PIN);
  randomSeed(analogRead(RANDOM_SEED_PIN));

  attachInterrupt(digitalPinToInterrupt(BUTTON_R2_PIN), R2_IRS, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_R3_PIN), R3_IRS, CHANGE);

  pinMode(RED_PIN_RGBD, OUTPUT);
  pinMode(GREEN_PIN_RGBD, OUTPUT);
  pinMode(BLUE_PIN_RGBD, OUTPUT);
}


void loop()
{
  // New value sensed and change compute
  mq_value = analogRead(MQ_PIN);
  int mq_change = mq_value - last_mq_value;

  // DEBUG
  if(DEBUG){
    Serial.print("Analog Reading= ");
    Serial.println(mq_value);
    Serial.print("Change= ");
    Serial.println(mq_change);
    Serial.print("Status= ");
    Serial.println(status);
    Serial.print("Side= ");
    Serial.println(active_side);
  }


  switch(status){

    case StandBy:

      // LCD show message "READY"

      // Show status
      if(sound_active){EasyBuzzer.stopBeep();}
      analogWrite(RED_PIN_RGBD,   0);
      analogWrite(GREEN_PIN_RGBD, 255);
      analogWrite(BLUE_PIN_RGBD,  0);

      // Status change
      if((abs(mq_change) > ACTIVE_TRESHOLD) && last_mq_value !=-1){
        status = Active;
        active_side = mq_change > 0;
      }
      break;

    case Active:

      // LCD show message "Blow"

      // Show status
      if(sound_active){custom_beep();}
      analogWrite(RED_PIN_RGBD,   255);
      analogWrite(GREEN_PIN_RGBD, 126);
      analogWrite(BLUE_PIN_RGBD,  0);

      // Status change
      if(!(mq_change > 0 && active_side) && abs(mq_change) > 1){
        active_count++;
      }
      if(active_count > RECOVERING_COUNT){
        status = Recovering;
        alcoholic_val = -1;
      }
      break;

    case Recovering:

      // LCD show alcoholic value
      if(alcoholic_val == -1){
        alcoholic_val = random_range(selected_range);
        Serial.print("ALC= ");
        Serial.println(alcoholic_val);
      }
      
      // Show status
      EasyBuzzer.stopBeep();
      analogWrite(RED_PIN_RGBD,   255);
      analogWrite(GREEN_PIN_RGBD, 0);
      analogWrite(BLUE_PIN_RGBD,  0);
      active_count = 0;

      // Status change
      if(mq_change == 0){
        status = StandBy;
        selected_range = random_range1;
      }
      break;

    default: 
      Serial.print("ERROR");
    }

  delay(CYCLE_DELAY);
  last_mq_value = mq_value;
}

