#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

int playing = 0;
int derrota = 0;
float timer = 0;
float timer_derrota = 0;

typedef enum
{
    GameOver = 0,
    GamePaused = 1,
    GamePlaying = 2,
    GameWinned = 3
} GameState;

int **criar_tabuleiro_informacao(int rows, int cols)
{
    int **tabuleiro = (int **)calloc(rows, sizeof(int *));

    for (int i = 0; i < rows; i++)
    {
        tabuleiro[i] = (int *)calloc(cols, sizeof(int));
    }

    return tabuleiro;
}

int marca_vazio(int **table, int lin_selected, int col_selected, int rows_tab, int cols_tab)
{
    if (lin_selected < 0 || lin_selected >= rows_tab || col_selected < 0 || col_selected >= cols_tab)
        return 0;

    if (table[lin_selected][col_selected] == 0)
    {
        table[lin_selected][col_selected] = 10; // marca como revelado

        if (col_selected != cols_tab - 1)
        {
            if (table[lin_selected][col_selected + 1] != 10)
                marca_vazio(table, lin_selected, col_selected + 1, rows_tab, cols_tab); // direita

            if (lin_selected != rows_tab - 1)
                marca_vazio(table, lin_selected + 1, col_selected + 1, rows_tab, cols_tab); // diagonal direita inferior
        }

        if (col_selected != 0)
        {
            if (table[lin_selected][col_selected - 1] != 10)
                marca_vazio(table, lin_selected, col_selected - 1, rows_tab, cols_tab); // esquerda

            if (lin_selected != 0)
                marca_vazio(table, lin_selected - 1, col_selected - 1, rows_tab, cols_tab); // diagonal esquerda superior
        }

        if (lin_selected != rows_tab - 1)
        {
            if (table[lin_selected + 1][col_selected] != 10)
                marca_vazio(table, lin_selected + 1, col_selected, rows_tab, cols_tab); // baixo

            if (col_selected != 0)
                marca_vazio(table, lin_selected + 1, col_selected - 1, rows_tab, cols_tab); // diagonal esquerda inferior
        }

        if (lin_selected != 0)
        {
            if (table[lin_selected - 1][col_selected] != 10)
                marca_vazio(table, lin_selected - 1, col_selected, rows_tab, cols_tab); // cima
            if (col_selected != cols_tab - 1)
                marca_vazio(table, lin_selected - 1, col_selected + 1, rows_tab, cols_tab); // diagonal direita superior
        }
    }
    else if (!(table[lin_selected][col_selected] > 0) && !(table[lin_selected][col_selected] < -10))
        table[lin_selected][col_selected] *= -1;

    return 0;
}

int gerar_bomba(int **tab, int difficulty, int rows_tab, int cols_tab, int lin_click, int col_click)
{
    int quant_bombas = 10, rand_lin, rand_col, i = 0;
    double distancia;

    switch (difficulty)
    {
    case 1:
        quant_bombas = 20;
        break;
    case 2:
        quant_bombas = 40;
        break;
    case 3:
        quant_bombas = 99;
        break;

    default:
        quant_bombas = 10;
        break;
    }

    while (i < quant_bombas)
    {
        rand_lin = rand() % rows_tab;
        rand_col = rand() % cols_tab;
        distancia = sqrt(pow((rand_col - col_click), 2) + pow((rand_lin - lin_click), 2));

        if (distancia >= 4 && tab[rand_lin][rand_col] != -9)
        {
            tab[rand_lin][rand_col] = -9;
            i++;
        }
    }
    return quant_bombas;
}

void marcar_bomba(int **tab, int rows, int cols)
{
    int quant_bombas = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (tab[i][j] == -9)
            {
                quant_bombas++;
                for (int y = i - (i == 0 ? 0 : 1); y <= i + (i == rows - 1 ? 0 : 1); y++) // começa na linha anterior, uma casa acima da bomba
                {
                    for (int x = j - (j == 0 ? 0 : 1); x <= j + (j == cols - 1 ? 0 : 1); x++) // começa na coluna anterior, uma casa acima da bomba
                    {
                        if (tab[y][x] != -9)
                        {
                            tab[y][x]--; // vai marcar mais uma bomba
                        }
                    }
                }
            }
        }
    }
}

void revelar_num(int **tab, int rows_tab, int cols_tab, int lin_selected, int col_selected)
{
    if (lin_selected < 0 || lin_selected >= rows_tab || col_selected < 0 || col_selected >= cols_tab)
        return;

    if (tab[lin_selected][col_selected] > 0)
        return;

    if (tab[lin_selected][col_selected] < -9) // Estiver com uma bandeira
        tab[lin_selected][col_selected] += 10;

    if (tab[lin_selected][col_selected] == 0)
        marca_vazio(tab, lin_selected, col_selected, rows_tab, cols_tab);
    else
        tab[lin_selected][col_selected] *= -1;
}

int marca_bandeira(int **tab, int rows_tab, int cols_tab, int lin_selected, int col_selected)
{
    if (lin_selected < 0 || lin_selected >= rows_tab || col_selected < 0 || col_selected >= cols_tab)
        return 0;

    if (tab[lin_selected][col_selected] > 0)
        return 0;

    // if (tab[lin_selected][col_selected] == -9)
    //     return 1;
    // else if (tab[lin_selected][col_selected] == -19)
    //     return -1;

    if (tab[lin_selected][col_selected] >= -9)
        tab[lin_selected][col_selected] -= 10; // Coloca uma bandeira no campo
    else
        tab[lin_selected][col_selected] += 10; // Tira uma bandeira no campo
}

Rectangle **criar_tabuleiro_visualizacao(int rows, int cols)
{
    Rectangle **tabuleiro = (Rectangle **)calloc(rows, sizeof(Rectangle *));

    for (int i = 0; i < rows; i++)
    {
        tabuleiro[i] = (Rectangle *)calloc(cols, sizeof(Rectangle));
    }

    return tabuleiro;
}

int isFirstPlay()
{
    return !playing;
}

int iniciar_partida(int **tab, int difficulty, int rows_tab, int cols_tab, int lins_bomb, int col_bomb)
{
    int quantidade_bandeira = gerar_bomba(tab, difficulty, rows_tab, cols_tab, lins_bomb, col_bomb);
    marcar_bomba(tab, rows_tab, cols_tab);

    return quantidade_bandeira;
}

void DrawMenu(int bandeiras, int screenWidth)
{
    DrawRectangle(0, 0, screenWidth, 100, BLACK);
    DrawLineV((Vector2){0, 100}, (Vector2){(float)screenWidth, 100}, LIGHTGRAY);
    const char *texto = TextFormat("%d Bandeiras", bandeiras);
    int font_size = 25;
    DrawText(texto, 10, 10, 30, WHITE);

    const char *texto2 = TextFormat("%00.0fs", timer);
    font_size = 10;
    DrawText(texto2, 30, 35, 30, WHITE);
}

GameState reset_game(int **tab, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            tab[i][j] = 0;
        }
    }
    playing = 0;
    derrota = 0;
    timer = 0;
    return GamePlaying;
}

int verificar_vitoria(int **tab, int rows, int cols, int quant_bombas)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (tab[i][j] < 0 && tab[i][j] != -9 && tab[i][j] != -19)
                return 0;
        }
    }

    return 1;
}

void animacao_derrota(int **tab_info, int tab_row, int tab_col)
{
    int i, j;
    for (i = 0; i < tab_row; i++)
    {
        for (j = 0; j < tab_col; j++)
        {
            if (tab_info[i][j] == -9 || tab_info[i][j] == -19 || tab_info[i][j] == 9)
            {
                tab_info[i][j] = 90;
                return;
            }
        }
    }

    if (i == tab_row && j == tab_col)
        derrota = 1;
}

void desenha_tela_vitoria_derrota(int **matrix_info_game, int rows, int cols, int screenHeight, int screenWidth, float piscar, GameState *estado, char *texto1, char *texto2)
{
    DrawRectangle(0, screenHeight / 2 - screenHeight / 4, screenWidth, screenHeight / 2, ColorAlpha(WHITE, 0.7f));
    int fontsize = 65;
    int textwidth = MeasureText(texto1, fontsize);

    DrawText(texto1, screenWidth / 2 - textwidth / 2, screenHeight / 2 - fontsize / 2 - 20, fontsize, RED);

    int fontsize2 = 25;
    int textwidth2 = MeasureText(texto1, fontsize);

    if ((int)(piscar * 10) % 10)
    {
        DrawText(texto2, screenWidth / 2 - textwidth2 / 2 + 20, screenHeight / 2 + 100, fontsize2, BLACK);
    }

    if (IsKeyPressed(KEY_ENTER))
        *estado = reset_game(matrix_info_game, rows, cols);
}

Color define_cor(int num)
{
    switch (num)
    {
    case 1:
        return (Color){2, 0, 255, 255};
        break;
    case 2:
        return (Color){0, 255, 100, 255};
        break;
    case 3:
        return (Color){255, 50, 0, 255};
        break;
    case 4:
        return (Color){25, 0, 150, 255};
        break;
    case 5:
        return (Color){131, 1, 0, 255};
        break;
    case 6:
        return (Color){3, 130, 132, 255};
        break;
    case 7:
        return (Color){133, 0, 131, 255};
        break;
    case 8:
        return (Color){150, 150, 150, 255};
        break;
    default:
        return (Color){255, 100, 255, 255};
        break;
    }
}

int main()
{
    srand(time(NULL));

    GameState *estado = malloc(sizeof(GameState));
    *estado = GamePlaying;

    int screenWidth = 600;
    int screenHeight = 650;
    int cell_size = 50;
    int dificuldade = 1;
    int bandeiras = 0;
    int quant_bombas = 0;
    float piscar = 0;

    int cols = screenWidth / cell_size;
    int rows = screenHeight / cell_size - 2; // -2 para o espaço do menu

    InitWindow(screenWidth, screenHeight, "Campo Minado");
    if (IsWindowState(FLAG_WINDOW_RESIZABLE))
        ClearWindowState(FLAG_WINDOW_RESIZABLE);

    Texture2D flag = LoadTexture("assets/flag.png");
    Texture2D tile = LoadTexture("assets/tile.png");

    Rectangle **matrix_view_game = criar_tabuleiro_visualizacao(rows, cols);
    int **matrix_info_game = criar_tabuleiro_informacao(rows, cols);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            matrix_view_game[i][j].height = cell_size;
            matrix_view_game[i][j].width = cell_size;
            matrix_view_game[i][j].x = cell_size * j;
            matrix_view_game[i][j].y = cell_size * i + (cell_size * 2); // + cell_size * 2 para o espaço do menu
        }
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);
        Vector2 mousePoint = GetMousePosition();

        // Desenha o tabuleiro
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (matrix_info_game[i][j] <= 0)
                    DrawTextureRec(tile, matrix_view_game[i][j], (Vector2){matrix_view_game[i][j].x, matrix_view_game[i][j].y}, WHITE);
                else
                    DrawRectangleRec(matrix_view_game[i][j], ((*estado == GameOver && (matrix_info_game[i][j] == 90)) ? RED : GRAY));

                if (*estado == GamePlaying)
                    printf("%d ", matrix_info_game[i][j]);
            }
            if (*estado == GamePlaying)
                printf("\n");
        }
        if (*estado == GamePlaying)
            printf("\n\n");

       // Desenha as linhas do tabuleiro
        for (int i = 0; i <= cols; i++)
        {
            DrawLineV((Vector2){i * cell_size, 0}, (Vector2){i * cell_size, (float)screenHeight}, LIGHTGRAY);
        }

        for (int i = 2; i <= rows + 2; i++)
        {
            DrawLineV((Vector2){0, i * cell_size}, (Vector2){(float)screenWidth, i * cell_size}, LIGHTGRAY);
        }

        // Desenha a barra de menu superior
        DrawMenu(bandeiras, screenWidth);

        // Escreve os valores se celula revelada

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (matrix_info_game[i][j] <= 0 && matrix_info_game[i][j] >= -9)
                    continue;

                const char *text = TextFormat("%c", matrix_info_game[i][j] < -9 ? 'B' : (matrix_info_game[i][j] == 10 || matrix_info_game[i][j] == 9) ? ' '
                                                                                                                                                      : matrix_info_game[i][j] + '0');

                int font_size = 30;
                int text_size = MeasureText(text, font_size);
                int x = matrix_view_game[i][j].x + (cell_size / 2) - (text_size / 2);
                int y = matrix_view_game[i][j].y + (cell_size / 2) - (font_size / 2);

                if (matrix_info_game[i][j] < -9)
                    DrawTexture(flag, j * cell_size, i * cell_size + 100, WHITE);
                else
                    DrawText(text, x, y, font_size, define_cor(matrix_info_game[i][j]));
            }
        }

        if (*estado == GamePlaying)
        {
            if (!isFirstPlay())
                timer += GetFrameTime();
            // Verifica se clicou em alguma célula
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    if (CheckCollisionPointRec(mousePoint, matrix_view_game[i][j]))
                    {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && matrix_info_game[i][j] >= -9)
                        {
                            if (isFirstPlay())
                            {
                                bandeiras = iniciar_partida(matrix_info_game, dificuldade, rows, cols, i, j);
                                quant_bombas = bandeiras;
                                playing = 1;
                            }

                            if (matrix_info_game[i][j] < -9)
                                bandeiras++;

                            revelar_num(matrix_info_game, rows, cols, i, j);

                            if (matrix_info_game[i][j] == 9)
                            {
                                DrawRectangleRec(matrix_view_game[i][j], RED);
                                *estado = GameOver;
                            }

                            if (verificar_vitoria(matrix_info_game, rows, cols, quant_bombas))
                            {
                                printf("GANHOU PAI!");
                                *estado = GameWinned;
                                bandeiras = quant_bombas;
                            }
                        }
                        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                        {
                            marca_bandeira(matrix_info_game, rows, cols, i, j);
                            if (matrix_info_game[i][j] >= -9 && matrix_info_game[i][j] < 0)
                                bandeiras++;
                            else if (matrix_info_game[i][j] <= -9 && matrix_info_game[i][j] < -9)
                                bandeiras--;
                        }
                    }
                }
            }
        }

        if (*estado == GamePaused)
        {
        }

        if (*estado == GameOver)
        {
            if (!derrota)
            {
                if (timer_derrota > 0.3)
                {
                    animacao_derrota(matrix_info_game, rows, cols);
                    timer_derrota = 0;
                }
                timer_derrota += GetFrameTime();
            }
            else
                desenha_tela_vitoria_derrota(matrix_info_game, rows, cols, screenHeight, screenWidth, piscar, estado, "Você Perdeu!", "Pressione Enter para reiniciar!");

            piscar += GetFrameTime();
        }

        if (*estado == GameWinned)
        {
            DrawRectangle(0, screenHeight / 2 - screenHeight / 4, screenWidth, screenHeight / 2, ColorAlpha(WHITE, 0.7f));
            char *texto = "Você Ganhou!";
            int fontsize = 65;
            int textwidth = MeasureText(texto, fontsize);

            DrawText(texto, screenWidth / 2 - textwidth / 2, screenHeight / 2 - fontsize / 2 - 20, fontsize, GREEN);

            char *texto2 = "Pressione Enter para reiniciar!";
            int fontsize2 = 25;
            int textwidth2 = MeasureText(texto, fontsize);

            if ((int)(piscar * 10) % 10)
            {
                DrawText(texto2, screenWidth / 2 - textwidth2 / 2 + 20, screenHeight / 2 + 100, fontsize2, BLACK);
            }

            piscar += GetFrameTime();

            if (IsKeyPressed(KEY_ENTER))
                *estado = reset_game(matrix_info_game, rows, cols);
        }

        EndDrawing();
    }

    CloseWindow();
    UnloadTexture(flag);
    UnloadTexture(tile);

    for (int i = 0; i < rows; i++)
    {
        free(matrix_info_game[i]);
    }

    for (int i = 0; i < rows; i++)
    {
        free(matrix_view_game[i]);
    }

    free(*matrix_info_game);
    free(*matrix_view_game);
    free(estado);

    return 0;
}
