/*
  Name:		TimerInterval.ino
  Created:	7/22/2018 1:16:07 PM
  Author:	Chicodefe
*/

/* Timer Interval
  Activa y desactiva un relay en ciclos completos de una hora, cada medio ciclo se programaran en multiplos de 10 minutos.
*/

//Librerias para Serial
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
//Librerias para Pantalla Grafica
#include <U8g2lib.h>
//Librerias para Memoria
#include <EEPROM.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Connections to LCD display

//Pantalla
U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 9);

//Variables
int ledState = LOW;// ledState used to set the LED

//Definiciones de los pines a los que van conectados los botones
int IntervalUpButton = 2;
int IntervalDownButton = 3;
int TimerUpButton = 4;
int TimerDownButton = 5;

//Definiciones de los pines a los que van conectados las salidas
int Relay1 = 13;

// I don't want to lose my interval settings when the arduino loses power, so we use the Arduino's
// built in EEPROM memory cache to store settings when they are changed. These set the memory addresses
// for the off and on interval settings
int EEPROM_CYCLE_ADDRESS = 0;
int EEPROM_INTERVAL_ADDRESS = 1;


// These set how much the interval changes when you press the up and down buttons
// Values in milliseconds
//long IntervalSteps = 300000;
//long TimerSteps = 300000;

long IntervalSteps = 30000;
long TimerSteps = 30000;

// If you hold down the up or down buttons, the interval will count up or down until you let go
// This sets the pause time between changes as you hold doun the button. Value in millisaconds.
int ButtonPressWait = 500;

// In order to make the up/down buttons count continuously up or down with a pause between jumps,
// we need a timer buffer to remember how long ago the last jump was.
long IntervalUpBuffer;
long IntervalDownBuffer;
long TimerUpBuffer;
long TimerDownBuffer;

// These are the interval variables
long Cycle;
long Interval;
long Timer;
long Activo;
long Inactivo;
int Status;


// Generally, you should use "unsigned long" for variables that hold time. The value will quickly become too large for an int to store
unsigned long PreviousMillis = 0;// millis() returns an unsigned long.
unsigned long IntervalMillis = 0;// millis() returns an unsigned long.
unsigned long CurrentMillis = 0;// millis() returns an unsigned long.

unsigned long time;

void setup()// the setup function runs once when you press reset or power the board
{//Parametros Serial
	Serial.begin(9600);// initialize serial communication at 9600 bits per second

	// Enable this code and push program once on the first run of a new board or to move EEPROM registers.
	  // Then disable and push the program to the board again for normal operation.
	EEPROM.write(EEPROM_CYCLE_ADDRESS,3);
	EEPROM.write(EEPROM_INTERVAL_ADDRESS, 1);


	  // This grabs the stored intervals from the EEPROM cache on startup
	Cycle = EEPROM.read(EEPROM_CYCLE_ADDRESS) * 60000;
	Interval = EEPROM.read(EEPROM_INTERVAL_ADDRESS) * 60000;


	Status = 1;

	// Iniciando el Timer con el valor del intervalo guadado
	Timer = Interval;


	// Initilize last Interval time buffer, set it to right now.
	Activo = Interval;
	Inactivo = Cycle - Activo;

	pinMode(Relay1, OUTPUT);
	digitalWrite(Relay1, LOW);

	pinMode(IntervalUpButton, INPUT);
	pinMode(IntervalDownButton, INPUT);
	pinMode(TimerUpButton, INPUT);
	pinMode(TimerDownButton, INPUT);

	// This enables the Arduino's built in pull up resistors on these pins to make wiring pushbuttons much easier.
	digitalWrite(IntervalUpButton, HIGH);
	digitalWrite(IntervalDownButton, HIGH);
	digitalWrite(TimerUpButton, HIGH);
	digitalWrite(TimerDownButton, HIGH);

	//Iniciando Pantalla
	u8g2.begin();
}

void loop()// the loop function runs over and over again until power down or reset
{
	CurrentMillis = millis();

	//Asignar el tiempo desde transcurrido
	IntervalMillis = CurrentMillis - PreviousMillis;
	

	if (Status == 1) {
		Activo = Activo - IntervalMillis;
		if (Activo >= 0) {
			Serial.print("Activo");
			Serial.println("");
			digitalWrite(Relay1, HIGH);
			Spray();
		}
		else {
			Status = 0;
			Activo = Interval;
		}

	}
	else {
		Inactivo = Inactivo - IntervalMillis;
		if (Inactivo >= 0) {
			Serial.print("Inactivo");
			Serial.println("");
			digitalWrite(Relay1, LOW);
			Spray();
		}
		else {
			Status = 1;
			Inactivo = Cycle - Activo;
		}


	}



	Serial.print("Status: ");
	Serial.println(Status);

	Serial.print("CurrentMillis: ");
	Serial.println(CurrentMillis);

	Serial.print("PreviousMillis: ");
	Serial.println(PreviousMillis);

	Serial.print("IntervalMillis: ");
	Serial.println(IntervalMillis);

	Serial.print("Timer: ");
	Serial.println(Timer);

	Serial.print("Activo: ");
	Serial.println(Activo);

	Serial.print("Inactivo: ");
	Serial.println(Inactivo);



	Serial.print("Cycle: ");
	Serial.println(Cycle);



	Serial.print("Interval: ");
	Serial.println(Interval);

	Serial.println("");




	u8g2.clearBuffer();// clear the internal memory
	u8g2.firstPage();
	do
	{

		if (Status == 1) {
			u8g2.setFont(u8g2_font_profont15_tf);// choose a suitable font
			u8g2.drawStr(0, 14, "Activo:");
			u8g2.setCursor(80, 14);
			u8g2.print(Activo / 1000 / 60);
			u8g2.print(":");
			u8g2.print(Activo / 1000);
		}
		else {
			u8g2.setFont(u8g2_font_profont15_tf);// choose a suitable font
			u8g2.drawStr(0, 14, "Inactivo:");
			u8g2.setCursor(80, 14);
			u8g2.print(Inactivo / 1000 / 60);
			u8g2.print(":");
			u8g2.print(Inactivo / 1000);
		}


		u8g2.setFont(u8g2_font_profont15_tf);// choose a suitable font
		u8g2.drawStr(0, 30, "Timer:");
		u8g2.setCursor(80, 30);
		u8g2.print(Timer / 1000 );

		u8g2.drawStr(0, 45, "Activo:");
		u8g2.setCursor(80, 45);
		u8g2.print(Interval / 1000 );

		u8g2.drawStr(0, 60, "Inactivo:");
		u8g2.setCursor(80, 60);
		u8g2.print(Cycle - Interval / 1000 );

	} while (u8g2.nextPage());


	if (digitalRead(IntervalUpButton) == LOW) {
		IntervalUp();
	}

	if (digitalRead(IntervalDownButton) == LOW) {
		IntervalDown();
	}

	if (digitalRead(TimerUpButton) == LOW) {
		TimerUp();
	}

	if (digitalRead(TimerDownButton) == LOW) {
		TimerDown();
	}




	

	PreviousMillis = millis();
	delay(1000);

}





////Asignar el tiempo desde transcurrido
//unsigned long currentMillis = millis();

//if (currentMillis - previousMillis1 >= intervalMillis1)
//{
//	// save the last time you blinked the LED
//	previousMillis1 = currentMillis;

//	// if the LED is off turn it on and vice-versa:
//	if (ledState == LOW)
//	{
//		ledState = HIGH;
//	}
//	else
//	{
//		ledState = LOW;
//	}

//	// set the LED with the ledState of the variable:
//	digitalWrite(13, ledState);
//}



//}

void Spray(void) {
	/*digitalWrite(Relay1, HIGH);
	long SprayBuffer = millis() + Interval;
	while (millis() < SprayBuffer) {

	}
	digitalWrite(Relay1, LOW);*/
}


// Do the hold down for continuous change loop, and write new value to the EEPROM
void IntervalUp(void) {
	if (Interval < Cycle) {
		if (IntervalUpBuffer + ButtonPressWait < millis()) {
			Interval = Interval+IntervalSteps;
			EEPROM.write(EEPROM_INTERVAL_ADDRESS, Interval / 60000);
			Activo = Interval;
			Inactivo = Cycle - Interval;
			IntervalUpBuffer = millis();
			u8g2.clearDisplay();
		}
	}
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void IntervalDown(void) {
	if (Interval > 0) {
		if (IntervalDownBuffer + ButtonPressWait < millis()) {
			Interval = Interval- IntervalSteps;
			EEPROM.write(EEPROM_INTERVAL_ADDRESS, Interval / 60000);
			Activo = Interval;
			Inactivo = Cycle - Interval;
			IntervalDownBuffer = millis();
			u8g2.clearDisplay();
		}
	}
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void TimerUp(void) {
	if (Timer < 3600000) {
		if (TimerUpBuffer + ButtonPressWait < millis()) {
			Timer += TimerSteps;
			TimerUpBuffer = millis();
		}
	}
}

// Do the hold down for continuous change loop, and write new value to the EEPROM
void TimerDown(void) {
	if (Timer > 300000) {
		if (TimerDownBuffer + ButtonPressWait < millis()) {
			Timer -= TimerSteps;
			TimerDownBuffer = millis();
		}
	}
}































