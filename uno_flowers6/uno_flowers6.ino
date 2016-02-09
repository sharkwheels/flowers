#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };
 
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:
 
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
	
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }
 
    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }
 
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }
 
    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }
 
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
 
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }
 
    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }
 
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

//////////////////////////////////////// LOGIC //////////////////////////////////////////////

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define servoMin  170 // this is the 'minimum' pulse length count (out of 4096)
#define servoMax  800 // this is the 'maximum' pulse length count (out of 4096)


int pixelPins[] = {11,9};
int servoPins[] = {15,13};

const size_t n = sizeof(pixelPins) / sizeof(pixelPins[0]);
const size_t o = sizeof(servoPins) / sizeof(servoPins[0]);


// Initialize everything and prepare to start
NeoPatterns Strip1(32, 0, NEO_GRB + NEO_KHZ800, &stripComplete);
NeoPatterns Strip2(32, 0, NEO_GRB + NEO_KHZ800, &stripComplete);


//// SERIAL AND PYTHON //////////////////////////////////////////////
byte incomingByte; // from python
int command = 0; // command (1 = open, 2 = close)
int startServo = 0; // the incoming command shit

//// GAME END ////////////////////////////////////////
bool endOfGame = false;

void setup()
{
  Serial.begin(9600);
    
   

    shuffle();

    Strip1.begin();
    Strip2.begin();
    
    // Kick off a pattern
    Strip1.ColorWipe(Strip1.Color(100,0,0), 20); //r
    Strip2.ColorWipe(Strip2.Color(0,100,0), 20); // g
    //Strip1.RainbowCycle(3);
    //Strip2.RainbowCycle(3);

    pwm.begin();
	pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
	yield();

}
 
// Main loop
void loop()
{
    // Update the rings.
    Strip1.setPin(pixelPins[0]);
    Strip2.setPin(pixelPins[1]);
    Strip1.Update();
    Strip2.Update();
    

    if (Serial.available() > 0) 
	{
		//incomingByte = Serial.parseInt(); // use if testing from arduino input
		incomingByte = Serial.read(); // use if live
		command = incomingByte;
		startServo = ServoGo(command);
		
		  
	}

	if (!endOfGame){

		if (startServo == 11)
		{	

			if(pixelPins[0] == 11)
			{
				pwm.setPWM(servoPins[0], 0, servoMax); // grab servo 15
				strip1open();
			
			}
			else if(pixelPins[0] == 9)
			{
				pwm.setPWM(servoPins[1], 0, servoMax); // grab servo 13
				strip1open();
			} 
			
		
		
		}
		else if(startServo == 12)
		{
		
			if(pixelPins[0] == 11)
			{
				pwm.setPWM(servoPins[0], 0, servoMin); // grab servo 15
				strip1close();
            
			}
			else if(pixelPins[0] == 9)
			{
				pwm.setPWM(servoPins[1], 0, servoMin); // grab servo 13
				strip1close();
			} 

		}
		else if(startServo == 13){
			if(pixelPins[0] == 11)
			{
				pwm.setPWM(servoPins[0], 0, servoMin); // grab servo 15
				strip1finish();
            
			}
			else if(pixelPins[0] == 9)
			{
				pwm.setPWM(servoPins[1], 0, servoMin); // grab servo 13
				strip1finish();
			} 
		}


		/// Flower 2 /////
		if (startServo == 21)
		{
			if(pixelPins[1] == 11){
				pwm.setPWM(servoPins[0], 0, servoMax); // grab servo 15
				strip2open();
			
			}
			else if(pixelPins[1] == 9)
			{	
				pwm.setPWM(servoPins[1], 0, servoMax); // grab servo 13
				strip2open();
			}
		
		}
		else if(startServo == 22)
		{
			if(pixelPins[1] == 11){
				pwm.setPWM(servoPins[0], 0, servoMin); // grab servo 15
				strip2close();
			
			}
			else if(pixelPins[1] == 9)
			{
				pwm.setPWM(servoPins[1], 0, servoMin); // grab servo 13
				strip2close();
			}
		
		}
		else if(startServo == 23){
			if(pixelPins[1] == 11){
				pwm.setPWM(servoPins[0], 0, servoMin); // grab servo 15
				strip2finish();
			
			}
			else if(pixelPins[1] == 9)
			{
				pwm.setPWM(servoPins[1], 0, servoMin); // grab servo 13
				strip2finish();
			}

		}

		

	////////////////////////////////////////////////
	} // end of !game
	////////////////////////////////////////////////

	if (startServo == 70) 
	{
	endOfGame = true;
	shuffle();
	resetGame();
	}


} // end of loop

int ServoGo(int com)
{
	Serial.println("!inServoGo");
	Serial.println(com);
	return com;
}

void shuffle(){

	Serial.print("Shuffling pixel pins");
  	for (size_t i = 0; i < n - 1; i++)
    {
        size_t j = random(1, n - i);
        //size_t j = i + rand() / (RAND_MAX / (n - i) + 1); // ?? look up how this actually works
        int t = pixelPins[i];
        pixelPins[i] = pixelPins[j];
        pixelPins[j] = t;  
    }

    Serial.println("------------pixelPins--------------");
    for(size_t y = 0; y < n; y++) 
    {
    	Serial.println(pixelPins[y]);
    }

    
	
}

void resetGame(){
	
	// reset all the pins
	pwm.setPWM(servoPins[0], 0, servoMin);
	pwm.setPWM(servoPins[1], 0, servoMin);

	restingStrips();
	// reset the shit out of everything
	Serial.println("game end, resetting");
	endOfGame = false;
	startServo = 0;
	
	
}

/////// NEOPATTERNS //////////////////////////

void restingStrips(){

	Strip1.ActivePattern = COLOR_WIPE;
    Strip1.Interval = 20;
    Strip1.Color1 = Strip1.Color(100,0,0); 

    Strip2.ActivePattern = COLOR_WIPE;
	Strip2.Interval = 20;
	Strip2.Color1 = Strip2.Color(0,100,0);
	

}

/// STRIP 1 ///////////////////////////////////

void strip1open(){
	// open and flash
	Strip1.ActivePattern = THEATER_CHASE;
    Strip1.Interval = 100;
    Strip1.Color1 = Strip1.Color(100,0,100);        
    Strip1.Color2 = Strip1.Color(0,0,100);
}

void strip1close(){
	// close back to original color
	Strip1.ActivePattern = COLOR_WIPE;
    Strip1.Interval = 20;
    Strip1.Color1 = Strip1.Color(100,0,0); 
	
}

void strip1finish(){
	// become a rainbow
	Strip1.ActivePattern = RAINBOW_CYCLE;
	Strip1.Interval = 3;
	
	
}

/// STRIP 2 ///////////////////////////////////

void strip2open(){
	Strip2.ActivePattern = THEATER_CHASE;
	Strip2.Interval = 100;
	Strip2.Color1 = Strip2.Color(100,0,0);        
   	Strip2.Color2 = Strip2.Color(0,100,0);

}

void strip2close(){
	Strip2.ActivePattern = COLOR_WIPE;
	Strip2.Interval = 20;
	Strip2.Color1 = Strip2.Color(0,100,0);
	

}

void strip2finish(){

	Strip2.ActivePattern = RAINBOW_CYCLE;
	Strip2.Interval = 3;
	
}

/// STRIP 3 ///////////////////////////////////


void stripComplete() {

	//Strip1.ColorWipe(Strip1.Color(100,0,0), 20); //r
    //Strip2.ColorWipe(Strip2.Color(0,100,0), 20); // g
    

}




/*
int pins[] = {6, 7, 8};
int positions[] = {0, 1, 2};
Adafruit_NeoPixel strip[] = { Adafruit_NeoPixel(10, 11 , NEO_GRB),
                             Adafruit_NeoPixel(10, 10 , NEO_GRB),
                             Adafruit_NeoPixel(10, 9 , NEO_GRB)
                           };

void setup() {
 for(int i = 0; i> 2; i++){
 strip[i].begin();
 strip[i].show();
 }
}

void loop(){
 for(int i = 0; i> 2; i++){
   strip[positions[i]].setPixelColor(0, strip[positions[i]].Color(100, 100, 100));
   strip[positions[i]].show();
 }
 shuffle();
}

int shuffle(int positions){ 
 // change positions{ 1, 2 3} to {3, 1, 2}
 return positions;
}

*/


/*
// Initialize everything and prepare to start
NeoPatterns Strip1(32, 0, NEO_GRB + NEO_KHZ800, &StripComplete);
NeoPatterns Strip2(32, 0, NEO_GRB + NEO_KHZ800, &StripComplete);

void setup()
{
  Serial.begin(9600);
    
    // Initialize all the pixelStrips
    tester();

    Strip1.begin();
    Strip2.begin();
    
    // Kick off a pattern
    Strip1.TheaterChase(Strip1.Color(100,0,100), Strip1.Color(0,0,50), 100);
    Strip2.TheaterChase(Strip2.Color(100,0,0), Strip2.Color(0,0,50), 100);

}
 
// Main loop
void loop()
{
    // Update the rings.
    Strip1.Update();
    Strip2.Update();
}

void tester(){
	Strip1.setPin(11);
	Strip2.setPin(9);
}

void StripComplete() {

}*/

