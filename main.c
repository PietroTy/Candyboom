#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define GRID_WIDTH 10      // Largura da grade
#define GRID_HEIGHT 10     // Altura da grade
#define CELL_SIZE 50      // Tamanho das células
#define NUM_CANDY_TYPES 5 // Tipos de doces
#define SELECTED_SIZE 40  // Tamanho da peça selecionada
#define FALL_SPEED 0.5f   // Velocidade de queda

typedef struct {
    int type;
    bool isMatched;
    bool isFallingOrSelected; // Indica se a peça está caindo ou foi selecionada
} Candy;

Candy grid[GRID_HEIGHT][GRID_WIDTH]; // Grade do jogo
int score = 0;
bool isDropping = false;
float dropTimer = 0.0f; // Timer para controlar o tempo de queda

// Protótipos das funções
void InitializeGrid();
void DrawGameGrid(int selectedX, int selectedY);
bool CheckMatches();
void ResolveMatches();
void SwapCandies(int x1, int y1, int x2, int y2);
bool IsValidSwap(int x1, int y1, int x2, int y2);
void DropCandies(float deltaTime);
void FillGrid();
void GenerateNewCandies();

int main() {
    InitWindow(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE + 50, "Candyboom");
    SetTargetFPS(60);
    srand(time(NULL));

    InitializeGrid();

    int selectedX = -1, selectedY = -1;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        if (!isDropping && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            int gridX = mousePos.x / CELL_SIZE;
            int gridY = mousePos.y / CELL_SIZE;

            if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
                if (selectedX == -1 && selectedY == -1) {
                    selectedX = gridX;
                    selectedY = gridY;
                } else {
                    if (IsValidSwap(selectedX, selectedY, gridX, gridY)) {
                        SwapCandies(selectedX, selectedY, gridX, gridY);
                        if (!CheckMatches()) {
                            SwapCandies(selectedX, selectedY, gridX, gridY);
                        } else {
                            ResolveMatches();
                        }
                    }
                    selectedX = -1;
                    selectedY = -1;
                }
            }
        }

        if (CheckMatches()) {
            ResolveMatches();
        }

        // Update the fall progress of candies
        dropTimer += deltaTime;
        if (dropTimer >= FALL_SPEED) {
            DropCandies(deltaTime);
            dropTimer = 0.0f;
        }

        DrawGameGrid(selectedX, selectedY);

        DrawText(TextFormat("Score: %d", score), 10, GRID_HEIGHT * CELL_SIZE + 10, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void InitializeGrid() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x].type = rand() % NUM_CANDY_TYPES;
            grid[y][x].isMatched = false;
            grid[y][x].isFallingOrSelected = false; // Inicialmente, nenhuma peça está caindo ou selecionada
        }
    }
}

void DrawGameGrid(int selectedX, int selectedY) {
    Color candyColorsOut[NUM_CANDY_TYPES] = {DARKRED, DARKGREEN, DARKBLUE, DARKYELLOW, DARKPURPLE};
    Color candyColorsIn[NUM_CANDY_TYPES] = {RED, GREEN, BLUE, YELLOW, PURPLE};

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int type = grid[y][x].type;

            // Verificar se a célula é a selecionada ou está caindo
            bool isSelectedOrFalling = (x == selectedX && y == selectedY) || grid[y][x].isFallingOrSelected;

            // Definir a largura e altura da célula (se está selecionada ou caindo)
            float width = isSelectedOrFalling ? SELECTED_SIZE : CELL_SIZE;
            float height = isSelectedOrFalling ? SELECTED_SIZE : CELL_SIZE;

            // Desenhar a célula
            if (isSelectedOrFalling) {
                // Para células selecionadas ou caindo, desenha com bordas
                DrawRectangle(x * CELL_SIZE + 5, y * CELL_SIZE + 5, width, height, candyColorsOut[type]);
                DrawRectangle(x * CELL_SIZE + 10, y * CELL_SIZE + 10, width - 10, height - 10, candyColorsIn[type]);
            } else {
                // Para células normais
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, width, height, candyColorsOut[type]);
                DrawRectangle(x * CELL_SIZE + 5, y * CELL_SIZE + 5, width - 10, height - 10, candyColorsIn[type]);
            }
        }
    }
}


bool CheckMatches() {
    bool foundMatch = false;

    // Verificar matches horizontais
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH - 2; x++) {
            int type = grid[y][x].type;
            if (type != -1 &&
                type == grid[y][x + 1].type &&
                type == grid[y][x + 2].type) {
                grid[y][x].isMatched = true;
                grid[y][x + 1].isMatched = true;
                grid[y][x + 2].isMatched = true;
                foundMatch = true;
            }
        }
    }

    // Verificar matches verticais
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT - 2; y++) {
            int type = grid[y][x].type;
            if (type != -1 &&
                type == grid[y + 1][x].type &&
                type == grid[y + 2][x].type) {
                grid[y][x].isMatched = true;
                grid[y + 1][x].isMatched = true;
                grid[y + 2][x].isMatched = true;
                foundMatch = true;
            }
        }
    }

    return foundMatch;
}

void ResolveMatches() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x].isMatched) {
                grid[y][x].type = -1; // Deixa a célula vazia
                grid[y][x].isMatched = false;
                score += 10;
            }
        }
    }
}

void SwapCandies(int x1, int y1, int x2, int y2) {
    Candy temp = grid[y1][x1];
    grid[y1][x1] = grid[y2][x2];
    grid[y2][x2] = temp;
}

bool IsValidSwap(int x1, int y1, int x2, int y2) {
    return (abs(x1 - x2) + abs(y1 - y2)) == 1;
}

void DropCandies(float deltaTime) {
    bool isDropped = false;
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
            if (grid[y][x].type == -1) {
                for (int k = y - 1; k >= 0; k--) {
                    if (grid[k][x].type != -1) {
                        // Move a candy down
                        grid[y][x].type = grid[k][x].type;
                        grid[k][x].type = -1;
                        grid[y][x].isFallingOrSelected = true; // Marca como caindo ou selecionada
                        isDropped = true;
                        break;
                    }
                }
            } else if (grid[y][x].isFallingOrSelected) {
                // Quando a peça atingir o fundo, pare de cair
                grid[y][x].isFallingOrSelected = false;
            }
        }
    }

    // Set dropping state based on whether any candy dropped
    isDropping = isDropped;

    // After the candies have settled, fill in new candies
    if (!isDropping) {
        GenerateNewCandies();
    }
}

void GenerateNewCandies() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x].type == -1) {
                grid[y][x].type = rand() % NUM_CANDY_TYPES;
                grid[y][x].isFallingOrSelected = true; // Marca como caindo ou selecionada
            }
        }
    }
}
