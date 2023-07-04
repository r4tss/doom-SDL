#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

#define WINDOW_SIDE 800

struct Tile
{
  int type;
};

void printText(SDL_Renderer *renderer, TTF_Font *font, char *str, int x, int y, int h, int r, int g, int b)
  {
    SDL_Color c = {r, g, b, 255};

    SDL_Surface *s = TTF_RenderText_Solid(font, str, c);
    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);

    SDL_Rect rect = {x, y, strlen(str) * (h / 2), h};

    SDL_RenderCopy(renderer, t, NULL, &rect);
  }

int editor(SDL_Renderer *renderer, SDL_Event event, TTF_Font *font, int map_w, int map_h, struct Tile map[map_w][map_h])
  {
    int framerate = 30, ticksPerFrame = 1000/framerate, startFrame = 0;
    int mx, my, type = 0, buttonPressed = 0;
    float side = WINDOW_SIDE / map_w;
    bool quit = false;
    char ty[16] = "";

    SDL_Rect dest[map_w][map_h];

    for(int y = 0;y < map_h;y++)
      {
        for(int x = 0;x < map_w;x++)
          {
            map[x][y].type = 0;
            dest[x][y].w = side;
            dest[x][y].h = side;
            dest[x][y].x = x * side;
            dest[x][y].y = y * side + 40;
          }
      }

    while(!quit)
      {
        startFrame = SDL_GetTicks();
        SDL_RenderClear(renderer);
        while(SDL_PollEvent(&event))
          {
            switch(event.type)
              {
                case SDL_KEYDOWN:
                  switch(event.key.keysym.sym)
                    {
                      case SDLK_ESCAPE:
                        return 0;
                        break;
                      case SDLK_1:
                        type = 1;
                        break;
                      case SDLK_2:
                        type = 2;
                        break;
                      case SDLK_3:
                        type = 3;
                        break;
                      case SDLK_0:
                        type = 0;
                        break;
                      case SDLK_s:
                        return 1;
                        break;
                    }
                  break;
                case SDL_MOUSEBUTTONDOWN:
                  switch(event.button.button)
                    {
                      case SDL_BUTTON_LEFT:
                        buttonPressed = 1;
                        break;
                    }
                  break;
                case SDL_MOUSEBUTTONUP:
                  buttonPressed = 0;
                  break;
                case SDL_QUIT:
                  return 0;
                  break;
              }
          }
        if(buttonPressed == 1)
          {
            int x, y;
            SDL_GetMouseState(&x, &y);
            map[(int)(x / side)][(int)((y - 40) / side)].type = type;
          }

        SDL_GetMouseState(&mx, &my);
        for(int y = 0;y < map_h;y++)
          {
            for(int x = 0;x < map_w;x++)
              {
                if(dest[x][y].x < mx && dest[x][y].x + side > mx && dest[x][y].y < my && dest[x][y].y + side > my)
                    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
                else if(map[x][y].type == 0)
                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if(map[x][y].type == 1)
                  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                if(map[x][y].type == 2)
                  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                if(map[x][y].type == 3)
                  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                SDL_RenderFillRect(renderer, &dest[x][y]);
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                SDL_RenderDrawLine(renderer, x * side, (y * side) + 40, (x * side) + side, (y * side) + 40);
                SDL_RenderDrawLine(renderer, x * side, (y * side) + 40, x * side, (y * side) + side + 40);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
              }
          }
        sprintf(ty, "Current Type: %i", type);
        printText(renderer, font, ty, (WINDOW_SIDE / 2) - 100, 0, 40, 255, 255, 255);
        SDL_RenderPresent(renderer);
        int ticks = startFrame - SDL_GetTicks();
        if(ticks < ticksPerFrame)
          SDL_Delay(ticksPerFrame - ticks);
      }
    return 0;
  }

void save(SDL_Renderer *renderer, SDL_Event event, TTF_Font *font, int map_w, int map_h, struct Tile map[map_w][map_h])
  {
    int framerate = 30, ticksPerFrame = 1000/framerate, startFrame = 0, len = 0;
    bool done = false;
    char text[64] = "", filename_disp[64], filename[64];
    FILE *f;
    SDL_StartTextInput();
    while(!done)
      {
        startFrame = SDL_GetTicks();
        SDL_RenderClear(renderer);

        sprintf(filename_disp, "%s.map", text);

        while(SDL_PollEvent(&event))
          {
            switch(event.type)
              {
                case SDL_KEYDOWN:
                  switch(event.key.keysym.sym)
                    {
                      case SDLK_ESCAPE:
                        done = true;
                        break;
                      case SDLK_BACKSPACE:
                        text[strlen(text) - 1] = '\0';
                        len--;
                        break;
                      case SDLK_RETURN:
                        // Save the map to file with name text
                        f = fopen(filename, "r");
                        if(f == NULL)
                        {
                          printf("here\n");
                          sprintf(filename, "./maps/%s", filename_disp);
                          f = fopen(filename, "w");
                          fprintf(f, "%i %i\n", map_w, map_h);
                          for(int y = 0;y < map_h;y++)
                          {
                            for(int x = 0;x < map_w;x++)
                            {
                              if(x < map_w - 1)
                                fprintf(f, "|%i", map[x][y].type);
                              else
                                fprintf(f, "|%i|", map[x][y].type);
                            }
                            fprintf(f, "\n");
                          }
                          fclose(f);
                        }
                        break;
                    }
                  break;
                case SDL_TEXTINPUT:
                  if(strlen(text)<15)
                    {
                      strcat(text, event.text.text);
                    }
                  break;
              }
          }
        //sprintf(filename, 64, "%s.txt", text);
        printText(renderer, font, filename_disp, 0, 0, 100, 255, 255, 255);

        SDL_RenderPresent(renderer);
        int ticks = startFrame - SDL_GetTicks();
        if(ticks < ticksPerFrame)
          SDL_Delay(ticksPerFrame - ticks);
      }
  }

int main()
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("OpenSans-Bold.ttf", 20);

    window = SDL_CreateWindow("Doom Map Maker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIDE, WINDOW_SIDE + 40, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int map_w = 32, map_h = 32, s = 1;
    struct Tile map[map_w][map_h];

    while(s != 0)
      {
        s = editor(renderer, event, font, map_w, map_h, map);
        if(s == 1)
          save(renderer, event, font, map_w, map_h, map);
      }

    TTF_Quit();
    SDL_Quit();
    return 0;
}
