#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

float playerSpeed = 300;

int main() {

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chungus Game");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        float deltaTime = GetFrameTime();

        BeginDrawing();

            ClearBackground((Color){195, 1, 0, 255});

            DrawFPS(0, 0);

        EndDrawing();

    }

    return 0;
}