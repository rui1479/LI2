#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define MAP_WIDTH 203
#define MAP_HEIGHT 48
#define space 32
#define enter 10

short arr_size_x;
#define s_wall "#"
#define s_exit "***"
#define s_empty "."
#define s_door "-^-"
#define s_space "..."
#define s_enemy_C "(c)"
#define s_enemy_I "(i)"
#define s_enemy_E "(e)"

struct class_obj
{
    char symbol[20];
    int hsp, vsp;
    int x, y;
    int direction;
    int lastKnownPlayerX, lastKnownPlayerY;

};

struct class_obj player;
struct class_obj enemy[5];

const short c_hud = 2;
bool EXIT = false;
int tecla_pressionada = 0;

short lifes = 1;

// index map object
#define i_wall 1
#define i_door 2
#define i_space 3
#define i_enemy_C 4
#define i_exit 5
#define i_enemy_I 6
#define i_empty 7
#define i_enemy_E 8

// Const Color
const short c_wall = 1;
const short c_door = 2;
const short c_space = 3;
const short c_empty = 4;
const short c_player = 5;
const short c_bullet = 6;
const short c_enemy = 7;
const short c_box = 8;
const short c_hud = 9;
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////
// HUB
///////////////////

void game_over()
{
    EXIT = true;
    endwin();
    printf("GAME OVER!\n");
}

// HUD do jogo
short hp = 100;
short kills = 0;
short score = 0;

int numero_mobs = 0;

// Desenhar o HUD
void desenha_hud()
{
    if (kills == 0)
    {
        mvprintw(1, 2, "objetivo: %d%%   kills: %d   hp: %d\n", score, kills, hp);
        return;
    }
    mvprintw(1, 2, "objetivo: %d%%   kills: %d   hp: %d\n", ((score * 100) / numero_mobs), kills, hp);
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Menu
///////////////////

// Tamanho da janela
int largura, altura;

const char *menu_logo[10] = {
    "#############    #############    ####      ####    ############",
    "#############    #############    ######  ######    ############",
    "####             ####     ####    #### #### ####    ####",
    "####             ####     ####    ####  ##  ####    ####",
    "####  #######    ####     ####    ####      ####    #########",
    "####  #######    #############    ####      ####    #########",
    "####     ####    #############    ####      ####    ####",
    "####     ####    ####     ####    ####      ####    ####",
    "#############    ####     ####    ####      ####    ############",
    "#############    ####     ####    ####      ####    ############",
};

int str_len(const char *str)
{
    int tamanho = 0;
    while (*str++)
        ++tamanho;
    return tamanho;
}

int altura_logo = sizeof(menu_logo) / sizeof(menu_logo[0]);

int calcula_largura_logo()
{
    int largura_logo = 1;
    for (int i = 0; i < altura_logo; i++)
    {
        int aux = str_len(menu_logo[i]);
        if (aux > largura_logo)
        {
            largura_logo = str_len(menu_logo[i]);
        }
    }
    return largura_logo;
}

int largura_logo = 1;
void desenha_logo(int largura)
{
    // Calcula largura
    if (largura_logo == 1)
    {
        largura_logo = calcula_largura_logo() / 2;
    }

    // Desenha
    attron(COLOR_PAIR(c_hud));
    for (int i = 0; i < altura_logo; i++)
    {
        mvprintw(3 + i, largura / 2 - largura_logo, "%s", menu_logo[i]);
    }
    attroff(COLOR_PAIR(c_hud));
}
/////////////////////////////////////////////////////////////////////////
/////////////
// PLAYER
////////////
int dir_x;
int dir_y;
int dir_shoot;

void player_move(int key)
{
    // Key check
    int key_left = (key == KEY_LEFT) ? 1 : 0;
    int key_right = (key == KEY_RIGHT) ? 1 : 0;
    int key_down = (key == KEY_DOWN) ? 1 : 0;
    int key_up = (key == KEY_UP) ? 1 : 0;

    // key dir
    dir_x = key_right - key_left;
    dir_y = key_down - key_up;

    // Animation and direction shoot
    if (dir_x == 0 && dir_y == 0)
    {
        strcpy(player.symbol, "|0|");
    }
    else
    {
        if (dir_x == 1)
        {
            dir_shoot = 1;
            strcpy(player.symbol, "|0>");
        }
        if (dir_x == -1)
        {
            dir_shoot = -1;
            strcpy(player.symbol, "<0|");
        }
        if (dir_y == -1)
        {
            dir_shoot = -2;
            strcpy(player.symbol, "/0\\");
        }
        if (dir_y == 1)
        {
            dir_shoot = 2;
            strcpy(player.symbol, "\\0/");
        }
    }

    player.hsp = 1 * dir_x;
    player.vsp = 1 * dir_y;

    if (player.hsp != 0)
    {
        player.vsp = 0;
    }
    else if (player.vsp != 0)
    {
        player.hsp = 0;
    }

    player.x += player.hsp;
    player.y += player.vsp;

    for (int i = 0; i < sizeof(enemy) / sizeof(enemy[0]); i++) {
        enemy[i].lastKnownPlayerX = player.x;
        enemy[i].lastKnownPlayerY = player.y;
    }
}

// Collsiion
void player_collision(short current_lvl[][arr_size_x])
{
    switch (current_lvl[player.y][player.x])
    {

    // Collision
    case i_wall:  // wall
    case i_door:  // door
    case i_space: // space
        player.x -= player.hsp;
        player.y -= player.vsp;
        break;
    }

    // Enemy collision
    for (short i = 0; i < sizeof(enemy) / sizeof(enemy[0]); i++)
    {
        if (player.y == enemy[i].y &&
            player.x == enemy[i].x)
        {
            lifes = lifes - 1;
        }
    }
}

//////////////
// OBJECT
//////////////

// Enemy movement
void enemy_move(short current_lvl[][arr_size_x], int index)
{
    // Se o mob é estúpido
    if (strcmp(enemy[index].symbol, "(e)") == 0)
    {
        // Dada a posição do jogador, ajuste a direção do mob estúpido para persegui-lo.
        if (player.y > enemy[index].y)
        {
            enemy[index].vsp = 1;
            enemy[index].y += enemy[index].vsp;
        }
        else if (player.y < enemy[index].y)
        {
            enemy[index].vsp = -1;
            enemy[index].y += enemy[index].vsp;
        }
        else if (player.x > enemy[index].x)
        {
            enemy[index].hsp = 1;
            enemy[index].x += enemy[index].hsp;
        }
        else if (player.x < enemy[index].x)
        {
            enemy[index].hsp = -1;
            enemy[index].x += enemy[index].hsp;
        }
        return;
    }
    else if (strcmp(enemy[index].symbol, "(c)") == 0)
    {
        if (isPlayerNearby(index,5))
        {
            if (isAllyNearby(index))
            {
                // Ataque o jogador - simplificado para mover na direção do jogador
                if (player.y > enemy[index].y)
                {
                    enemy[index].vsp = 1;
                    enemy[index].y += enemy[index].vsp;
                }
                else if (player.y < enemy[index].y)
                {
                    enemy[index].vsp = -1;
                    enemy[index].y += enemy[index].vsp;
                }
                else if (player.x > enemy[index].x)
                {
                    enemy[index].hsp = 1;
                    enemy[index].x += enemy[index].hsp;
                }
                else if (player.x < enemy[index].x)
                {
                    enemy[index].hsp = -1;
                    enemy[index].x += enemy[index].hsp;
                }
            }
            else
            {
                // Grita e foge do jogador - simplificado para mover na direção oposta ao jogador
                if (player.y > enemy[index].y)
                {
                    enemy[index].vsp = -1;
                    enemy[index].y += enemy[index].vsp;
                }
                else if (player.y < enemy[index].y)
                {
                    enemy[index].vsp = 1;
                    enemy[index].y += enemy[index].vsp;
                }
                else if (player.x > enemy[index].x)
                {
                    enemy[index].hsp = -1;
                    enemy[index].x += enemy[index].hsp;
                }
                else if (player.x < enemy[index].x)
                {
                    enemy[index].hsp = 1;
                    enemy[index].x += enemy[index].hsp;
                }
            }
        }
        else
        {
            if (isAllyNearby(index))
            {
                // Junta-se aos companheiros - simplificado para mover em direção ao aliado mais próximo
                int closestAlly = -1;
                for (int i = 0; i < sizeof(enemy) / sizeof(enemy[0]); i++)
                {
                    if (i != index && (closestAlly == -1 ||
                                       abs(enemy[i].x - enemy[index].x) + abs(enemy[i].y - enemy[index].y) <
                                           abs(enemy[closestAlly].x - enemy[index].x) + abs(enemy[closestAlly].y - enemy[index].y)))
                    {
                        closestAlly = i;
                    }
                }
                if (closestAlly != -1)
                {
                    if (enemy[closestAlly].y > enemy[index].y)
                    {
                        enemy[index].vsp = 1;
                        enemy[index].y += enemy[index].vsp;
                    }
                    else if (enemy[closestAlly].y < enemy[index].y)
                    {
                        enemy[index].vsp = -1;
                        enemy[index].y += enemy[index].vsp;
                    }
                    else if (enemy[closestAlly].x > enemy[index].x)
                    {
                        enemy[index].hsp = 1;
                        enemy[index].x += enemy[index].hsp;
                    }
                    else if (enemy[closestAlly].x < enemy[index].x)
                    {
                        enemy[index].hsp = -1;
                        enemy[index].x += enemy[index].hsp;
                    }
                }
            }
            else
            {
                // Move-se aleatoriamente
                int randomDirection = rand() % 4;
                switch (randomDirection)
                {
                case 0:
                    enemy[index].vsp = 1; // move para baixo
                    break;
                case 1:
                    enemy[index].vsp = -1; // move para cima
                    break;
                case 2:
                    enemy[index].hsp = 1; // move para a direita
                    break;
                case 3:
                    enemy[index].hsp = -1; // move para a esquerda
                    break;
                }
                enemy[index].x += enemy[index].hsp;
                enemy[index].y += enemy[index].vsp;
            }
        }
        return;
    }
    else if (strcmp(enemy[index].symbol, "(i)" == 0))
    {
        if (strcmp(enemy[index].symbol, "smart") == 0)
        {
            // Se o jogador está próximo
            if (isPlayerNearby(index, 10))
            {
                // Se o mob está perto o suficiente para atacar com uma bomba
                if (isPlayerNearby(index, 3))
                {
                    // Use uma bomba
                    useBomb(index);
                }
                else
                {
                    // Mover na direção do jogador para atacar
                    findDirection(enemy[index].x, enemy[index].y, player.x, player.y, &enemy[index].hsp, &enemy[index].vsp);
                    enemy[index].x += enemy[index].hsp;
                    enemy[index].y += enemy[index].vsp;
                }
            }
            else
            {
                // Se não há jogador próximo, mover na direção da última posição conhecida do jogador
                findDirection(enemy[index].x, enemy[index].y, enemy[index].lastKnownPlayerX, enemy[index].lastKnownPlayerY, &enemy[index].hsp, &enemy[index].vsp);
                enemy[index].x += enemy[index].hsp;
                enemy[index].y += enemy[index].vsp;
            }
            return;
        }
    }
}

// Função para verificar se o jogador está próximo
bool isPlayerNearby(int index, int range) {
    // Este é apenas um exemplo. Substitua por sua própria lógica.
    return abs(enemy[index].x - player.x) <= range && abs(enemy[index].y - player.y) <= range;
}


// Função para verificar se um aliado está próximo
bool isAllyNearby(int index)
{
    // Este é apenas um exemplo. Substitua por sua própria lógica.
    for (int i = 0; i < sizeof(enemy) / sizeof(enemy[0]); i++)
    {
        if (i != index && abs(enemy[i].x - enemy[index].x) <= 5 && abs(enemy[i].y - enemy[index].y) <= 5)
        {
            return true;
        }
    }
    return false;
}

// Encontrar a direção para mover de um ponto a outro
void findDirection(int startX, int startY, int endX, int endY, int *hsp, int *vsp)
{
    if (endY > startY)
    {
        *vsp = 1;
    }
    else if (endY < startY)
    {
        *vsp = -1;
    }
    else if (endX > startX)
    {
        *hsp = 1;
    }
    else if (endX < startX)
    {
        *hsp = -1;
    }
}

// Enemy update
void enemy_update(short current_lvl[][arr_size_x])
{
    for (int i = 0; i < sizeof(enemy) / sizeof(enemy[0]); i++)
    {
        enemy_move(current_lvl, i);
    }
}

// Enemy clear
void clear_enemy()
{
    for (int i = 0; i < sizeof(enemy) / sizeof(enemy[0]); i++)
    {
        enemy[i].y = 0;
        enemy[i].x = 0;
        enemy[i].direction = 0;
    }
}

// set obj Parametrs
void obj_init(struct class_obj *obj, int x, int y, int dir, char *objname)
{
    obj->x = x;
    obj->y = y;
    obj->direction = dir;
    strcpy(obj->symbol, objname);
}

/////////////////////////////////////////////////////////////////////////
///////////////
// MAPA
//////////////

void generateMap(int width, int height, char** map) {
    // Criação das paredes nas primeiras e últimas linhas
    for (int i = 0; i < width; i++) {
        map[0][i] = '#';
        map[1][i] = '#';
        map[height - 2][i] = '#';
        map[height - 1][i] = '#';
    }

    // Criação das paredes nas primeiras e últimas colunas
    for (int i = 0; i < height; i++) {
        map[i][0] = '#';
        map[i][1] = '#';
        map[i][width - 2] = '#';
        map[i][width - 1] = '#';
    }

    // Preenchimento do restante do mapa com paredes probabilísticas
    srand(time(NULL)); // Inicialização do gerador de números aleatórios
    for (int i = 2; i < height - 2; i++) {
        for (int j = 2; j < width - 2; j++) {
            int randomValue = rand() % 101; // Gera um número aleatório entre 0 e 100

            if (randomValue <= 40) {
                map[i][j] = '#';
            }
        }
    }
}

void printMap(int width, int height, char** map) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
}

void updateMap(int width, int height, char** map) {
    char** tempMap = (char**)malloc(height * sizeof(char*));
    for (int i = 0; i < height; i++) {
        tempMap[i] = (char*)malloc(width * sizeof(char));
        memcpy(tempMap[i], map[i], width); // Copia o mapa original para o mapa temporário
    }

    for (int i = 2; i < height - 2; i++) {
        for (int j = 2; j < width - 2; j++) {
            // Verifica as 9 células ao redor
            int count = 0;
            for (int k = i - 1; k <= i + 1; k++) {
                for (int l = j - 1; l <= j + 1; l++) {
                    if (map[k][l] == '#') {
                        count++;
                    }
                }
            }
            // Define como parede se tiver 5 ou mais paredes vizinhas
            if (count >= 5) {
                tempMap[i][j] = '#';
            } else {
                tempMap[i][j] = ' ';
            }
        }
    }

    // Copia o mapa atualizado de volta para o mapa original
    for (int i = 2; i < height - 2; i++) {
        for (int j = 2; j < width - 2; j++) {
            map[i][j] = tempMap[i][j];
        }
    }

    // Libera a memória do mapa temporário
    for (int i = 0; i < height; i++) {
        free(tempMap[i]);
    }
    free(tempMap);
}

/////////////////////////////////////////////////////////////////////////
///////////////
// MAIN
//////////////
int main()
{
    // Inicializa a biblioteca ncurses
    initscr();
    raw();
    keypad(stdscr, TRUE);
    savetty();
    cbreak();
    noecho();
    timeout(0);
    leaveok(stdscr, TRUE);
    curs_set(0);
char** map = (char**)malloc(MAP_HEIGHT * sizeof(char*));
        for (int i = 0; i < MAP_HEIGHT; i++) {
        map[i] = (char*)malloc(MAP_WIDTH * sizeof(char));
        memset(map[i], ' ', MAP_WIDTH); // Preenche a linha com espaços em branco
        }

    // Enumera os estados do jogo
    typedef enum
    {
        ESTADO_MENU,
        ESTADO_GAME,
        ESTADO_EXIT,
    } estados_jogo;

    // Inicia o estado atual
    estados_jogo estado_atual;
    estado_atual = ESTADO_MENU;

    const char *item_start_game[2] = {
        "> START GAME <",
        "START GAME",
    };

    const char *item_exit[2] = {
        "> EXIT <",
        "EXIT",
    };

    while (!EXIT)
    {

        // Cor
        SetColor();

        // Calcula altura e largura da janela
        getmaxyx(stdscr, altura, largura);

        // Estado do menu
        static int item_menu = 0;
        if (tecla_pressionada == KEY_UP)
            item_menu = 0;
        if (tecla_pressionada == KEY_DOWN)
            item_menu = 1;

        switch (estado_atual)
        {
        // Menu
        case ESTADO_MENU:
            desenha_logo(largura);

            int select_start_game = item_menu == 0 ? 0 : 1;
            mvprintw(altura / 2 + 3, largura / 2 - str_len(item_start_game[select_start_game]) / 2, "%s", item_start_game[select_start_game]);

            int select_exit = item_menu == 1 ? 0 : 1;
            mvprintw(altura / 2 + 5, largura / 2 - str_len(item_exit[select_exit]) / 2, "%s", item_exit[select_exit]);

            mvprintw(altura - 5, largura / 2 - str_len("DEVELOPERS") / 2, "DEVELOPERS");

            mvprintw(altura - 3, largura / 2 - str_len(" > João Veloso   > Gonçalo Gonçalves     > Pedro Figueiredo   > Rui Ribeiro") / 2, " > João Veloso   > Gonçalo Gonçalves     > Pedro Figueiredo   > Rui Ribeiro ");

            // Desenha caixa em volta do menu
            attron(COLOR_PAIR(c_hud));
            box(stdscr, 0, 0);
            attron(COLOR_PAIR(c_hud));

            // Muda o estado_atual quando se clica no enter
            if (tecla_pressionada == enter)
            {
                switch (item_menu)
                {
                case 0:
                    estado_atual = ESTADO_GAME;
                    break;

                case 1:
                    estado_atual = ESTADO_EXIT;
                    break;
                }
            }
            break;

        // Jogo
        case ESTADO_GAME:

            generateMap(MAP_WIDTH, MAP_HEIGHT, map);
            printMap(MAP_WIDTH, MAP_HEIGHT, map);
            desenha_hud();
            box(stdscr, 0, 0);
            break;

        // Sair
        case ESTADO_EXIT:
            endwin();
            EXIT = TRUE;
            break;
        }

        // Sair para o menu
        if (tecla_pressionada == 'q' || tecla_pressionada == 'Q')
            estado_atual = ESTADO_MENU;

        // Ler tecla pressionada
        tecla_pressionada = wgetch(stdscr);
        napms(50);
        tecla_pressionada = wgetch(stdscr);

        // Clear
        erase();
    }

    // Finaliza a biblioteca ncurses
    endwin();

    return 0;
}