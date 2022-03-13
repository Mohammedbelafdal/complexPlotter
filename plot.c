#include "definition.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_haptic.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define defaultXStart -1.5
#define defaultXEnd 1.5
#define defaultYStart -1.5
#define defaultYEnd 1.5
#define FPS 60
#define defaultTextSize 18
#define tolerance 0.01
unsigned int giterator = 15;
unsigned int ganimation = 0;

int plot(SDL_Renderer *screen, settings *settings);
int setFunction(function *func, char *name, function *prev, function *next,
                double (*f)(double, double), SDL_bool visible,
                SDL_Colour colour, plotType plotType,
                functionType functionType);

SDL_Rect textRect = {
    .h = defaultTextSize, .w = defaultTextSize, .x = 10, .y = 10};
SDL_Color defaultColor = {.r = 0xAA, .g = 0xAA, .b = 0xAA, .a = 0xAA};
SDL_Color white = {.r = 0xAA, .g = 0xAA, .b = 0xAA, .a = 0xFF};
SDL_Color black = {.r = 0, .g = 0, .b = 0, .a = 0xAA};
SDL_Color red = {.r = 0xFF, .g = 0, .b = 0, .a = 0xAA};
SDL_Color green = {.r = 0, .g = 0xFF, .b = 0, .a = 0xAA};
SDL_Color blue = {.r = 0, .g = 0, .b = 0xFF, .a = 0xAA};
SDL_Color shade = {.r = 0, .g = 0, .b = 0, .a = 0x33};
SDL_Color transparent = {.r = 0, .g = 0, .b = 0, .a = 0};
float HueToRGB(float v1, float v2, float vH) {
  if (vH < 0)
    vH += 1;

  if (vH > 1)
    vH -= 1;

  if ((6 * vH) < 1)
    return (v1 + (v2 - v1) * 6 * vH);

  if ((2 * vH) < 1)
    return v2;

  if ((3 * vH) < 2)
    return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

  return v1;
}
SDL_Color hue2rgb(float h, float s, float l) {
  SDL_Color rgb;
  if (h == 0) {
    rgb.r = rgb.g = rgb.b = (unsigned char)(l * 255);
  } else {
    float v1, v2;
    float hue = (float)h / 360;

    v2 = (l < 0.5) ? (l * (1 + s)) : ((l + s) - (l * s));
    v1 = 2 * l - v2;

    rgb.r = (unsigned char)(255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
    rgb.g = (unsigned char)(255 * HueToRGB(v1, v2, hue));
    rgb.b = (unsigned char)(255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
  }

  return rgb;
}
void eventHandler(SDL_Event *events, settings *settings, SDL_Renderer *screen) {
  double width = fabs(settings->x_end - settings->x_start);
  double height = fabs(settings->y_end - settings->y_start);
  while (SDL_PollEvent(events)) {
    switch (events->type) // checks wich type of event
    {
    case SDL_QUIT:
      settings->running = SDL_FALSE;
    case SDL_MOUSEWHEEL:
      if (events->wheel.y < 0) {
        settings->y_end += 0.05 * settings->y_end;
        settings->x_end += 0.05 * settings->y_end;
        settings->x_start -= 0.05 * settings->x_start;
        settings->y_start -= 0.05 * settings->y_start;
        break;
      }
      if (events->wheel.y > 0) {
        settings->y_end -= 0.05 * settings->y_end;
        settings->x_end += 0.05 * settings->x_end;
        settings->x_start += 0.05 * settings->x_start;
        settings->y_start += 0.05 * settings->y_start;
        break;
      }

      // TODO:
      // ADD GRAB
      double projectedMouseX, projectedMouseY, projectedMouseX2,
          projectedMouseY2;
      double plotterWidth = fabs(settings->x_end - settings->x_start);
      double plotterHeight = fabs(settings->y_end - settings->y_start);
    case SDL_MOUSEBUTTONDOWN:
      switch (events->button.button) {
      case SDL_BUTTON_LEFT:
        SDL_GetMouseState(&settings->mouseX, &settings->mouseY);
        break;
      case SDL_BUTTON_RIGHT:
        SDL_GetMouseState(&settings->mouseX2, &settings->mouseY2);
        projectedMouseX =
            (((double)settings->mouseX / settings->screenWidth) * plotterWidth +
             settings->x_start); // perceived X
        projectedMouseY =
            -(((double)settings->mouseY / settings->screenHeight) *
                  plotterHeight -
              settings->y_end);
        projectedMouseX2 =
            (((double)settings->mouseX2 / settings->screenWidth) *
                 plotterWidth +
             settings->x_start); // perceived X
        projectedMouseY2 =
            -(((double)settings->mouseY2 / settings->screenHeight) *
                  plotterHeight -
              settings->y_end);
        settings->y_end = fmax(projectedMouseY, projectedMouseY2);
        settings->x_end = fmax(projectedMouseX, projectedMouseX2);
        settings->x_start = fmin(projectedMouseX, projectedMouseX2);
        settings->y_start = fmin(projectedMouseY, projectedMouseY2);
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_ESCAPE]) {
    settings->running = SDL_FALSE;
  }
  if (state[SDL_SCANCODE_E]) {
    settings->plotType++;
    SDL_Delay(100);
  }
  if (state[SDL_SCANCODE_LEFT]) {
    settings->x_start -= (double)1 / FPS;
    settings->x_end -= (double)1 / FPS;
  }
  if (state[SDL_SCANCODE_RIGHT]) {
    settings->x_start += (double)1 / FPS;
    settings->x_end += (double)1 / FPS;
  }
  if (state[SDL_SCANCODE_DOWN]) {
    settings->y_start -= (double)1 / FPS;
    settings->y_end -= (double)1 / FPS;
  }
  if (state[SDL_SCANCODE_UP]) {
    settings->y_start += (double)1 / FPS;
    settings->y_end += (double)1 / FPS;
  }
  if (state[SDL_SCANCODE_R]) {
    settings->y_start = defaultYStart;
    settings->y_end = defaultYEnd;
    settings->x_start = defaultXStart;
    settings->x_end = defaultXEnd;
  }
  if (state[SDL_SCANCODE_K]) {
    giterator++;
  }
  if (state[SDL_SCANCODE_J]) {
    giterator--;
  }
  if (state[SDL_SCANCODE_L]) {
    ganimation++;
  }
  if (state[SDL_SCANCODE_H]) {
    ganimation--;
  }
  if (state[SDL_SCANCODE_S]) {
  char *text=malloc(sizeof(char));
  sprintf(text,"screeshot%d.bmp",SDL_GetTicks());
  SDL_Surface *sshot = SDL_CreateRGBSurface(0, settings->screenWidth, settings->screenHeight, 32,
  0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
  SDL_RenderReadPixels(screen, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels,
  sshot->pitch); SDL_SaveBMP(sshot, text); SDL_FreeSurface(sshot);
  free(text);
}
}
SDL_Texture *textToSurface(char *text, SDL_Renderer *screen, SDL_Rect *textRect,
                           SDL_Color fg, SDL_Color bg) {
  // render Text
  TTF_Init();
  TTF_Font *defaultFont = TTF_OpenFont("Roboto-Bold.ttf", defaultTextSize);
  char *textInfo = (char *)text;
  textRect->h = TTF_FontHeight(defaultFont);
  textRect->w = strlen(textInfo) * (defaultTextSize);
  SDL_Surface *textSurface = TTF_RenderText(defaultFont, textInfo, fg, bg);
  SDL_Texture *textTexture;
  TTF_CloseFont(defaultFont);

  return SDL_CreateTextureFromSurface(screen, textSurface);
}
double sawTooth(double x, double y) {
  double a = 5;
  return fabs(modf(x, &a));
}
double circle(double x, double y) { return sqrt(x * x + y * y) - 1; }
double comp(double x, double y) {
  double _Complex z = x + I * y;
  double _Complex c = ((double)ganimation / 100) *10* (I);
  for (int i = 0; i < giterator; i++) {
    z = z* z *z* z*z + c;
  }
  return cabs(z);
}
double comp2(double x, double y) {
  double _Complex z = x + I * y;
  for (int i = 0; i < giterator; i++) {
    z = z * z + sqrt(2) / 2;
  }
  return cabs(z);
}
void renderText(SDL_Renderer *screen, settings *settings, corner corner,
                char *textInfo) {
  SDL_Texture *textTexture;
  SDL_Rect tempTextRect = textRect;
  textTexture = textToSurface(textInfo, screen, &tempTextRect, white, shade);
  int position = corner;
  switch (position) {
  case topLeft:
    tempTextRect.y += settings->topLeftOffset;
    SDL_RenderCopy(screen, textTexture, NULL, &tempTextRect);
    settings->topLeftOffset += defaultTextSize;
    break;
  case topRight:
    tempTextRect.y += settings->topRightOffset;
    tempTextRect.x = settings->screenWidth - tempTextRect.w;
    SDL_RenderCopy(screen, textTexture, NULL, &tempTextRect);
    settings->topRightOffset += defaultTextSize;
    break;
  case bottomRight:
    tempTextRect.y =
        settings->screenHeight - settings->bottomRightOffset - tempTextRect.h;
    tempTextRect.x = settings->screenWidth - tempTextRect.w;
    SDL_RenderCopy(screen, textTexture, NULL, &tempTextRect);
    settings->bottomRightOffset += defaultTextSize;
    break;
  default:
    break;
  }
  // SDL_free(textTexture);
}
void Render(SDL_Renderer *screen, settings *settings) {
  // clear screen
  int screenHeight = settings->screenHeight;
  int screenWidth = settings->screenWidth;
  SDL_SetRenderDrawBlendMode(screen, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(screen, 0x11, 0x11, 0x11, 200);
  SDL_RenderClear(screen);

  double plotterWidth = fabs(settings->x_end - settings->x_start);
  double plotterHeight = fabs(settings->y_end - settings->y_start);
  // plot
  SDL_SetRenderDrawColor(screen, 0xFF, 0xFF, 0xFF, 100);

  // vertical lines
  for (int i = settings->x_start; i <= settings->x_end; i++) {
    SDL_RenderDrawLineF(
        screen, (i - settings->x_start) / plotterWidth * screenWidth, 0,
        (i - settings->x_start) / plotterWidth * screenWidth, screenHeight);
  }
  // horizontal lines
  for (int i = settings->y_end; i >= settings->y_start; i--) {
    SDL_RenderDrawLineF(
        screen, 0, (settings->y_end - i) / plotterHeight * screenHeight,
        screenWidth, (settings->y_end - i) / plotterHeight * screenHeight);
  }

  // plot functions
  plot(screen, settings);
  // render text
  char *textInfo = (char *)malloc(sizeof(char) * 100);
  SDL_Texture *textTexture;
  SDL_Rect tempTextRect = textRect;

  sprintf(textInfo, "x_start: %.3g x_end:%.3g", settings->x_start,
          settings->x_end);
  renderText(screen, settings, topLeft, textInfo);
  sprintf(textInfo, "y_start: %.3g y_end:%.3g", settings->y_start,
          settings->y_end);
  renderText(screen, settings, topLeft, textInfo);
  sprintf(textInfo, "c=%.2f", (float)ganimation / 100);
  renderText(screen, settings, topLeft, textInfo);
  sprintf(textInfo, "i=%d", giterator);
  renderText(screen, settings, topLeft, textInfo);

  int mouseX, mouseY;
  double projectedMouseX, projectedMouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  projectedMouseX = (((double)mouseX / screenWidth) * plotterWidth +
                     settings->x_start); // perceived X
  projectedMouseY =
      -(((double)mouseY / screenHeight) * plotterHeight - settings->y_end);
  sprintf(textInfo, "mouse x: %.3f mouse y:%.3f", projectedMouseX,
          projectedMouseY);
  renderText(screen, settings, topRight, textInfo);
  projectedMouseX = (((double)settings->mouseX / screenWidth) * plotterWidth +
                     settings->x_start); // perceived X
  projectedMouseY =
      -(((double)settings->mouseY / screenHeight) * plotterHeight -
        settings->y_end);
  sprintf(textInfo, "mouse x1: %.3f mouse y1:%.3f", projectedMouseX,
          projectedMouseY);
  renderText(screen, settings, topRight, textInfo);
  projectedMouseX = (((double)settings->mouseX2 / screenWidth) * plotterWidth +
                     settings->x_start); // perceived X
  projectedMouseY =
      -(((double)settings->mouseY2 / screenHeight) * plotterHeight -
        settings->y_end);
  sprintf(textInfo, "mouse x2: %.3f mouse y2:%.3f", projectedMouseX,
          projectedMouseY);
  renderText(screen, settings, topRight, textInfo);
  float fps = 1000 / ((float)SDL_GetTicks() - (float)settings->lastTick);
  sprintf(textInfo, "FPS: %f/%d", fps, FPS);
  renderText(screen, settings, bottomRight, textInfo);

  SDL_RenderPresent(screen);
  

  free(textInfo);
}
int plot(SDL_Renderer *screen, settings *settings) {
  int screenHeight;
  int screenWidth;
  SDL_GetRendererOutputSize(screen, &screenWidth, &screenHeight);
  SDL_SetRenderDrawBlendMode(screen, SDL_BLENDMODE_BLEND);
  double plotterWidth = fabs(settings->x_end - settings->x_start);
  double plotterHeight = fabs(settings->y_end - settings->y_start);
  double heightRatio = screenHeight / plotterHeight;
  double widthRatio = screenWidth / plotterWidth;
  double virtualX;
  double virtualY;
  double projectedVirtualX;
  double projectedVirtualY;
  function *currentFunction = settings->functions;
  while (currentFunction != NULL) {
    if (currentFunction->visible == SDL_TRUE) {

      SDL_SetRenderDrawColor(
          screen, currentFunction->colour.r, currentFunction->colour.g,
          currentFunction->colour.b, currentFunction->colour.a);
      // ready to plot
      switch (currentFunction->functionType) {
      case rinr:
        for (int x = 0; x < screenWidth; x++) {
          virtualX = (((double)x / screenWidth) * plotterWidth +
                      settings->x_start); // x values on the plotter
          projectedVirtualX =
              (virtualX - settings->x_start) * widthRatio; // x values on screen
          virtualY = (double)currentFunction->f(virtualX, 0);
          projectedVirtualY = (settings->y_end - virtualY) * heightRatio;
          if (currentFunction->plotType == filled)
            SDL_RenderDrawLineF(screen, projectedVirtualX,
                                settings->y_end / plotterHeight * screenHeight,
                                projectedVirtualX, projectedVirtualY);
          if (currentFunction->plotType == outLine)
            SDL_RenderDrawPointF(screen, projectedVirtualX, projectedVirtualY);
        }
        break;
      case r2inr:
        for (int x = 0; x < screenWidth; x++) {
          virtualX = (((double)x / screenWidth) * plotterWidth +
                      settings->x_start); // x values on the plotter
          for (int y = 0; y < screenHeight; y++) {
            virtualY = (((double)y / screenHeight) * plotterHeight +
                        settings->y_start); // x values on the plotter
            projectedVirtualX = (virtualX - settings->x_start) *
                                widthRatio; // x values on screen
            projectedVirtualY = (settings->y_end - virtualY) * heightRatio;
            switch (currentFunction->plotType) {
            case filled:
              if (currentFunction->f(virtualX, virtualY) < tolerance) {
                SDL_RenderDrawPointF(screen, projectedVirtualX,
                                     projectedVirtualY);
              }
              break;
            case outLine:
              if (fabs(currentFunction->f(virtualX, virtualY)) < tolerance) {
                SDL_RenderDrawPointF(screen, projectedVirtualX,
                                     projectedVirtualY);
              }
              break;
            default:
              break;
            }
          }
        }
        break;
      case cinr:
        for (int x = 0; x < screenWidth; x++) {
          virtualX = (((double)x / screenWidth) * plotterWidth +
                      settings->x_start); // x values on the plotter
          for (int y = 0; y < screenHeight; y++) {
            virtualY = (((double)y / screenHeight) * plotterHeight +
                        settings->y_start); // x values on the plotter
            projectedVirtualX = (virtualX - settings->x_start) *
                                widthRatio; // x values on screen
            projectedVirtualY = (settings->y_end - virtualY) * heightRatio;
            long int value = (currentFunction->f(virtualX, virtualY));
            SDL_Color colour=hue2rgb(value, 255, 255);
            SDL_SetRenderDrawColor(
                screen, colour.r,
                colour.g,
                colour.b, 0xA0);
            SDL_RenderDrawPointF(screen, projectedVirtualX, projectedVirtualY);
          }
        }
      default:
        break;
      }
    }
    currentFunction = currentFunction->next;
  }

  return '\0';
}
int setFunction(function *func, char *name, function *prev, function *next,
                double (*f)(double, double), SDL_bool visible,
                SDL_Colour colour, plotType plotType,
                functionType functionType) {
  func->colour = colour;
  func->prev = prev;
  func->next = next;
  func->f = f;
  func->plotType = plotType;
  func->visible = visible;
  func->name = name;
  func->functionType = functionType;
  return EXIT_SUCCESS;
}
int main(int argc, char *argv[])

{

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }
  SDL_DisplayMode DM;
  SDL_Event event;
  settings settings = {.x_start = defaultXStart,
                       .y_start = defaultYStart,
                       .x_end = defaultXEnd,
                       .y_end = defaultYEnd,
                       .increasing = 1,
                       .animation = 1,
                       .functions = NULL,
                       .running = SDL_TRUE};
  settings.functions = malloc(10 * sizeof(function));
  setFunction(&settings.functions[0], "saw tooth", NULL, &settings.functions[1],
              circle, SDL_FALSE, red, filled, rinr);
  setFunction(&settings.functions[1], "circle", &settings.functions[0],
              &settings.functions[2], comp2, SDL_FALSE, green, outLine, cinr);
  setFunction(&settings.functions[2], "mandelbrot", &settings.functions[1],
              NULL, comp, SDL_TRUE, white, outLine, cinr);

  SDL_GetDesktopDisplayMode(0, &DM);
  SDL_Window *win = SDL_CreateWindow("plotter", // creates a window
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, DM.w, DM.h, 0);
  SDL_Renderer *screen = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetWindowResizable(win, SDL_TRUE);

  while (settings.running == SDL_TRUE) {
    settings.lastTick = SDL_GetTicks();
    SDL_GetRendererOutputSize(screen, &settings.screenWidth,
                              &settings.screenHeight);
    settings.topLeftOffset = settings.topRightOffset =
        settings.bottomRightOffset = settings.bottomLeftOffset = 0;
    Render(screen, &settings);
    eventHandler(&event, &settings, screen);
    if (settings.animation > 100 && (settings.increasing) > 0) {
      settings.increasing = -1;
    }
    if (settings.animation <= 0 && (settings.increasing) < 0) {
      settings.increasing = 1;
    }
    settings.animation += settings.increasing;
    // ganimation=settings.animation;
    settings.lastTick = SDL_GetTicks();
    SDL_Delay(1000 / FPS);
  }

  // destroy window
  SDL_DestroyWindow(win);

  // close SDL
  SDL_Quit();

  return EXIT_SUCCESS;
}
