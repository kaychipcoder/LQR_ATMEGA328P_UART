/*

    FILE: LQR_ATMEGA328P_UART.ino
    VERSION: 1.0
    AUTHOR: KAY CHIP
    COPYRIGHT: SPK-TURBO

*/

uint8_t errorID;    // there's error ID which is returned after processing the ADC buffer
uint8_t Mode;   // there's control modes, it can be received by UART or computer communicating
uint8_t sensor[8] = {A0, A1, A2, A3, A4, A5, A6, A7};     // Listing LQR sensors pin
uint8_t led[8] = {2, 3, 4, 5, 6, 7, 8, 9};    // can be change

// using struct for manage arrays which contain analog values
typedef struct ADC_ARR
{
  uint8_t value[8];
  bool flag = 0;
} ADC_ARR;

// initialize some color's value buffer
ADC_ARR lightColor;
ADC_ARR darkColor;
ADC_ARR greyColor;

// initialize ADC's value buffer & Error's value buffer
ADC_ARR ADCVal;
ADC_ARR Error;

// use for learn Light Color and Dark Color
void learnColor(ADC_ARR *arr)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    arr->value[i] = analogRead(sensor[i]);    // assign analog values that read from analog pin (sensor pin list)
  }

  // turn flag as 1 when complete
  arr->flag = 1;
}

// use for calibrating Grey Color (between Light Color and Dark Color)
void calibColor(ADC_ARR *arr)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    arr->value[i] = (lightColor.value[i] + darkColor.value[i]) / 2;   // assign the average values between Light Color and Dark Color
  }
}

// use for read and save analog values as pointer
void READ_ADC(ADC_ARR *arr)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    arr->value[i] = analogRead(sensor[i]);
  }
}

// use for processing ADCVal buffer and greyColor buffer
// after processing return digital buffer (only 1 and 0) and errorID ( send UART )
void TransmitErrorID(ADC_ARR *arr, uint8_t errID)
{
  // reset the errorID before start processing
  errID = 0;
  for(uint8_t i = 0; i < 8; i++)
  {
    // when present analog values is higher than average values
    if(ADCVal.value[i] > greyColor.value[i])
    {
      arr->value[i] = 1;    // assign 1
    }
    // when present analog values is lower than average values 
    else
    {
      arr->value[i] = 0;
    }

    // total up the value of error buffer - shift bit corresponding with i variables (position of value in array)
    errID += (arr->value[i] << i);

    // turn on led that detected the line
    digitalWrite(led[i], (arr->value[i] & 1));
  }

  Serial.write(errID);
  // Serial.println(errID);             // (use this line for debug - show errorID)

}

// use for show array values in Serial Monitor
void showArr(ADC_ARR *arr)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    Serial.print(arr->value[i]);
    Serial.print("   ");
  }
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);       // set standard baud rate - can be changed if needed 

  // config LQR sensor pins
  for(uint8_t i = 0; i < 8; i++)
  {
    pinMode(sensor[i], INPUT);
    pinMode(led[i], OUTPUT);
  }

  // READ ME - config notes
  Serial.println("---------LQR ATMEGA32P UART---------\nAUTHOR: KAY CHIP\nVERSION: 1.0\nCOPYRIGHT: SPK-TURBO\n------------------------------------------------\n");
  Serial.println("PLEASE SELECT MODE:\n1. LEARN LIGHT COLOR AND SHOW LIGHT COLOR VALUES.\n2. LEARN DARK COLOR AND SHOW DARK COLOR VALUES.\n3. CALIBRATING GREY COLOR AND SHOW GREY COLOR VALUES.\n4. PROCESSING AND SEND ERROR ID TO UART.");
}

void loop() {
  // put your main code here, to run repeatedly:
  // check the UART connection
  if(Serial.available())
  {
    // receive value from UART with uint8_t variable
    Mode = Serial.read();
  }
  else
  {
    // communicating with computer
    Mode = Serial.parseInt();
  }

  // Serial.println(Mode);      // (use this line for debug - show mode which read from UART)
  switch(Mode)
  {
    // learn Light Color & show Light Color values
    case 1:
      learnColor(&lightColor);
      showArr(&lightColor);
      break;
    // learn Dark Color & show Dark Color values
    case 2:
      learnColor(&darkColor);
      showArr(&darkColor);
      break;
    // calibrating Grey Color & show Grey Color values
    case 3:
      calibColor(&greyColor);
      showArr(&greyColor);
      break;
    // read analog values and send errorID to UART
    case 4:
      while(1)
      {
        READ_ADC(&ADCVal);
        TransmitErrorID(&Error, errorID);
        // showArr(&Error);    // show the digital array (use for debug)
      }
      break;
  }
}