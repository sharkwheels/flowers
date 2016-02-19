#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <elapsedMillis.h>


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

#define servoMin  190 // this is the 'minimum' pulse length count (out of 4096)
#define servoMax  500 // this is the 'maximum' pulse length count (out of 4096)


int pixelPins[] = {11,10,9,8};
int servoPins[] = {15,14,13,12};
int finalServo = 6;
int finalStrip = 7;

const size_t n = sizeof(pixelPins) / sizeof(pixelPins[0]);
const size_t o = sizeof(servoPins) / sizeof(servoPins[0]);

// Initialize everything and prepare to start
NeoPatterns Strip1(6, 0, NEO_GRB + NEO_KHZ800, &stripComplete);
NeoPatterns Strip2(6, 0, NEO_GRB + NEO_KHZ800, &stripComplete);
NeoPatterns Strip3(6, 0, NEO_GRB + NEO_KHZ800, &stripComplete);
NeoPatterns Strip4(6, 0, NEO_GRB + NEO_KHZ800, &stripComplete);
NeoPatterns Fstrip(6, 0, NEO_GRB + NEO_KHZ800, &stripComplete);

//// SERIAL AND PYTHON //////////////////////////////////////////////
byte incomingByte; // from python
int command = 0; // command (1 = open, 2 = close)
int startServo = 0; // the incoming command shit

//// GAME CONTROL ////////////////////////////////////////
bool endOfGame = false;
elapsedMillis timeElapsed; 
unsigned int wait = 5000;

//// SETUP ////////////////////////////////////////

void setup()
{
  Serial.begin(9600);

    shuffle();

    Strip1.begin();
    Strip2.begin();
    Strip3.begin();
    Strip4.begin();
    Fstrip.begin();
    
    // Kick off a pattern
    Strip1.ColorWipe(Strip1.Color(40,0,0), 20); //red
    Strip2.ColorWipe(Strip2.Color(0,40,0), 20); // green
    Strip3.ColorWipe(Strip3.Color(40,0,40), 20); // purple
    Strip4.ColorWipe(Strip4.Color(0,0,40), 20); // blue
    Fstrip.ColorWipe(Strip4.Color(40,40,40), 20); // white

    pwm.begin();
	pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
	yield();
    
    Serial.println("setup done");

}



 
//// LOOP ////////////////////////////////////////
void loop()
{   

    /// Strip Update and Master Pins //////////////////////
    Strip1.setPin(pixelPins[0]);
    Strip2.setPin(pixelPins[1]);
    Strip3.setPin(pixelPins[2]);
    Strip4.setPin(pixelPins[3]);
    Fstrip.setPin(finalStrip);

    Strip1.Update();
    Strip2.Update();
    Strip3.Update();
    Strip4.Update();
    Fstrip.Update();



    /// Serial Start and Read ////////////////////////////////////
    if (Serial.available() > 0) 
    {
        //incomingByte = Serial.parseInt(); // use if testing from arduino input
        incomingByte = Serial.read(); // use if live
        command = incomingByte;
        startServo = ServoGo(command);
        timeElapsed = 0;   
    }



    /// Servo and Light Control /////////////////////////////////////////////

    if (!endOfGame)
    {   
        /// one ////////////////////////////////
        if (startServo == 11)
        {
            openFlowers(pixelPins[0],1); 
        }
        if(startServo == 12)
        {
            closeFlowers(pixelPins[0],1);
        }
        if(startServo == 13)
        {
            finishFlowers(pixelPins[0],1);
        }

        /// two ////////////////////////////////
        if (startServo == 21)
        {
            openFlowers(pixelPins[1],2);
        }
        else if(startServo == 22)
        {
            closeFlowers(pixelPins[1],2);
        }
        else if(startServo == 23)
        {
             finishFlowers(pixelPins[1],2);
        }

        /// Three //////////////////////////////////////////////////

        if (startServo == 31)
        {
            openFlowers(pixelPins[2],3);
        }
        else if(startServo == 32)
        {
            closeFlowers(pixelPins[2],3);
        }
        else if(startServo == 33)
        {
            finishFlowers(pixelPins[2],3);
        }

        /// FOUR //////////////////////////////////////////////////
        
        if (startServo == 41)
        {
            openFlowers(pixelPins[3],4);
        }
        else if(startServo == 42)
        {
            closeFlowers(pixelPins[3],4);
        }
        else if(startServo == 43)
        {   

            finishFlowers(pixelPins[0],1);
            finishFlowers(pixelPins[1],2);
            finishFlowers(pixelPins[2],3);
            finishFlowers(pixelPins[3],4);
            pwm.setPWM(finalServo, 0, servoMax);
            
            Fstrip.ActivePattern = THEATER_CHASE;
            Fstrip.Interval = 40;
            Fstrip.Color1 = Fstrip.Color(40,50,60);        
            Fstrip.Color2 = Fstrip.Color(0,0,40);
            
            if (timeElapsed >= wait) 
            { 
                startServo = 70;  
            }
        }
           

    }

    /// RESET GAME //////////////////////////////////////////////
    if (startServo == 70) 
    {   
        endOfGame = true;
        resetGame();  
    }



}

//// FUNCTIONS ////////////////////////////////////////

int ServoGo(int com)
{
    Serial.println("!inServoGo");
    Serial.println(com);
    return com;
}

void shuffle()
{

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
    // RESET ///////////////////////////////////////
    Serial.println("resetting Game");

    // close all the servos
    pwm.setPWM(servoPins[0], 0, servoMin);
    pwm.setPWM(servoPins[1], 0, servoMin);
    pwm.setPWM(servoPins[2], 0, servoMin);
    pwm.setPWM(servoPins[3], 0, servoMin);
    pwm.setPWM(finalServo, 0, servoMin);

    //reset all the strips
    restingStrips();

    // shuffle pins
    shuffle();

    //clear all the booleans
    startServo = 0;
    timeElapsed = 0;
    endOfGame = false;

}

///// SERVO CONTROL //////////////////////////////////

void openFlowers(int p, int s){
    if(p == 11)
    {
        pwm.setPWM(servoPins[0], 0, servoMax); // grab servo 15
        if(s == 1){
            strip1open(); 
        }
        else if(s == 2)
        {
            strip2open();
        }
        else if(s == 3)
        {
            strip3open();
            //Serial.println(p);
        }
        else if(s == 4)
        {
            strip4open();
        }

                
    }
    else if(p == 10)
    {
        pwm.setPWM(servoPins[1], 0, servoMax); // grab servo 14
        if(s == 1){
            strip1open(); 
        }
        else if(s == 2){
            strip2open();
        }
        else if(s == 3)
        {
            strip3open();
            //Serial.println(p);
        } 
        else if(s == 4)
        {
            strip4open();
        }
        
    }
    else if(p == 9)
    {
        pwm.setPWM(servoPins[2], 0, servoMax); // grab servo 13
        if(s == 1){
            strip1open(); 
        }
        else if(s == 2){
            strip2open();
        }
        else if(s == 3)
        {
            strip3open();
            //Serial.println(p);
        }
        else if(s == 4)
        {
            strip4open();
        }
        
    }
    else if(p == 8)
    {
        pwm.setPWM(servoPins[3], 0, servoMax); // grab servo 13
        if(s == 1){
            strip1open(); 
        }
        else if(s == 2){
            strip2open();
        }
        else if(s == 3)
        {
            strip3open();
            //Serial.println(p);
        }
        else if(s == 4)
        {
            strip4open();
        }
    }
    
}

void closeFlowers(int p, int s)
{
    if(p == 11)
    {
        pwm.setPWM(servoPins[0], 0, servoMin); // grab servo 15
        if(s == 1)
        {
            strip1close();
        }
        else if(s == 2)
        {
            strip2close();
        }
        else if(s == 3)
        {
            strip3close();

        }
        else if(s == 4)
        {
            strip4close();
        }    
    }
    else if(p == 10)
    {
        pwm.setPWM(servoPins[1], 0, servoMin); // grab servo 14
        if(s == 1)
        {
            strip1close();
        }
        else if(s == 2)
        {
            strip2close();
        } 
        else if(s == 3)
        {
            strip3close();
        }
        else if(s == 4)
        {
            strip4close();
        }        
    }
    else if(p == 9)
    {
        pwm.setPWM(servoPins[2], 0, servoMin); // grab servo 13
        if(s == 1)
        {
            strip1close();
        }
        else if(s == 2)
        {
            strip2close();
        } 
        else if(s == 3)
        {
            strip3close();
        }
        else if(s == 4)
        {
            strip4close();
        }
    }
    
    else if(p == 8)
    {
        pwm.setPWM(servoPins[3], 0, servoMin); // grab servo 13
        if(s == 1)
        {
            strip1close();
        }
        else if(s == 2)
        {
            strip2close();
        } 
        else if(s == 3)
        {
            strip3close();
        }
        else if(s == 4)
        {
            strip4close();
        }
    }
}

void finishFlowers(int p, int s)
{
    if(p == 11)
    {
        pwm.setPWM(servoPins[0], 0, servoMax); // grab servo 15
        if(s == 1)
        {
            strip1finish();  
        }
        else if(s == 2)
        {
            strip2finish();  
        }
        else if(s == 3)
        {
            strip3finish();  
        }
        else if(s == 4)
        {
            strip4finish();
        } 
     
    }
    else if(p == 10)
    {
        pwm.setPWM(servoPins[1], 0, servoMax); // grab servo 14
        if(s == 1)
        {
            strip1finish();  
        }
        else if(s == 2)
        {
            strip2finish(); 
        }
        else if(s == 3)
        {
            strip3finish();  
        }
        else if(s == 4)
        {
            strip4finish();
            
        }
      
    } 
    else if(p == 9)
    {
        pwm.setPWM(servoPins[2], 0, servoMax); // grab servo 13
        if(s == 1)
        {
            strip1finish();  
        }
        else if(s == 2)
        {
            strip2finish(); 
        }
        else if(s == 3)
        {
            strip3finish();  
        } 
        else if(s == 4)
        {
            strip4finish();
        } 
                
    }
    else if(p == 8)
    {
        pwm.setPWM(servoPins[3], 0, servoMax); // grab servo 13
        if(s == 1)
        {
            strip1finish();  
        }
        else if(s == 2)
        {
            strip2finish(); 
        }
        else if(s == 3)
        {
            strip3finish();  
        } 
        else if(s == 4)
        {
            strip4finish();  
        }
    }
    
}

/////// NEOPATTERNS //////////////////////////



void restingStrips(){

    Strip1.ActivePattern = COLOR_WIPE;
    Strip1.Interval = 20;
    Strip1.Color1 = Strip1.Color(40,0,0); 

    Strip2.ActivePattern = COLOR_WIPE;
    Strip2.Interval = 20;
    Strip2.Color1 = Strip2.Color(0,40,0);

    Strip3.ActivePattern = COLOR_WIPE;
    Strip3.Interval = 20;
    Strip3.Color1 = Strip3.Color(40,0,40);

    Strip4.ActivePattern = COLOR_WIPE;
    Strip4.Interval = 20;
    Strip4.Color1 = Strip4.Color(0,40,40);

    Fstrip.ActivePattern = COLOR_WIPE;
    Fstrip.Interval = 20;
    Fstrip.Color1 = Fstrip.Color(40,40,40); 

}

/// STRIP 1 ///////////////////////////////////

void strip1open(){
    // open and flash
    Strip1.ActivePattern = THEATER_CHASE;
    Strip1.Interval = 40;
    Strip1.Color1 = Strip1.Color(40,0,0); // red      
    Strip1.Color2 = Strip1.Color(40,0,40); // purple
}

void strip1close(){
    // close back to original color
    Strip1.ActivePattern = COLOR_WIPE;
    Strip1.Interval = 20;
    Strip1.Color1 = Strip1.Color(40,0,0); // red
    
}

void strip1finish(){
    // become a rainbow
    Strip1.ActivePattern = RAINBOW_CYCLE;
    Strip1.Interval = 3;

    
    
}

/// STRIP 2 ///////////////////////////////////

void strip2open(){
    Strip2.ActivePattern = THEATER_CHASE;
    Strip2.Interval = 40;
    Strip2.Color1 = Strip2.Color(0,40,0);  //green  
    Strip2.Color2 = Strip2.Color(0,0,40); // blue

}

void strip2close(){
    Strip2.ActivePattern = COLOR_WIPE;
    Strip2.Interval = 20;
    Strip2.Color1 = Strip2.Color(0,40,0); // green
    

}

void strip2finish(){

    Strip2.ActivePattern = RAINBOW_CYCLE;
    Strip2.Interval = 40;
    
}

/// STRIP 3 ///////////////////////////////////

void strip3open(){
    Strip3.ActivePattern = THEATER_CHASE;
    Strip3.Interval = 40;
    Strip3.Color1 = Strip2.Color(40,40,40);     // whiteish   
    Strip3.Color2 = Strip2.Color(40,0,40); // purple

}

void strip3close(){
    Strip3.ActivePattern = COLOR_WIPE;
    Strip3.Interval = 20;
    Strip3.Color1 = Strip2.Color(40,0,40); // purple
    

}

void strip3finish(){

    Strip3.ActivePattern = RAINBOW_CYCLE;
    Strip3.Interval = 3;
    
}

/// STRIP 4 ///////////////////////////////////

void strip4open(){
    Strip4.ActivePattern = THEATER_CHASE;
    Strip4.Interval = 40;
    Strip4.Color1 = Strip2.Color(40,40,0);        
    Strip4.Color2 = Strip2.Color(0,40,40);

}

void strip4close(){
    Strip4.ActivePattern = COLOR_WIPE;
    Strip4.Interval = 20;
    Strip4.Color1 = Strip2.Color(0,40,40);
}

void strip4finish(){

    Strip4.ActivePattern = RAINBOW_CYCLE;
    Strip4.Interval = 3;
    
}

void stripComplete() {

}
