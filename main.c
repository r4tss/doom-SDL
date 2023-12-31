#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1088

// TODO FIRSST get rendering and movement around map working. DONE DONE DONE

// Improving movement with strafing.

// TODO Enemies, Load maps. Weapons

// Map loading - needs to save maps to files. Maybe implement a map editor

// Enemies - Some kind of AI? gotta shoot at you and do damage. Maybe less chance to hit when moving

// Weapons - need some sort of graphic for different weapon types. Have to shoot and be able to hit things. Hitboxes for enemies and such.

struct Tile
{
  int type;
  bool discovered;
};

struct Player
{
  float x;
  float y;
  float angle;
  int health;
};

struct Ray
{
  float angle;
  float xDir;
  float yDir;
  float distance;
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

void loadMap(char *file, int map_w, int map_h, struct Tile map[map_w][map_h])
{
  FILE *f;
  char mapString[64];
  f = fopen(file, "r");

  fgets(mapString, 64, f);

  int ch = getc(f), x = 0, y = 0;
  while(ch != EOF)
  {
    printf("%c", ch);
    if(ch == '\n')
    {
      y++;
      x = 0;
    }
    else
    {
      map[x][y].type = ch - '0';
      x++;
    }
    ch = getc(f);
  }
}

float shootRay(float xDir, float yDir, float x, float y, float angle, float baseAngle, float map_depth, int map_w, int map_h, struct Tile map[map_w][map_h], int *type)
  {
    float stepX, stepY, vx, vy, hx, hy;
    bool hit;
    //Checking Vertical Intersections
    if(xDir < 0.0)
      {
        stepX = -1;
        vx = floor(x);
      }
    else
      {
        stepX = 1;
        vx = ceil(x);
      }
    if(yDir < 0.0)
      vy = y - (fabs(vx - x) * fabs(tan(angle)));
    else
      vy = y + (fabs(vx - x) * fabs(tan(angle)));

    //printf("xDir: %f yDir: %f\n", xDir, yDir);
    //printf("--- Vertical check ---\n");
    //printf("x: %f, y: %f", vx, vy);
    if(sqrt(pow(fabs(vx - x), 2) + pow(fabs(vy - y), 2)) > map_depth)
      hit = true;
    else if(xDir < 0.0 && map[(int)vx - 1][(int)vy].type != 0)
        hit = true;
    else if(map[(int)vx][(int)vy].type != 0)
      hit = true;
    else
      hit = false;
    //printf(" hit: %b\n", hit);
    while(!hit)
      {
        vx += stepX;
        if(yDir < 0.0)
          vy -= fabs(tan(angle));
        else
          vy += fabs(tan(angle));
        //printf("x: %f, y: %f", vx, vy);
        if(sqrt(pow(fabs(vx - x), 2) + pow(fabs(vy - y), 2)) > map_depth)
          hit = true;
        else if(xDir < 0.0 && map[(int)vx - 1][(int)vy].type != 0)
            hit = true;
        else if(map[(int)vx][(int)vy].type != 0)
          hit = true;
        //printf(" hit: %b\n", hit);
      }
    //printf("distance: %f\n", sqrt(pow(fabs(vx - x), 2) + pow(fabs(vy - y), 2)));
    //printf("--- Done ---\n\n");

    //Checking Horizontal Intersections
    if(yDir < 0.0)
      {
        stepY = -1;
        hy = floor(y);
      }
    else
      {
        stepY = 1;
        hy = ceil(y);
      }

    if(xDir < 0.0)
      hx = x - (fabs(hy - y) / fabs(tan(angle)));
    else
      hx = x + (fabs(hy - y) / fabs(tan(angle)));

    //printf("--- Horizontal check ---\n");
    //printf("x: %f, y: %f", hx, hy);
    if(sqrt(pow(fabs(hx - x), 2) + pow(fabs(y - hy), 2)) > map_depth)
      hit = true;
    else if(yDir < 0.0 && map[(int)hx][(int)hy - 1].type != 0)
        hit = true;
    else if(map[(int)hx][(int)hy].type != 0)
      hit = true;
    else
      hit = false;
    //printf(" hit: %b\n", hit);
    while(!hit)
      {
        hy += stepY;
        if(xDir < 0.0)
          hx -= 1/fabs(tan(angle));
        else
          hx += 1/fabs(tan(angle));
        //printf("x: %f, y: %f", hx, hy);
        if(sqrt(pow(fabs(hx - x), 2) + pow(fabs(hy - y), 2)) > map_depth)
            hit = true;
        else if(yDir < 0.0 && map[(int)hx][(int)hy - 1].type != 0)
            hit = true;
        else if(map[(int)hx][(int)hy].type != 0)
          hit = true;
        //printf(" hit: %b\n", hit);
      }
    //printf("distance: %f\n", sqrt(pow(fabs(hx - x), 2) + pow(fabs(hy - y), 2)));
    //printf("--- Done ---\n\n");

    float vdistance = sqrt(pow(fabs(vx - x), 2) + pow(fabs(vy - y), 2)), hdistance = sqrt(pow(fabs(hx - x), 2) + pow(fabs(hy - y), 2));
    if(vdistance < hdistance)
      {
        //if((fabs(ceil(vx) - vx) < 0.05) && (fabs(ceil(vy) - vy) < 0.05))
        //  *type = 0;
        if(xDir < 0.0)
          *type = map[(int)vx - 1][(int)vy].type;
        else
          *type = map[(int)vx][(int)vy].type;
        return(vdistance * cos(angle - baseAngle));
      }
    else
      {
        //if((fabs(ceil(hx) -hx) < 0.05) && (fabs(ceil(hy) - hy) < 0.05))
        //  *type = 0;
        if(yDir < 0.0)
          *type = map[(int)hx][(int)hy - 1].type;
        else
          *type = map[(int)hx][(int)hy].type;
        return(hdistance * cos(angle - baseAngle));
      }
  }

bool gameLoop(int map_w, int map_h, SDL_Renderer *renderer, SDL_Event event, SDL_Window* window, struct Tile map[map_w][map_h])
  {
    int framerate = 30, ticksPerFrame = 1000/framerate, startFrame = 0, ray_n = WINDOW_WIDTH / 2;
    float fov = 90 * (M_PI/180), angle_step = fov/ray_n, map_depth = sqrt(pow(map_w, 2) + pow(map_h, 2)), acceleration = 0.0, sideAcceleration = 0.0, mouseMovement;
    bool end = false;

    SDL_Rect floor = {0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2};
    SDL_Rect skybox = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2};

    // Make minimap squares 60x60 pixels. DONE
    SDL_Rect minimap[map_w][map_h];
    for(int y = 0;y < map_h;y++)
      {
        for(int x = 0;x < map_w;x++)
          {
            minimap[x][y].x = x * 34 + WINDOW_WIDTH;
            minimap[x][y].y = y * 34;
            minimap[x][y].w = 34;
            minimap[x][y].h = 34;
          }
      }

    struct Player player;
    player.x = 3.95;
    player.y = 3.95;
    player.angle = 0.0;
    player.health = 10;

    //struct Ray ray[ray_n];
    struct Ray ray[ray_n + 1];
    SDL_Rect rayDisplay[ray_n + 1];

    while(!end)
      {
        startFrame = SDL_GetTicks();
        SDL_RenderClear(renderer);

        // Make player shoot out rays from position with an FoV of 60 degrees DONE
        for(int i = 0;i <= ray_n;i++)
          {
            ray[i].angle = player.angle - (fov / 2) + (angle_step * i);
            ray[i].xDir = cos(ray[i].angle);
            ray[i].yDir = sin(ray[i].angle);

            ray[i].distance = shootRay(ray[i].xDir, ray[i].yDir, player.x, player.y, ray[i].angle, player.angle, map_depth, map_w, map_h, map, &ray[i].type);
          }

        /*          --- CONTROLS ---          */
        mouseMovement = 0;
        while(SDL_PollEvent(&event))
          {
            switch(event.type)
              {
                case SDL_MOUSEMOTION:
                  mouseMovement = (float)event.motion.xrel;
                  break;
                case SDL_QUIT:
                  return true;
                  break;
              }
          }
        player.angle += mouseMovement / 25;
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_W])
          {
            if(acceleration < 0.5)
              acceleration += 0.1;
           }
        if(keys[SDL_SCANCODE_S])
          {
            if(acceleration > -0.5)
              acceleration -= 0.1;
          }
        if(keys[SDL_SCANCODE_A])
          {
            if(sideAcceleration < 0.5)
              sideAcceleration -= 0.1;
          }
        if(keys[SDL_SCANCODE_D])
          {
            if(sideAcceleration > -0.5)
              sideAcceleration += 0.1;
          }
        if(keys[SDL_SCANCODE_Q])
          {
            player.angle -= 5 * angle_step;
            if(player.angle < 0.0)
              player.angle += 2 * M_PI;
          }
        if(keys[SDL_SCANCODE_E])
          {
            player.angle += 5 * angle_step;
            if(player.angle > 2 * M_PI)
              player.angle -= 2 * M_PI;
          }

        if(map[(int)(player.x + acceleration * cos(player.angle))][(int)(player.y + acceleration * sin(player.angle))].type == 0)
          {
            player.x += acceleration * cos(player.angle);
            player.y += acceleration * sin(player.angle);
          }
        if(map[(int)(player.x + sideAcceleration * cos(player.angle + M_PI_2))][(int)(player.y + sideAcceleration * sin(player.angle + M_PI_2))].type == 0)
          {
            player.x += sideAcceleration * cos(player.angle + M_PI_2);
            player.y += sideAcceleration * sin(player.angle + M_PI_2);
          }
        if(acceleration > 0.0)
          acceleration -= 0.1;
        else if(acceleration < 0.0)
          acceleration += 0.1;
        if(acceleration < 0.01 && acceleration > -0.01)
          acceleration = 0.0;

        if(sideAcceleration > 0.0)
          sideAcceleration -= 0.1;
        else if(sideAcceleration < 0.0)
          sideAcceleration += 0.1;
        if(sideAcceleration < 0.01 && sideAcceleration > -0.01)
          sideAcceleration = 0.0;

        /*          --- RENDERING ---          */
        SDL_SetRenderDrawColor(renderer, 0, 213, 255, 255);
        SDL_RenderFillRect(renderer, &skybox);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &floor);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for(int y = 0;y < map_h;y++)
          {
            for(int x = 0;x < map_w;x++)
              {
                if(map[x][y].type == 0)
                  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if(map[x][y].type == 1)
                  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                if(map[x][y].type == 2)
                  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                if(map[x][y].type == 3)
                  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                SDL_RenderFillRect(renderer, &minimap[x][y]);
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 50);
                SDL_RenderDrawLine(renderer, (x * 34) + WINDOW_WIDTH, y * 34, (x * 34) + 1920 + 34, y * 34);
                SDL_RenderDrawLine(renderer, (x * 34) + WINDOW_WIDTH, y * 34, (x * 34) + WINDOW_WIDTH, (y * 34) + 34);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
              }
          }

        for(int i = 0;i <= ray_n;i++)
          {
            rayDisplay[i].w = 2;
            rayDisplay[i].h = 800 / ray[i].distance;
            rayDisplay[i].x = i * 2;
            rayDisplay[i].y = WINDOW_HEIGHT / 2 - (rayDisplay[i].h / 2);
            //if(ray[i].type == 0)
            //  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            if(ray[i].type == 1)
              SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else if(ray[i].type == 2)
              SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            else if(ray[i].type == 3)
              SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRect(renderer, &rayDisplay[i]);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, (player.x * 34) + WINDOW_WIDTH, player.y * 34, ((player.x + cos(ray[i].angle) * (ray[i].distance / cos(ray[i].angle - player.angle))) * 34) + WINDOW_WIDTH, (player.y + sin(ray[i].angle) * (ray[i].distance / cos(ray[i].angle - player.angle))) * 34);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
          }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        //SDL_RenderDrawLine(renderer, (player.x * 60) + WINDOW_WIDTH, player.y * 60, ((player.x + cos(player.angle)) * 60) + WINDOW_WIDTH, (player.y + sin(player.angle)) * 60);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);
        int ticks = startFrame - SDL_GetTicks();
        if(ticks < ticksPerFrame)
          SDL_Delay(ticksPerFrame - ticks);
      }
    return true;
  }

int main()
  {
    int map_w = 32, map_h = 32;
    struct Tile map[map_w][map_h];
    bool quit = false;

    //for(int y = 0;y < map_h;y++)
    //  {
    //    for(int x = 0;x < map_w;x++)
    //      {
    //        if(x == 0 || x == map_w - 1 || y == 0 || y == map_h - 1)
    //          map[x][y].type = 1;
    //        else if(x == 3 && y == 5)
    //          map[x][y].type = 2;
    //        else if(x == 2 && y >= 2 && y <= 6)
    //          map[x][y].type = 3;
    //        else
    //          map[x][y].type = 0;
    //      }
    //  }

    loadMap("./maps/test-lineless.map", map_w, map_h, map);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    window = SDL_CreateWindow("Doom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH + 1088, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    while(!quit)
      {
        quit = gameLoop(map_w, map_h, renderer, event, window, map);
      }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
  }
