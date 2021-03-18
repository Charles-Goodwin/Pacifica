//
//  "Pacifica"
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
//

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define DATA_PIN            3         // You may need to change this to match how you have wired up your Arduino
#define NUM_LEDS            60        // Amend this to match the number of leds
#define MAX_POWER_MILLIAMPS 500       // This is simply a power management function to throttle what power is fed to the leds
                                      // 500 is low and assumes that the leds are simply powered by your arduino which in turn 
                                      // is powered by a USB cable
                                      // You can crank this up if your leds are powered seperately 

#define LED_TYPE            WS6812B   // Apparently, this is the one you have - Just to check, is the strip 5mm in width?
#define COLOR_ORDER         GRB       // Each led has three elements - (R)ed, (G)reen and (B)lue 
                                      // This simply caters for the different ordering that some strips have

//////////////////////////////////////////////////////////////////////////

CRGB leds[NUM_LEDS];                  // This basically defines the buffer(array) for holding your led settings

void setup() {                        // Standard routine that is executed once and is generally dedicated to initialising stuff
    
  delay( 3000); // 3 second delay for boot recovery, and a moment of silence - A reference to Dan by the author, Mark

  //Initialise the FastLED library so it knows what it is dealing with
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)   
        .setCorrection( TypicalLEDStrip );
        
  // Set a limit to how much power the leds can draw.
  // 500 is sensible if the leds are powered entirely by the MCU (Arduino)
  // If an attempt is made to use too much power then the library automatically 
  // dims the leds to conserve energy        
  FastLED.setMaxPowerInVoltsAndMilliamps( 5, MAX_POWER_MILLIAMPS);
}

void loop() // Standard Routine for repectitive tasks
{
  // Only execute this code block every 20 milliseconds
  // Another way of looking at it is that this code block inherently 
  // has a frequency of (20/1000) 50 frames per second 
  EVERY_N_MILLISECONDS( 20) {
    // Go off and arange your led buffer to exactly how you want it
    pacifica_loop();
    // Use whatever settings are in the buffer to light up the leds
    FastLED.show();
  }
}

//////////////////////////////////////////////////////////////////////////
//
// The code for this animation is more complicated than other examples, and 
// while it is "ready to run", and documented in general, it is probably not 
// the best starting point for learning.  Nevertheless, it does illustrate some
// useful techniques - Thanks Mark!
//
//////////////////////////////////////////////////////////////////////////
//
// In this animation, there are four "layers" of waves of light.  
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then 
// another filter is applied that adds "whitecaps" of brightness where the 
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent 
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//

// Pretty dam good explanation. He combines a bunch of various size waves to give it's 'random' wave nature
// and then applies two filters to add some highlights and then enrich the greens/blues 


// This is where the colours are defined. It all looks a little criptic but it's quite straight forwrd to decipher
// As you can see, the colours are grouped into  palettes, pacifica_palette_1, 2 and 3.
// CRGBPalette16 is a type of pallet that allows you to define 16 colours - you can see them listed within the declaration.
// So each of your palettes has 16 specific colours defined but it allows you to select a colour from a spectrum of 256 colours
// FastLED acheives this by performing a transition between adjacent colour definitions to make up for the gaps.
// For example you could have a palette declared with the first two colours defined as white and red.
// Zero on the palette colour spectrum would return white but as you advanced along the spectrum the colour would become
// pinker and pinker until at position 16 red would be returned. 
// One last thing, the colours are defined below as hexadecimal numbers (base 16 number system) as apposed to decimal.
// Each RGB colour is represented by two hex values giving a range of 00(0) to FF(255). Zero being off, FF being full on
// 
// eg lets work out the colour - 0x0AF055

// Prefix 0x - this is just notation that the following value is depicted as hexadecimal
// 0A - (R)ed value and represents the decimal value 10
// F0 - (G)reen value and represents the decimal value 240
// 55 - (B)lue value and represents the decimal value 85
// By combining these values we get our specific colour which in the above example is predominantly green with a blue tinge.
// the small red element of 10 gives the effect of slightly lightening the colour
// If you look at the various colours defined, most of them have no red (00 as the first two digits) with weightings in the 
// green (second two digts) and the blue (last two digits) 
// The key point to take away is that all the colours rendered by the leds are simply refering to the three RGB values.
//
// Palettes are a critical element of fastLED so if there is something you don't understand then please ask

CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };


// Main routine called from the standard Loop routine
void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.

  // The list below defines all the key parameters that control the behaviour 
  // of the pattern

  // These pointers are used 
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;

  // You'll see the author use the beatsin command a lot
  // beatsin16 allows you to define the characteristics of a regular sine wave using three parameters
  // beats per minute followed by the minimum and maximum values the sine wave to oscilate between
  // eg the first definition below is setting the sine wave to 3 beats per minute (very slow) 
  // oscillating between the values of 179 and 269 
  // the clunky numbers used is a reflection of how the values were fine tuned to get the desired effect 
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Apply each of the four layers, with different scales and speeds, that vary over time
  
  // As Mark explains, the Pacifica pattern is achieved by adding layers of effects
  // Just below, you can see four layers being applied .
  // I'm happy to go into detail about what is being done but I think you will gain a better grasp of 
  // of how the ultimate effect is achieved by looking in turn at the layers in isolation

  // To this end I highly recommend, that you observe the effect generated by only leaving one of the four lines of 
  // code to be executed. This is achieved by rendering three of the lines as comments by prefixing the line with // 
  
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Treat these next two lines as applying a filter. The first one adds whitecaps 
  // And the other enriches the greens/blues
  // Comment out both lines and execute the code to see what effect you get
  // Then in turn uncomment the lines of code to see how they impact the final effect.
  // It may be worthwhile to video your observations or better yet have the two different versions
  // of code running side by side because the difference is subtle! 
  
  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // And this one makes the blues and greens richer/darker
  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}
