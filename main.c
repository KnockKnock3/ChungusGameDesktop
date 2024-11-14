#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define NUM_OF_DEATH_MESSAGES 6
#define NUM_OF_QUOTES 8
#define FONT_SIZE 20
#define CHUNGUS_SPEED 15 * 10
#define TURRET_COOLDOWN 1.0f

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

typedef struct Chungus {
    Rectangle rectange;
    bool active;
} Chungus;

typedef struct Turret {
    Vector2 position;
    float cooldown;
    Vector2 target;
} Turret;

const Rectangle ChungusSize = (Rectangle) {0, 0, 87, 136};

Chungus *chungi;
Turret *turrets;
int chungi_length = 0;
int turrets_length = 0;

char *death_messages[] = {"The Chungus got the better of you", "The Chungus where too big for you", "Who's the big chungus now?", "Big chungus chungused you out", "Chungus is too cool for school", "You got big chungused"};
char *quotes[] = {"\"A single death is a tragedy; a million deaths is a statistic.\"", "\"Those who vote decide nothing. Those who count the vote decide everything.\"", "\"In the Soviet army it takes more courage to retreat than advance.\"", "\"Death is the solution to all problems. No man - no problem.\"", "\"History has shown there are no invincible armies.\"", "\"Quantity has a quality all its own.\"", "\"University of Bristol iz bad init\"", "\"There is no hope.\""};

char *death_message;
char *quote;

int wave = 1;
int wave_chungus_left = 0;
int money = 0;
bool running = true;
bool display_fps = false;

void spawn_chungi(int num) {
    chungi_length = num;
    free(chungi);
    chungi = malloc(chungi_length * sizeof(Chungus));
    for (int i=0; i<chungi_length; i++) {
        chungi[i] = (Chungus) {WINDOW_WIDTH + i*CHUNGUS_SPEED*1/wave, GetRandomValue(0, WINDOW_HEIGHT - 150), ChungusSize.width, ChungusSize.height, true};
    }
}

void spawn_turret(Vector2 position) {
    turrets_length++;
    Turret *new_turrets = malloc(turrets_length*sizeof(Turret));
    for (int i=0; i<(turrets_length-1); i++) {
        new_turrets[i] = turrets[i];
    }
    free(turrets);
    turrets = new_turrets;
    turrets[turrets_length - 1] = (Turret) {position, 0};
}

Chungus* find_nearest_chungus(Vector2 position) {
    Chungus* nearest_chungus = NULL;
    float chungus_distance = 10000.0f;
    for (int i=0; i<chungi_length; i++) {
        if (chungi[i].active && chungi[i].rectange.x < WINDOW_WIDTH) {
            float distance = Vector2Distance(position, (Vector2){chungi[i].rectange.x, chungi[i].rectange.y});
            if (distance < chungus_distance) {
                chungus_distance = distance;
                nearest_chungus = &chungi[i];
            }
        }
    }
    return nearest_chungus;
}

int get_turret_cost(void) {
    return 20 * (turrets_length + 1);
}

int main() {

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chungus Game - Wave 1");
    InitAudioDevice();

    Texture2D chungus_texture = LoadTexture("res/big.png");
    Sound hit_sound = LoadSound("res/eep.wav");
    Sound laser_sound = LoadSound("res/laser.wav");
    Music soviet_anthem = LoadMusicStream("res/soviet-anthem.mp3");

    PlayMusicStream(soviet_anthem);

    char *death_message = death_messages[GetRandomValue(0, NUM_OF_DEATH_MESSAGES-1)];
    char *quote = quotes[GetRandomValue(0, NUM_OF_QUOTES-1)];
    chungi = malloc(0);
    turrets = malloc(0);

    // Spawn 5 chungi to start
    spawn_chungi(5);

    char *wave_message;
    char *money_message;
    char *turret_cost_message;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        float deltaTime = GetFrameTime();
        UpdateMusicStream(soviet_anthem);

        if (IsKeyPressed(KEY_F3)) {
            display_fps = !display_fps;
        }

        if (running) {
            // Check if left mouse button clicked, if so, check if chungus hit
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePosition = GetMousePosition();
                if (mousePosition.x >= 0 && mousePosition.x < 1000 && mousePosition.y >= 0 && mousePosition.y < 1000) {
                    for (int i=0; i<chungi_length; i++) {
                        if (CheckCollisionPointRec(mousePosition, chungi[i].rectange) && chungi[i].active) {
                            chungi[i].active = false;
                            money++;
                            PlaySound(hit_sound);
                            break;
                        }
                    }
                }
            }

            // Check if right mouse button clicked, if so, spawn turret
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                Vector2 mousePosition = GetMousePosition();
                if (mousePosition.x >= 0 && mousePosition.x < 1000 && mousePosition.y >= 0 && mousePosition.y < 1000) {
                    if (money >= get_turret_cost()) {
                        money -= get_turret_cost();
                        spawn_turret(mousePosition);
                    }
                }
            }
            
            // Loop through chungi, move and check if lost
            for (int i=0; i<chungi_length; i++) {
                if (!chungi[i].active) {continue;}
                chungi[i].rectange.x = chungi[i].rectange.x - CHUNGUS_SPEED*deltaTime;
                if (chungi[i].rectange.x < -chungi[i].rectange.width) {
                    running = false;
                }
            }

            // Loop through and try to shoot all turrets
            for (int i=0; i<turrets_length; i++) {
                if (turrets[i].cooldown > 0) {
                    turrets[i].cooldown -= deltaTime;
                } else {
                    Chungus *nearest_chungus = find_nearest_chungus(turrets[i].position);
                    if (nearest_chungus) {
                        turrets[i].target = (Vector2){nearest_chungus->rectange.x + nearest_chungus->rectange.width/2, nearest_chungus->rectange.y + nearest_chungus->rectange.height/2};
                        turrets[i].cooldown = TURRET_COOLDOWN;
                        nearest_chungus->active = false;
                        PlaySound(laser_sound);
                    }
                }
            }

            // Check if wave is passed
            bool passed = true;
            for (int i=0; i<chungi_length; i++) {
                if (chungi[i].active) {passed = false;}
            }
            if (passed) {
                wave++;
                spawn_chungi(5*wave);
                char *new_window_title = TextFormat("Chungus Game - Wave %d", wave);
                SetWindowTitle(new_window_title);
            }
        }

        BeginDrawing();

            ClearBackground((Color){195, 1, 0, 255});

            // Draw messages
            if (running) {
                wave_message = TextFormat("Wave %d", wave);
                money_message = TextFormat("Money %d", money);
                turret_cost_message = TextFormat("Turret Cost %d", get_turret_cost());
                DrawText(wave_message, WINDOW_WIDTH/2 - MeasureText(wave_message, FONT_SIZE)/2, WINDOW_HEIGHT/2 - FONT_SIZE*2, FONT_SIZE, (Color){251, 214, 0, 255});
                DrawText(money_message, WINDOW_WIDTH/2 - MeasureText(money_message, FONT_SIZE)/2, WINDOW_HEIGHT/2 - FONT_SIZE, FONT_SIZE, (Color){251, 214, 0, 255});
                DrawText(turret_cost_message, WINDOW_WIDTH/2 - MeasureText(turret_cost_message, FONT_SIZE)/2, WINDOW_HEIGHT/2, FONT_SIZE, (Color){251, 214, 0, 255});
                DrawText(quote, WINDOW_WIDTH/2 - MeasureText(quote, FONT_SIZE)/2, WINDOW_HEIGHT/2 + FONT_SIZE, FONT_SIZE, (Color){251, 214, 0, 255});
            } else {
                DrawText(death_message, WINDOW_WIDTH/2 - MeasureText(death_message, FONT_SIZE)/2, WINDOW_HEIGHT/2 - FONT_SIZE/2, FONT_SIZE, (Color){251, 214, 0, 255});
            }

            // Draw turrets
            for (int i=0; i<turrets_length; i++) {
                DrawCircleV(turrets[i].position, 15, BLACK);
                if (turrets[i].cooldown > TURRET_COOLDOWN - 0.2) {
                    DrawLineV(turrets[i].position, turrets[i].target, (Color){50, 219, 81, 255});
                }
            }

            // Draw chungi
            for (int i=0; i<chungi_length; i++) {
                if (!chungi[i].active) {continue;}
                DrawTexturePro(chungus_texture, (Rectangle){0, 0, ChungusSize.width, ChungusSize.height}, chungi[i].rectange, (Vector2){0, 0}, 0, WHITE);
            }

            if (display_fps) {
                DrawFPS(0, 0);
            }

        EndDrawing();

    }

    UnloadMusicStream(soviet_anthem);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}