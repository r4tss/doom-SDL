#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>
// TODO FIRSST get rendering and movement around map working.

// TODO Enemies, Load maps. Weapons

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
        if((fabs(ceil(vx) - vx) < 0.05) && (fabs(ceil(vy) - vy) < 0.05))
          *type = 0;
        else if(xDir < 0.0)
          *type = map[(int)vx - 1][(int)vy].type;
        else
          *type = map[(int)vx][(int)vy].type;
        return(vdistance * cos(angle - baseAngle));
      }
    else
      {
        if((fabs(ceil(hx) -hx) < 0.05) && (fabs(ceil(hy) - hy) < 0.05))
          *type = 0;
        else if(yDir < 0.0)
          *type = map[(int)hx][(int)hy - 1].type;
        else
          *type = map[(int)hx][(int)hy].type;
        return(hdistance * cos(angle - baseAngle));
      }
  }

bool gameLoop(int map_w, int map_h, SDL_Renderer *renderer, SDL_Event event, struct Tile map[map_w][map_h])
  {
    int framerate = 30, ticksPerFrame = 1000/framerate, startFrame = 0, ray_n = 150;
    float fov = 90 * (M_PI/180), angle_step = fov/ray_n, map_depth = sqrt(pow(map_w, 2) + pow(map_h, 2));
    bool end = false;

    SDL_Rect floor;
    floor.w = 640;
    floor.h = 240;
    floor.x = 0;
    floor.y = 240;

    // Make minimap squares 60x60 pixels. DONE
    SDL_Rect minimap[map_w][map_h];
    for(int y = 0;y < map_h;y++)
      {
        for(int x = 0;x < map_w;x++)
          {
            minimap[x][y].x = x * 60 + 640;
            minimap[x][y].y = y * 60;
            minimap[x][y].w = 60;
            minimap[x][y].h = 60;
          }
      }

    struct Player player;
    player.x = 3.5;
    player.y = 3.5;
    player.angle = 0.0;
    player.health = 10;

    //struct Ray ray[ray_n];
    struct Ray ray[ray_n + 1];
    SDL_Rect rayDisplay[ray_n + 1];

    while(!end)
      {
        startFrame = SDL_GetTicks();
        SDL_RenderClear(renderer);

        // Make player shoot out rays from position with an FoV of 60 degrees
        for(int i = 0;i <= ray_n;i++)
          {
            ray[i].angle = player.angle - (fov / 2) + (angle_step * i);
            ray[i].xDir = cos(ray[i].angle);
            ray[i].yDir = sin(ray[i].angle);

            ray[i].distance = shootRay(ray[i].xDir, ray[i].yDir, player.x, player.y, ray[i].angle, player.angle, map_depth, map_w, map_h, map, &ray[i].type);
          }

        /*          --- CONTROLS ---          */
        while(SDL_PollEvent(&event))
          {
            switch(event.type)
              {
                case SDL_KEYDOWN:
                  switch(event.key.keysym.sym)
                    {
                      case SDLK_w:
                        player.x += cos(player.angle);
                        player.y += sin(player.angle);
                        break;
                      case SDLK_s:
                        player.x -= cos(player.angle);
                        player.y -= sin(player.angle);
                        break;
                      case SDLK_a:
                        player.angle -= 5 * angle_step;
                        if(player.angle < 0)
                          player.angle += 2 * M_PI;
                        break;
                      case SDLK_d:
                        player.angle += 5 * angle_step;
                        if(player.angle > 2 * M_PI)
                          player.angle -= 2 * M_PI;
                        break;
                    }
                  break;
                case SDL_QUIT:
                  return true;
                  break;
              }
          }

        /*          --- RENDERING ---          */
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
                SDL_RenderDrawLine(renderer, (x * 60) + 640, y * 60, (x * 60) + 700, y * 60);
                SDL_RenderDrawLine(renderer, (x * 60) + 640, y * 60, (x * 60) + 640, (y * 60) + 60);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
              }
          }

        for(int i = 0;i <= ray_n;i++)
          {
            rayDisplay[i].w = 640 / ray_n;
            rayDisplay[i].h = 300 / ray[i].distance;
            rayDisplay[i].x = i * (640 / ray_n);
            rayDisplay[i].y = 240 - (rayDisplay[i].h / 2);
            if(ray[i].type == 0)
              SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            else if(ray[i].type == 1)
              SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else if(ray[i].type == 2)
              SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            else if(ray[i].type == 3)
              SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRect(renderer, &rayDisplay[i]);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, (player.x * 60) + 640, player.y * 60, ((player.x + cos(ray[i].angle) * ray[i].distance) * 60) + 640, (player.y + sin(ray[i].angle) * ray[i].distance) * 60);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
          }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, (player.x * 60) + 640, player.y * 60, ((player.x + cos(player.angle)) * 60) + 640, (player.y + sin(player.angle)) * 60);
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
    int map_w = 8, map_h = 8;
    struct Tile map[map_w][map_h];
    bool quit = false;
    for(int y = 0;y < map_h;y++)
      {
        for(int x = 0;x < map_w;x++)
          {
            if(x == 0 || x == map_w - 1 || y == 0 || y == map_h - 1)
              map[x][y].type = 1;
            else if(x == 3 && y == 5)
              map[x][y].type = 2;
            else if(x == 2 && y >= 2 && y <= 6)
              map[x][y].type = 3;
            else
              map[x][y].type = 0;
          }
      }

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    window = SDL_CreateWindow("Doom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640 + 480, 480, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    while(!quit)
      {
        quit = gameLoop(map_w, map_h, renderer, event, map);
      }
  }
