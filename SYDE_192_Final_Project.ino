enum possibleStates {OFF, ARMED, TRIPPED};
enum buttons {BLUE, GREEN, WHITE, BLACK, RED, YELLOW, LASER};

volatile int state = OFF; // state which determines which mode the device operates in 
volatile bool buttonPressed = false; // state that shows if a button is pressed 
int passwordState = 0; // state for the password FSM

// pins
int input = 10;
int inputPin = 2;
int laserRead = 3;
int laserPin= 6;
int ledPin = 5;
int speakerPin = 4;

volatile int analogReading = 0; // value read from analog pin 5 (used to determine pushed button)

//timer variables 
volatile unsigned long timerOverflowes = 0;
volatile unsigned long previousOverflowes = 0;
volatile unsigned long previousButtonPress = 0;

// Passcode is green green white white blue black blue black red yellow

void setup() {
     pinMode(inputPin, INPUT);
     pinMode(laserRead, INPUT);
     pinMode(ledPin, OUTPUT);
     pinMode(laserPin, OUTPUT);
     pinMode(speakerPin, OUTPUT);
     pinMode(A5, INPUT);
     attachInterrupt(digitalPinToInterrupt(inputPin),buttonChange,FALLING); 
     attachInterrupt(digitalPinToInterrupt(laserRead),laserTrip,FALLING);
     TIMSK1 |= 1; // turn on the overflow interupt for timer one 
     Serial.begin(9600);
}

void loop() {

  // will time out if there have been more than 5000 overflows from the last pressed button 
  unsigned long difference = timerOverflowes - previousButtonPress;
  if(difference > 5000 && passwordState != 0)
  {
    wrongButton();
  }

  //only entered if a button has been pressed
  if(buttonPressed)
  {
    
    previousButtonPress = timerOverflowes;
    input = determineButton();
    
    switch (input)
    {
      case BLUE:
        if(state == OFF)
        {
          state = ARMED;
          passwordState = 0;
          digitalWrite(ledPin, HIGH);
          digitalWrite(laserPin, HIGH);
        }
        else if(state == ARMED || state == TRIPPED)
        {
          if(passwordState == 4 || passwordState == 6)
          {
             passwordState++;
             Serial.println(passwordState);
          }
          else
          {
             wrongButton(); 
          }
        }
        break;
  
       case GREEN:
        if(state == ARMED || state == TRIPPED)
        {
          if(passwordState == 0 || passwordState == 1)
          {
             passwordState++;
             Serial.println(passwordState);
          }
          else
          {
             wrongButton();  
          }
        }
        break;
  
       case WHITE:
        if(state == ARMED || state == TRIPPED)
        {
          if(passwordState == 2 || passwordState == 3)
          {
             passwordState++;
             Serial.println(passwordState);
          }
          else
          {
            wrongButton(); 
          }
        }
        break;
             
       case BLACK:
        if(state == ARMED || state == TRIPPED)
        {
          if(passwordState == 5 || passwordState == 7)
          {
             passwordState++;
             Serial.println(passwordState);
          }
          else
          {
            wrongButton();
          }
        }
        break;
  
       case RED:
        if(state == ARMED || state == TRIPPED)
        {
          if(passwordState == 8)
          {
             passwordState++;
             Serial.println(passwordState);
          }
          else
          {
            wrongButton();
          }
        }
        break;
  
      case YELLOW:
        if(state == ARMED || state == TRIPPED)
        {
          if(passwordState == 9)
          {
            state = OFF;
            Serial.println(passwordState);
            passwordState = 0;
            digitalWrite(ledPin, LOW);
            digitalWrite(laserPin, LOW);
          }
          else
          {
            wrongButton();
          }
        }
        break;
       default:
        break;
    }
    buttonPressed = false;
  }

  //flashes LED and plays a tone when in the tripped state
  if (state == TRIPPED)
  {
    digitalWrite(laserPin, LOW);
    digitalWrite(ledPin, HIGH);
    tone(speakerPin,440); // output pin, frequency
    delay(200);
    digitalWrite(ledPin, LOW);
    noTone(speakerPin);
    delay(200);
  }

}

// determines the button pressed depending on what is read from the analog pin 
int determineButton()
{
  if (analogReading < 10)
    {
      return BLUE;
    }
    else if (analogReading < 100)
    {
      return GREEN;
    }
    else if (analogReading < 200)
    {
      return WHITE;
    }
    else if (analogReading < 250)
    {
      return BLACK;
    }
     else if (analogReading < 300)
    {
      return RED;
    }
     else if (analogReading < 400)
    {
      return YELLOW;
    }
    else
    {
      return 10; // return a value outside the range of the buttons enum to signify an error
    }
}

//ressets password and plays an error tone
void wrongButton()
{
  passwordState = 0;
  //outputs a wave with frequency 46 Hz to the speaker pin
  tone(speakerPin, 46); 
  delay(100); // necessary for the tone to be played while in the tripped state
  noTone(speakerPin); // stops outputing a wave to the speaker pin
}

//increrement a varable when timer one overflows
ISR (TIMER1_OVF_vect)
{
  timerOverflowes++;
}

//laser interrupt neesacary for it to be on its own
//as otherwise the ambient light dissrupts the values read for the buttons
void laserTrip()
{
  if(state == ARMED)
  {
    state = TRIPPED;
  }  
}

//interupt for reading what button was pressed
void buttonChange()
{

  // will only modify the analogRead value if it has been a sufficent amount of time from the last interrupt
  // this prevents buncing values from analogRead()
  unsigned long difference = timerOverflowes - previousOverflowes;
  previousOverflowes = timerOverflowes;

  if (difference < 100)
  {
    return;
  }
  else
  {
    analogReading = analogRead(A5); // This yields a resolution between readings of: 5 volts / 1024 units or, .0049 volts (4.9 mV) per unit
    buttonPressed = true; 
  }
}
