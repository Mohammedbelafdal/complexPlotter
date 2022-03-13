#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
enum plotType
{
  outLine,
  filled,
}typedef plotType;
enum corner
{
  topLeft,
  topRight,
  bottomLeft,
  bottomRight

}typedef corner;
enum functionType
{
  rinr,
  r2inr,
  cinr,
}typedef functionType;
struct variable
{
  char* name;
  long double value;
  struct variable *prev,*next;
}typedef variable;
struct function{
  struct function *prev,*next;
  char* name;
  double (*f)(double , double);
  SDL_bool visible;
  SDL_Colour colour;
  functionType functionType;
  plotType plotType;
} typedef function;
struct settings {
  double x_start;
  double y_start;
  double x_end;
  double y_end;
  int increasing;
  unsigned char animation; // value going from 0 to 100
  int plotType;//changes plotting (outline,filled)
  struct function *functions;
  SDL_bool running;
  int mouseX,mouseY,mouseX2,mouseY2;
  int bottomLeftOffset,bottomRightOffset,topLeftOffset,topRightOffset;
  int screenHeight,screenWidth;
  Uint32 lastTick;
} typedef settings;
