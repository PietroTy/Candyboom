#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define GRID_WIDTH 10      // Largura da grade
#define GRID_HEIGHT 10     // Altura da grade
#define CELL_SIZE 50      // Tamanho das células
#define NUM_CANDY_TYPES 5 // Tipos de doces
#define SELECTED_SIZE 40  // Tamanho da peça selecionada
#define FALL_SPEED 0.1f   // Velocidade de queda
#define EXPLOSION_RADIUS 2 // Raio da explosão 5x5


typedef struct {
    int type;
    bool isMatched;
    bool isFallingOrSelected;
    float fallingY; // Posição animada
} Candy;


Candy grid[GRID_HEIGHT][GRID_WIDTH]; // Grade do jogo
int score = 0;
int highscore = 0;
bool isDropping = false;
float dropTimer = 0.0f; // Timer para controlar o tempo de queda

int comboCount = 0;  // Rastreia o número de combos consecutivos
int baseScore = 1;  // Pontuação base para cada doce eliminado


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

// Função para salvar o *highscore* em um arquivo
void SaveHighscore(int highscore) {
    FILE *file = fopen("resources/CandyHighscore.txt", "w");
    if (file != NULL) {
        fprintf(file, "%d\n", highscore);
        fclose(file);
        printf("Highscore salvo: %d\n", highscore);
    } else {
        printf("Erro ao salvar o highscore.\n");
    }
}

// Função para carregar o *highscore* do arquivo
int LoadHighscore() {
    FILE *file = fopen("resources/CandyHighscore.txt", "r");
    int highscore = 0;

    if (file != NULL) {
        fscanf(file, "%d", &highscore);
        fclose(file);
        printf("Highscore carregado: %d\n", highscore);
    } else {
        printf("Nenhum highscore salvo encontrado.\n");
    }

    return highscore;
}


int main() {
    InitWindow(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE + 40, "Candyboom");
    SetWindowIcon(LoadImage("resources/iconeCandy.png"));
    SetTargetFPS(60);
    srand(time(NULL));

    InitializeGrid();
    InitAudioDevice();

    Sound pop = LoadSound("resources/Pop.wav");
    highscore = LoadHighscore();

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
                            PlaySound(pop);

                            // Jogada manual - reseta o combo
                            comboCount = 0;
                        }
                    }
                    selectedX = -1;
                    selectedY = -1;
                }
            }
        }

        // Matches automáticos - combos consecutivos
        if (CheckMatches()) {
            ResolveMatches();
            PlaySound(pop);
        } else if (!isDropping) {
            // Reseta o combo quando não houver mais matches automáticos
            comboCount = 0;
        }

        dropTimer += deltaTime;
        if (dropTimer >= FALL_SPEED) {
            DropCandies(deltaTime);
            dropTimer = 0.0f;
        }

        DrawGameGrid(selectedX, selectedY);

        // Mostra a pontuação e o combo
        DrawText(TextFormat("Score: %d", score), 10, GRID_HEIGHT * CELL_SIZE + 10, 20, WHITE);
        DrawText(TextFormat("Combo: x%d", comboCount + 1), 200, GRID_HEIGHT * CELL_SIZE + 10, 20, WHITE);
        DrawText(TextFormat("High: %d", highscore), (GetScreenWidth() - MeasureText(TextFormat("High: %d", highscore), 20)) - 10, GRID_HEIGHT * CELL_SIZE + 10, 20, WHITE);
        DrawText(TextFormat("©PietroTy 2024"), 10, 10, 20, WHITE);


        EndDrawing();
    }



    CloseAudioDevice();
    CloseWindow();
    return 0;
}



void InitializeGrid() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x].type = rand() % NUM_CANDY_TYPES;
            grid[y][x].isMatched = false;
            grid[y][x].isFallingOrSelected = false;
            grid[y][x].fallingY = y * CELL_SIZE; // Posição inicial
        }
    }
}


void DrawGameGrid(int selectedX, int selectedY) {
    Color candyColorsOut[NUM_CANDY_TYPES] = {DARKRED, DARKGREEN, DARKBLUE, DARKYELLOW, DARKPURPLE};
    Color candyColorsIn[NUM_CANDY_TYPES] = {RED, GREEN, BLUE, YELLOW, PURPLE};

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int type = grid[y][x].type;

            if (type == -1) continue;

            // Verificar se a célula é a selecionada ou está caindo
            bool isSelectedOrFalling = (x == selectedX && y == selectedY) || grid[y][x].isFallingOrSelected;

            // Definir a largura e altura da célula (se está selecionada ou caindo)
            float width = isSelectedOrFalling ? SELECTED_SIZE : CELL_SIZE;
            float height = isSelectedOrFalling ? SELECTED_SIZE : CELL_SIZE;

            float drawY = grid[y][x].fallingY;

            // Desenhar a célula (ajuste para animação)
            if (isSelectedOrFalling) { 
                DrawRectangle(x * CELL_SIZE + (CELL_SIZE - SELECTED_SIZE) / 2, drawY + (CELL_SIZE - SELECTED_SIZE) / 2, width, height, candyColorsOut[type]);
                DrawRectangle(x * CELL_SIZE + (CELL_SIZE - SELECTED_SIZE) / 2 + 5, drawY + (CELL_SIZE - SELECTED_SIZE) / 2 + 5, width - 10, height - 10, candyColorsIn[type]);
            } else {
                DrawRectangle(x * CELL_SIZE, drawY, CELL_SIZE, CELL_SIZE, candyColorsOut[type]);
                DrawRectangle(x * CELL_SIZE + 5, drawY + 5, CELL_SIZE - 10, CELL_SIZE - 10, candyColorsIn[type]);
            }

        }
    }
}

void TriggerExplosion(int centerX, int centerY) {
    for (int y = centerY - EXPLOSION_RADIUS; y <= centerY + EXPLOSION_RADIUS; y++) {
        for (int x = centerX - EXPLOSION_RADIUS; x <= centerX + EXPLOSION_RADIUS; x++) {
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                if (grid[y][x].type != -1) {
                    grid[y][x].type = -1;
                    score += 25 * (comboCount + 1); // Pontuação adicional
                }
            }
        }
    }
    int startX = centerX - 2; // Posição inicial da explosão (5x5)
    int startY = centerY - 2;
    int endX = centerX + 2;
    int endY = centerY + 2;

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, DARKORANGE);
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
            int matchLength = 1;

            if (type != -1) {
                for (int k = 1; x + k < GRID_WIDTH && grid[y][x + k].type == type; k++) {
                    matchLength++;
                }

                if (matchLength >= 3) {
                    for (int k = 0; k < matchLength; k++) {
                        grid[y][x + k].isMatched = true;
                    }
                    foundMatch = true;

                    // Explosão para matches grandes
                    if (matchLength >= 5) {
                        TriggerExplosion(x + matchLength / 2, y);
                    }
                }

                x += matchLength - 1; // Pula as peças já verificadas
            }
        }
    }

    // Verificar matches verticais
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT - 2; y++) {
            int type = grid[y][x].type;
            int matchLength = 1;

            if (type != -1) {
                for (int k = 1; y + k < GRID_HEIGHT && grid[y + k][x].type == type; k++) {
                    matchLength++;
                }

                if (matchLength >= 3) {
                    for (int k = 0; k < matchLength; k++) {
                        grid[y + k][x].isMatched = true;
                    }
                    foundMatch = true;

                    // Explosão para matches grandes
                    if (matchLength >= 5) {
                        TriggerExplosion(x, y + matchLength / 2);
                    }
                }

                y += matchLength - 1; // Pula as peças já verificadas
            }
        }
    }

    return foundMatch;
}

void ResolveMatches() {
    bool matchResolved = false;

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x].isMatched) {
                grid[y][x].type = -1; // Deixa a célula vazia
                grid[y][x].isMatched = false;

                // Aumenta a pontuação com base no combo atual
                score += baseScore * (comboCount + 1);
                matchResolved = true;
            }
        }
    }

    // Se ao menos uma combinação foi resolvida, aumente o comboCount
    if (matchResolved) {
        comboCount++;
    }

    if (score > highscore) {
        highscore = score;
        SaveHighscore(highscore);
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
                        // Transferir a peça
                        grid[y][x].type = grid[k][x].type;
                        grid[y][x].fallingY = grid[k][x].fallingY;
                        grid[y][x].isFallingOrSelected = true;

                        grid[k][x].type = -1;
                        grid[k][x].fallingY = k * CELL_SIZE; // Resetar a posição
                        isDropped = true;
                        break;
                    }
                }
            }

            // Se a peça estiver animando, atualizar posição em incrementos de CELL_SIZE
            if (grid[y][x].fallingY < y * CELL_SIZE) {
                grid[y][x].fallingY += CELL_SIZE; // Incrementar de forma discreta
                if (grid[y][x].fallingY >= y * CELL_SIZE) {
                    grid[y][x].fallingY = y * CELL_SIZE; // Corrigir posição
                    grid[y][x].isFallingOrSelected = false;
                }
                isDropped = true;
            }
        }
    }

    if (!isDropped) {
        // Após completar a queda, garantir que todas as peças paradas não estejam marcadas como caindo
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                grid[y][x].isFallingOrSelected = false;
            }
        }

        GenerateNewCandies();
    }

    isDropping = isDropped;
}



void GenerateNewCandies() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x].type == -1) {
                grid[y][x].type = rand() % NUM_CANDY_TYPES;
                grid[y][x].fallingY = -CELL_SIZE; // Inicia fora da tela
                grid[y][x].isFallingOrSelected = true;
            }
        }
    }
}