// Uno Flowers v2.0 (copy of uno tentacle)
// get this working w/ 1 first
// Nadine L.
// January 2016

//// LIBRARIES /////////////////////////////////////////////////////

#include <Adafruit_NeoPixel.h>
//#include <Servo.h>
#include <Adafruit_TiCoServo.h>

//// NEO PATTERNS CLASS /////////////////////////////////////////////////////
//// https://learn.adafruit.com/multi-tasking-the-arduino-part-3/utility-functions ////

 
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

//// SERVO //////////////////////////////////////
class FlowerServo {
  Adafruit_TiCoServo servo;              // the servo

  public:
    FlowerServo(){
      // eventually stuff
      int position;
      int last_position;
      int increment;
      
    }

  void Attach(int pin){
    servo.attach(pin);
  }
  void Detach(){
    servo.detach();
  }
  
  void Open(){
    servo.write(179);
  }

  void Close(){
    servo.write(20);
  }
};


//// SERIAL AND PYTHON //////////////////////////////////////////////
byte incomingByte; // from python
int command = 0; // command (1 = open, 2 = close)

//// SERVO //////////////////////////////////////////////////////////
static const uint8_t servoPins[] = {9,10};

int startServo = 0; // the incoming command shit

FlowerServo Flower1;
FlowerServo Flower2;


//// NEO PATTERNS ///////////////////////////////////////////////////////

NeoPatterns Strip1(32, 12, NEO_GRB + NEO_KHZ800, &Strip1Complete); 
NeoPatterns Strip2(5, 11, NEO_GRB + NEO_KHZ800, &Strip2Complete);

 
//// INITIALIZE //////////////////////////////////////////////////////////

void setup()
{
  // serial
  Serial.begin(9600);
  Serial.setTimeout(20); // set the timeout, the default is 1 second which is nuts.

  // servo instances
  Flower1.Attach(servoPins[0]); // attach servo pin 5
  Flower2.Attach(servoPins[1]); // attach servo pin 6
  
  
  // neopixel instances 
  Strip1.begin();
  Strip2.begin();
  Strip1.ColorWipe(Strip1.Color(100, 0, 0), 20);
  Strip2.ColorWipe(Strip1.Color(0, 100, 0), 20);
  Strip1.Update();
  Strip2.Update();

 
}
 
//// GO YOU BASTARD /////////////////////////////////////////////////////

void loop()
{
  
  Strip2.Update();
  Strip1.Update();

  if (Serial.available() > 0) 
  {
      //incomingByte = Serial.parseInt(); // use if testing from arduino input
      incomingByte = Serial.read(); // use if live
      command = incomingByte;
      startServo = ServoGo(command);
      
  }

  if (startServo == 11)
  {
    //Serial.println(startServo);
    Flower1.Open();
    // change neopixel patterns
    Strip1.ActivePattern = SCANNER;
    Strip1.TotalSteps = 32;
  }
  else if (startServo == 12)
  {
    Flower1.Close();
    //Serial.println(startServo);
    Strip1.ActivePattern = COLOR_WIPE;
    Strip1.Interval = 20;
    
  } 
  
  if (startServo == 21)
  {
    Flower2.Open();
    Strip2.ActivePattern = SCANNER;
    Strip2.TotalSteps = 5;
  }
  else if (startServo == 22){
    Flower2.Close();
    Strip2.ActivePattern = COLOR_WIPE;
    Strip2.Interval = 20;
  }
 
}

//// SERVO COMMAND /////////////////////////////

int ServoGo(int com)
{
  Serial.println("!inServoGo");
  Serial.println(com);
  return com;
}


//// COMPLETION ROUTINES - get called on completion of a pattern ///////////////////


void Strip1Complete()
{
  // Just leave it empty for now
  
}

void Strip2Complete()
{
  /// Just leave it empty for now
  
}

/*
if (startServo == 1)
  {
    //Flower1.Open();
    // change neopixel patterns
    //Serial.println(startServo);
    Strip1.ActivePattern = THEATER_CHASE;
    Strip1.Interval = 40;
    Strip2.ActivePattern = COLOR_WIPE;

  } else if (startServo == 2){
    //Flower2.Close();
    //Serial.println(startServo);
    Strip2.ActivePattern = THEATER_CHASE;
    Strip2.Interval = 100;
    Strip1.ActivePattern = COLOR_WIPE;

  } else {
    Strip1.ActivePattern = COLOR_WIPE;
    Strip2.ActivePattern = COLOR_WIPE;
  }

*/
//Strip2.RainbowCycle(random(0,10));
//Strip1.RainbowCycle(random(0,10));
  //Strip1.TheaterChase(Strip1.Color(255,255,0), Strip1.Color(0,0,50), 100);
  //Strip1.ColorWipe(Strip1.Color(255, 0, 0), 20);
  //Strip1.Fade(Strip1.Color(255,0,0), Strip1.Color(255,0,255), 100, 100, FORWARD);
  //Strip1.Scanner(Strip1.Color(255,0,0),20);
