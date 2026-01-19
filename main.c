#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "assets/icon.h"
#include "assets/flag.h"
#include "assets/tile.h"
#include "assets/mine.h"
#include "assets/tile_error.h"
#include "assets/volume_off.h"
#include "assets/volume_up.h"
#include "assets/revelar0.h"
#include "assets/revelar1.h"
#include "assets/revelar2.h"
#include "assets/revelar3.h"
#include "assets/revelar4.h"
#include "assets/tirar_bandeira.h"
#include "assets/colocar_bandeira.h"
#include "assets/mine_sound.h"


#define MAX_SOUNDS 10

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

typedef struct
{
    Sound sound[MAX_SOUNDS];
    int currentsound;
} SoundList;

typedef struct
{
    Texture2D volume_off;
    Texture2D volume_on;
    Rectangle collision;
    bool status;
} Volume;

typedef struct
{
    int screenHeight;
    int screenWidth;
    int cell_size;
    int cols;
    int rows;
    int difficulty;

} Screen;

typedef struct
{
    Rectangle rectangle;
    Color color;
    char *text;
    int font_size;
} Option;

typedef struct
{
    Option mini_menu;
    Rectangle janela;
    Option *options;
    bool menu_aberto;
} MenuDifficulty;

int **criar_tabuleiro_informacao(Screen *screen)
{
    int **tabuleiro = (int **)calloc(screen->rows, sizeof(int *));

    for (int i = 0; i < screen->rows; i++)
    {
        tabuleiro[i] = (int *)calloc(screen->cols, sizeof(int));
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

int gerar_bomba(int **tab, Screen *screen, int lin_click, int col_click)
{
    int quant_bombas = 10, rand_lin, rand_col, i = 0;
    double distancia;

    switch (screen->difficulty)
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
        rand_lin = rand() % screen->rows;
        rand_col = rand() % screen->cols;
        distancia = sqrt(pow((rand_col - col_click), 2) + pow((rand_lin - lin_click), 2));

        if (distancia >= screen->difficulty * 2 && tab[rand_lin][rand_col] != -9)
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

    if (tab[lin_selected][col_selected] >= -9)
        tab[lin_selected][col_selected] -= 10; // Coloca uma bandeira no campo
    else
        tab[lin_selected][col_selected] += 10; // Tira uma bandeira no campo
}

Rectangle **criar_tabuleiro_visualizacao(Screen *screen)
{
    Rectangle **tabuleiro = (Rectangle **)calloc(screen->rows, sizeof(Rectangle *));

    for (int i = 0; i < screen->rows; i++)
    {
        tabuleiro[i] = (Rectangle *)calloc(screen->cols, sizeof(Rectangle));
    }

    for (int i = 0; i < screen->rows; i++)
    {
        for (int j = 0; j < screen->cols; j++)
        {
            tabuleiro[i][j].height = screen->cell_size;
            tabuleiro[i][j].width = screen->cell_size;
            tabuleiro[i][j].x = screen->cell_size * j;
            tabuleiro[i][j].y = screen->cell_size * i + 100; // + 100 para o espaço do menu
        }
    }

    return tabuleiro;
}

int isFirstPlay()
{
    return !playing;
}

int iniciar_partida(int **tab, Screen *screen, int lins_bomb, int col_bomb)
{
    int quantidade_bandeira = gerar_bomba(tab, screen, lins_bomb, col_bomb);
    marcar_bomba(tab, screen->rows, screen->cols);

    return quantidade_bandeira;
}

void DrawMenu(int bandeiras, Screen *screen, Volume *volume)
{
    // Desenha o menu
    DrawRectangle(0, 0, screen->screenWidth, 100, BLACK);
    DrawLineV((Vector2){0, 100}, (Vector2){(float)screen->screenWidth, 100}, LIGHTGRAY);

    // Desenha o contador de bandeiras
    const char *texto = TextFormat("%d Bandeiras", bandeiras);
    int font_size = 25;
    DrawText(texto, 10, 10, 30, WHITE);

    // Desenha o timer
    const char *texto2 = TextFormat("%00.0fs", timer);
    font_size = 10;
    DrawText(texto2, 30, 35, 30, WHITE);

    // Desenha o botão de mute
    if (volume->status)
        DrawTexture(volume->volume_on, volume->collision.x, volume->collision.y, WHITE);
    else
        DrawTexture(volume->volume_off, volume->collision.x, volume->collision.y, WHITE);
}

GameState reset_game(int **tab, Screen *screen)
{
    for (int i = 0; i < screen->rows; i++)
    {
        for (int j = 0; j < screen->cols; j++)
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

void animacao_derrota(int **tab_info, Screen *screen, Vector2 clickmouse)
{
    int i, j, aux = 0;

    if (clickmouse.x != -1 && clickmouse.y != -1)
    {
        tab_info[(int)clickmouse.x][(int)clickmouse.y] = 94;
        return;
    }

    for (i = 0; i < screen->rows; i++)
    {
        for (j = 0; j < screen->cols; j++)
        {
            if (tab_info[i][j] == -9 || tab_info[i][j] == 9) // Marca como mina não revalada
            {

                tab_info[i][j] = 90;
                return;
            }
            else if (tab_info[i][j] < -9 && tab_info[i][j] != -19)
            { // Se a bandeira está marcando um local que não é mina
                tab_info[i][j] = 95;
            }

            if (tab_info[i][j] >= 90 && tab_info[i][j] < 94) // Itera sob os estagios da explosao
            {
                tab_info[i][j]++;
                aux = 0;
            }
            else if (tab_info[i][j] == 94)
            {
                aux = 1;
            }
        }
    }

    if (i == screen->rows && j == screen->cols && aux)
        derrota = 1;
}

void desenha_tela_vitoria_derrota(int **matrix_info_game, Screen *screen, float piscar, GameState *estado, char *texto1, char *texto2)
{
    DrawRectangle(0, screen->screenHeight / 2 - screen->screenHeight / 4, screen->screenWidth, screen->screenHeight / 2, ColorAlpha(WHITE, 0.7f));
    int fontsize = 65;
    int textwidth = MeasureText(texto1, fontsize);

    DrawText(texto1, screen->screenWidth / 2 - textwidth / 2, screen->screenHeight / 2 - fontsize / 2 - 20, fontsize, RED);

    int fontsize2 = 25;
    int textwidth2 = MeasureText(texto1, fontsize);

    if ((int)(piscar * 10) % 10)
    {
        DrawText(texto2, screen->screenWidth / 2 - textwidth2 / 2 + 20, screen->screenHeight / 2 + 100, fontsize2, BLACK);
    }

    if (IsKeyPressed(KEY_ENTER))
        *estado = reset_game(matrix_info_game, screen);
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

Screen *definir_tela(int difficulty)
{
    Screen *tela = (Screen *)malloc(sizeof(Screen));

    switch (difficulty)
    {
    case 1:
        *tela = (Screen){
            .screenHeight = 650,
            .screenWidth = 500,
            .cell_size = 50,
            .difficulty = difficulty};
        break;
    case 2:
        *tela = (Screen){
            .screenHeight = 650,
            .screenWidth = 600,
            .cell_size = 50,
            .difficulty = difficulty};
        break;
    case 3:
        *tela = (Screen){
            .screenHeight = 650,
            .screenWidth = 600,
            .cell_size = 25,
            .difficulty = difficulty};
        break;

    default:
        *tela = (Screen){
            .screenHeight = 650,
            .screenWidth = 600,
            .cell_size = 50,
            .difficulty = difficulty};
        break;
    }

    tela->rows = floor((tela->screenHeight - 100.0) / tela->cell_size); // (100 / cell_size) para o espaço do menu;
    tela->cols = tela->screenWidth / tela->cell_size;

    return tela;
}

void criar_janela_dificuldade(MenuDifficulty *menu)
{
    menu->janela.height = menu->mini_menu.rectangle.height * 3;
    menu->janela.width = menu->mini_menu.rectangle.width;
    menu->janela.x = menu->mini_menu.rectangle.x;
    menu->janela.y = menu->mini_menu.rectangle.y + menu->mini_menu.rectangle.height;

    menu->options = (Option *)malloc(sizeof(Option) * 3);

    for (int i = 0; i < 3; i++)
    {
        menu->options[i].font_size = menu->mini_menu.font_size;
        menu->options[i].rectangle.height = menu->mini_menu.font_size;
        menu->options[i].rectangle.width = menu->mini_menu.rectangle.width;
        menu->options[i].rectangle.x = menu->mini_menu.rectangle.x;
        menu->options[i].rectangle.y = menu->mini_menu.rectangle.y + menu->mini_menu.font_size + (i * menu->mini_menu.font_size);
        menu->options[i].color = WHITE;

        switch (i)
        {
        case 0:
            menu->options[i].text = (char *)malloc(sizeof("Fácil"));
            strcpy(menu->options[i].text, "Fácil");
            break;
        case 1:
            menu->options[i].text = (char *)malloc(sizeof("Médio"));
            strcpy(menu->options[i].text, "Médio");
            break;
        case 2:
            menu->options[i].text = (char *)malloc(sizeof("Dificil") * sizeof(char));
            strcpy((menu->options[i].text), "Dificil");
            break;
        default:
            break;
        }
    }
}

MenuDifficulty *criar_menu_dificuldade(Screen *screen)
{
    MenuDifficulty *menu = (MenuDifficulty *)malloc(sizeof(MenuDifficulty));

    menu->mini_menu.color = WHITE;
    menu->mini_menu.font_size = 20;
    menu->mini_menu.rectangle.width = screen->screenWidth / 4 - 5;
    menu->mini_menu.rectangle.height = menu->mini_menu.font_size;
    menu->mini_menu.rectangle.x = screen->screenWidth / 2 + menu->mini_menu.rectangle.width;
    menu->mini_menu.rectangle.y = 5;
    menu->menu_aberto = false;

    switch (screen->difficulty)
    {
    case 1:
        menu->mini_menu.text = (char *)malloc(sizeof("Fácil"));
        strcpy(menu->mini_menu.text, "Fácil");
        break;
    case 2:
        menu->mini_menu.text = (char *)malloc(sizeof("Médio"));
        strcpy(menu->mini_menu.text, "Médio");
        break;
    case 3:
        menu->mini_menu.text = (char *)malloc(sizeof("Dificil") * sizeof(char));
        strcpy((menu->mini_menu.text), "Dificil");
        break;
    default:
        menu->mini_menu.text = (char *)malloc(sizeof("Médio"));
        strcpy(menu->mini_menu.text, "Médio");
        break;
    }

    criar_janela_dificuldade(menu);

    return menu;
}

void DrawMenuDifficulty(MenuDifficulty *menu)
{
    DrawRectangleRec(menu->mini_menu.rectangle, menu->mini_menu.color);
    DrawText(menu->mini_menu.text, menu->mini_menu.rectangle.x + 5, menu->mini_menu.rectangle.y, menu->mini_menu.font_size, BLACK);

    if (menu->menu_aberto)
    {
        DrawRectangleRec(menu->janela, WHITE);
        for (int i = 0; i < 3; i++)
        {
            DrawRectangleRec(menu->options[i].rectangle, menu->options[i].color);
            DrawText(menu->options[i].text, menu->options[i].rectangle.x + 5, menu->options[i].rectangle.y, menu->options[i].font_size, BLACK);
        }
    }
}

Texture2D carregar_textura(Screen *screen, char *src, int size)
{
    Image image = LoadImageFromMemory(".png", src, size);
    if (image.data == NULL)
        return (Texture2D){0};

    ImageResize(&image, screen->cell_size, screen->cell_size);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    return texture;
}

SoundList *carregar_som(char *src, int size)
{
    SoundList *lista_som = (SoundList *)malloc(sizeof(SoundList));
    
    Wave temp = LoadWaveFromMemory(".wav", src, size);
    lista_som->sound[0] = LoadSoundFromWave(temp);

    for (int i = 1; i < MAX_SOUNDS; i++)
        lista_som->sound[i] = LoadSoundAlias(lista_som->sound[0]);
    lista_som->currentsound = 0;

    return lista_som;
}

void reproduzir_audio(SoundList *som)
{
    PlaySound(som->sound[som->currentsound]);
    som->currentsound++;

    if (som->currentsound >= MAX_SOUNDS)
        som->currentsound = 0;
}

Volume *definir_volume(Screen *screen)
{
    Volume *volume = (Volume *)malloc(sizeof(Volume));
    volume->volume_off = carregar_textura(screen, volume_off, volume_off_size);
    volume->volume_on = carregar_textura(screen, volume_up, volume_up_size);
    volume->status = true;

    volume->collision.x = screen->screenWidth - 50;
    volume->collision.y = 50;
    volume->collision.height = 50;
    volume->collision.width = 50;

    return volume;
}

int main()
{
    srand(time(NULL));

    GameState *estado = malloc(sizeof(GameState));
    Screen *tela = NULL;
    Volume *volume = NULL;
    MenuDifficulty *menu_dificuldade = NULL;
    *estado = GamePlaying;
    tela = definir_tela(2);
    menu_dificuldade = criar_menu_dificuldade(tela);

    int bandeiras = 0;
    int quant_bombas = 0;
    float piscar = 0;
    int skip_animacao_derrota = 0;

    InitWindow(tela->screenWidth, tela->screenHeight, "Campo Minado");
    if (IsWindowState(FLAG_WINDOW_RESIZABLE))
        ClearWindowState(FLAG_WINDOW_RESIZABLE);

    Image img_icon = LoadImageFromMemory(".png", icon, icon_size);
    SetWindowIcon(img_icon);

    Texture2D flag = carregar_textura(tela, flag_header, flag_size);
    Texture2D tile = carregar_textura(tela, tile_header, tile_size);
    Texture2D tile_error = carregar_textura(tela, tile_error_header, tile_error_size);
    Texture2D mine = LoadTextureFromImage(LoadImageFromMemory(".png", mine_header, mine_header_size));
    SetTextureFilter(mine, TEXTURE_FILTER_POINT);
    volume = definir_volume(tela);

    InitAudioDevice();

    SoundList *flag_sound_down = carregar_som(tirar_bandeira, tirar_bandeira_size);
    SoundList *flag_sound_up = carregar_som(colocar_bandeira, colocar_bandeira_size);
    SoundList *mine_sound = carregar_som(mine_sound_header, mine_sound_header_size);
    SoundList *revelar_sound[5];

    revelar_sound[0] = carregar_som(revelar0, revelar0_size);
    revelar_sound[1] = carregar_som(revelar1, revelar1_size);
    revelar_sound[2] = carregar_som(revelar2, revelar2_size);
    revelar_sound[3] = carregar_som(revelar3, revelar3_size);
    revelar_sound[4] = carregar_som(revelar4, revelar4_size);

    Rectangle **matrix_view_game = criar_tabuleiro_visualizacao(tela);

    int **matrix_info_game = criar_tabuleiro_informacao(tela);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);
        Vector2 mousePoint = GetMousePosition();

        // Desenha o tabuleiro
        for (int i = 0; i < tela->rows; i++)
        {
            for (int j = 0; j < tela->cols; j++)
            {
                if (matrix_info_game[i][j] <= 0)
                {
                    Color color;
                    if (!CheckCollisionPointRec(mousePoint, matrix_view_game[i][j]))
                        color = WHITE;
                    else
                        color = LIGHTGRAY;

                    Rectangle fonteOriginal = {0, 0, (float)tile.width, (float)tile.height};
                    DrawTexturePro(tile,
                                   fonteOriginal,
                                   matrix_view_game[i][j], // Destino na tela
                                   (Vector2){0, 0},        // Origem de rotação
                                   0.0f,                   // Rotação
                                   color);
                }
                else if (matrix_info_game[i][j] >= 90 && matrix_info_game[i][j] < 95)
                {
                    Rectangle corte = {(matrix_info_game[i][j] - 90) * 50, 0, (float)mine.width / 5, (float)mine.height};
                    DrawTexturePro(mine,
                                   corte,
                                   matrix_view_game[i][j], // Destino na tela
                                   (Vector2){0, 0},        // Origem de rotação
                                   0.0f,                   // Rotação
                                   WHITE);
                }
                else if (matrix_info_game[i][j] == 95)
                {
                    Rectangle fonteOriginal = {0, 0, (float)tile_error.width, (float)tile_error.height};
                    DrawTexturePro(tile_error,
                                   fonteOriginal,
                                   matrix_view_game[i][j], // Destino na tela
                                   (Vector2){0, 0},        // Origem de rotação
                                   0.0f,                   // Rotação
                                   WHITE);
                }
                else
                    DrawRectangleRec(matrix_view_game[i][j], GRAY);
            }
        }

        // Desenha as linhas do tabuleiro
        for (int i = 0; i <= tela->cols; i++)
        {
            DrawLineV((Vector2){i * tela->cell_size, 0}, (Vector2){i * tela->cell_size, (float)tela->screenHeight}, LIGHTGRAY);
        }

        for (int i = 100 / tela->cell_size; i <= tela->rows + 2; i++)
        {
            DrawLineV((Vector2){0, i * tela->cell_size}, (Vector2){(float)tela->screenWidth, i * tela->cell_size}, LIGHTGRAY);
        }

        // Desenha a barra de menu superior
        DrawMenu(bandeiras, tela, volume);
        DrawMenuDifficulty(menu_dificuldade);

        // Escreve os valores se celula revelada

        for (int i = 0; i < tela->rows; i++)
        {
            for (int j = 0; j < tela->cols; j++)
            {
                if (matrix_info_game[i][j] <= 0 && matrix_info_game[i][j] >= -9)
                    continue;

                const char *text = TextFormat("%c", matrix_info_game[i][j] < -9 ? 'B' : (matrix_info_game[i][j] == 10 || matrix_info_game[i][j] == 9) ? ' '
                                                                                                                                                      : matrix_info_game[i][j] + '0');

                int font_size = tela->cell_size;
                int text_size = MeasureText(text, font_size);
                int x = matrix_view_game[i][j].x + (tela->cell_size / 2) - (text_size / 2);
                int y = matrix_view_game[i][j].y + (tela->cell_size / 2) - (font_size / 2);

                if (matrix_info_game[i][j] < -9)
                    DrawTexture(flag, j * tela->cell_size, i * tela->cell_size + 100, WHITE);

                else if (!(matrix_info_game[i][j] >= 90 && matrix_info_game[i][j] <= 95))
                    DrawText(text, x, y, font_size, define_cor(matrix_info_game[i][j]));
            }
        }

        // Verificacao mudanca de dificuldade
        if (CheckCollisionPointRec(mousePoint, menu_dificuldade->mini_menu.rectangle))
        {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                menu_dificuldade->menu_aberto = !menu_dificuldade->menu_aberto;
            }
        }
        else
        {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        // verifica a colisão nas opções de dificuldade
        for (size_t i = 0; i < 3; i++)
        {
            if (CheckCollisionPointRec(mousePoint, menu_dificuldade->options[i].rectangle) && menu_dificuldade->menu_aberto)
            {
                menu_dificuldade->options[i].color = LIGHTGRAY;

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && i + 1 != tela->difficulty)
                {
                    menu_dificuldade->menu_aberto = false;
                    strcpy(menu_dificuldade->mini_menu.text, menu_dificuldade->options[i].text);

                    free(tela);
                    free(matrix_info_game);
                    free(matrix_view_game);
                    free(menu_dificuldade);

                    tela = definir_tela(i + 1);
                    matrix_info_game = criar_tabuleiro_informacao(tela);
                    matrix_view_game = criar_tabuleiro_visualizacao(tela);
                    menu_dificuldade = criar_menu_dificuldade(tela);
                    flag = carregar_textura(tela, flag_header, flag_size);
                    tile = carregar_textura(tela, tile_header, tile_size);
                    *estado = reset_game(matrix_info_game, tela);
                    SetWindowSize(tela->screenWidth, tela->screenHeight);
                }
            }
            else
            {
                menu_dificuldade->options[i].color = WHITE;
            }
        }

        if (CheckCollisionPointRec(mousePoint, volume->collision) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            volume->status = !volume->status;

        // Jogo em execução
        if (*estado == GamePlaying)
        {
            if (!isFirstPlay())
                timer += GetFrameTime();
            // Verifica se clicou em alguma célula
            for (int i = 0; i < tela->rows; i++)
            {
                for (int j = 0; j < tela->cols; j++)
                {
                    if (CheckCollisionPointRec(mousePoint, matrix_view_game[i][j]))
                    {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && matrix_info_game[i][j] >= -9 && !(CheckCollisionPointRec(mousePoint, menu_dificuldade->mini_menu.rectangle)))
                        {
                            if (matrix_info_game[i][j] != -9 && matrix_info_game[i][j] <= 0 && volume->status)
                            {
                                if (matrix_info_game[i][j] >= -4)
                                { // Evita acesso idevido de memoria

                                    reproduzir_audio(revelar_sound[matrix_info_game[i][j] * -1]);
                                }
                                else
                                {
                                    reproduzir_audio(revelar_sound[1]);
                                }
                            }

                            if (isFirstPlay())
                            {
                                bandeiras = iniciar_partida(matrix_info_game, tela, i, j);
                                quant_bombas = bandeiras;
                                playing = 1;
                            }

                            if (matrix_info_game[i][j] < -9)
                                bandeiras++;

                            revelar_num(matrix_info_game, tela->rows, tela->cols, i, j);

                            if (matrix_info_game[i][j] == 9)
                            {
                                *estado = GameOver;
                                animacao_derrota(matrix_info_game, tela, (Vector2){i, j});
                                if (volume->status)
                                    reproduzir_audio(mine_sound);
                            }

                            if (verificar_vitoria(matrix_info_game, tela->rows, tela->cols, quant_bombas))
                            {
                                printf("GANHOU PAI!");
                                *estado = GameWinned;
                                bandeiras = quant_bombas;
                            }
                        }
                        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                        {
                            marca_bandeira(matrix_info_game, tela->rows, tela->cols, i, j);
                            if (matrix_info_game[i][j] >= -9 && matrix_info_game[i][j] < 0)
                            {
                                bandeiras++;
                                // Reproduzir SFX
                                if (volume->status)
                                    reproduzir_audio(flag_sound_down);
                            }
                            else if (matrix_info_game[i][j] <= -9 && matrix_info_game[i][j] < -9)
                            {
                                bandeiras--;
                                // Reproduzir SFX
                                if (volume->status)
                                    reproduzir_audio(flag_sound_up);
                            }
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                skip_animacao_derrota++;

            if (timer_derrota > 0.15)
            {
                animacao_derrota(matrix_info_game, tela, (Vector2){-1, -1});
                timer_derrota = 0;
            }
            timer_derrota += GetFrameTime();

            if (derrota || skip_animacao_derrota > 1)
            {
                desenha_tela_vitoria_derrota(matrix_info_game, tela, piscar, estado, "Você Perdeu!", "Pressione Enter para reiniciar!");

                if (IsKeyPressed(KEY_ENTER))
                    skip_animacao_derrota = 0;
            }

            piscar += GetFrameTime();
        }

        if (*estado == GameWinned)
        {
            DrawRectangle(0, tela->screenHeight / 2 - tela->screenHeight / 4, tela->screenWidth, tela->screenHeight / 2, ColorAlpha(WHITE, 0.7f));
            char *texto = "Você Ganhou!";
            int fontsize = 65;
            int textwidth = MeasureText(texto, fontsize);

            DrawText(texto, tela->screenWidth / 2 - textwidth / 2, tela->screenHeight / 2 - fontsize / 2 - 20, fontsize, GREEN);

            char *texto2 = "Pressione Enter para reiniciar!";
            int fontsize2 = 25;
            int textwidth2 = MeasureText(texto, fontsize);

            if ((int)(piscar * 10) % 10)
            {
                DrawText(texto2, tela->screenWidth / 2 - textwidth2 / 2 + 20, tela->screenHeight / 2 + 100, fontsize2, BLACK);
            }

            piscar += GetFrameTime();

            if (IsKeyPressed(KEY_ENTER))
                *estado = reset_game(matrix_info_game, tela);
        }

        EndDrawing();
    }

    CloseWindow();
    UnloadTexture(flag);
    UnloadTexture(tile);
    UnloadTexture(tile_error);
    UnloadTexture(mine);

    for (size_t i = 0; i < MAX_SOUNDS; i++)
    {
        UnloadSound(flag_sound_down->sound[i]);
    }

    for (size_t i = 0; i < MAX_SOUNDS; i++)
    {
        UnloadSound(flag_sound_up->sound[i]);
    }

    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < MAX_SOUNDS; j++)
        {
            UnloadSound(revelar_sound[i]->sound[j]);
        }
    }

    for (int i = 0; i < tela->rows; i++)
    {
        free(matrix_info_game[i]);
    }

    for (int i = 0; i < tela->rows; i++)
    {
        free(matrix_view_game[i]);
    }

    free((*revelar_sound));
    free(flag_sound_down);
    free(flag_sound_up);
    free(*matrix_info_game);
    free(*matrix_view_game);
    free(estado);
    free(tela);

    return 0;
}
