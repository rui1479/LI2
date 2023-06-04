#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


#define MAP_WIDTH largura
#define MAP_HEIGHT altura
#define space 32
#define enter 10
#define MAP_RADIUS 5

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
    for (short i = 0; i < (short)(sizeof(enemy) / sizeof(enemy[0])); i++)

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

void desenhaMobs(char **map, WINDOW *win)
{
    int largura = getmaxx(win);
    int altura = getmaxy(win);
    for (int y = 0; y < altura; y++) {
            for (int x = 0; x < largura; x++) {
                if (map[y][x] == 'e'||map[y][x] == 'b' || map[y][x] == 'c')
                mvwaddch(win, y, x, map[y][x]);
            }
        }
}

void spawnMob(int playerX, int playerY, char **map, WINDOW *win)
{
    int mobX, mobY;
    do {
        mobX = rand() % (MAP_WIDTH - 4) + 2;
        mobY = rand() % (MAP_HEIGHT - 4) + 2;
    } while (map[mobY][mobX] != '.' || (abs(mobX - playerX) <= MAP_RADIUS && abs(mobY - playerY) <= MAP_RADIUS));
    int value = rand() % 3 + 1;
        switch (value) 
        {   case 1 : map[mobY][mobX] = 'e'; mvwaddch(win, mobY, mobX, 'e');break;
            case 2 : map[mobY][mobX] = 'b'; mvwaddch(win, mobY, mobX, 'b');break;
            case 3 : map[mobY][mobX] = 'c'; mvwaddch(win, mobY, mobX, 'c');break;
            default : break;
        }

}



/////////////////////////////////////////////////////////////////////////
///////////////
// MAPA
//////////////

void addBorder(int width, int height, char **map)
{
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
}

void generateMap(int width, int height, char **map)
{   // Preenchimento do restante do mapa com paredes probabilísticas
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

void printMap(int width, int height, char **map)
{
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
}

void updateMap(int width, int height, char **map)
{
    char **tempMap = (char **)malloc(height * sizeof(char *));
    for (int i = 0; i < height; i++) {
        tempMap[i] = (char *)malloc(width * sizeof(char));
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
                tempMap[i][j] = '.';
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

bool verificaParedeNaDirecao(int playerX, int playerY, double x, double y, char **map, WINDOW *win) {
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        if (map[(int)y][(int)x] == '#') {
            mvwaddch(win, y, x, map[(int)y][(int)x]);
            return false;
        }
    }
    mvwaddch(win, y, x, map[(int)y][(int)x]);return true;
}

void desenhaVisao(int playerX, int playerY, char **map, WINDOW *win, int visaoAtivada) {
    int largura = getmaxx(win);
    int altura = getmaxy(win);

    if (visaoAtivada) {
        double anguloIncremento = M_PI / 64.0;
        for (double angulo = 0; angulo < 2.0 * M_PI; angulo += anguloIncremento) {
            for (int i = 1; i < MAP_RADIUS; i++) {
                double x = playerX + i * cos(angulo);
                double y = playerY + i * sin(angulo);
                if (verificaParedeNaDirecao(playerX, playerY, x, y, map, win)) {
                    mvwaddch(win, y, x, map[(int)y][(int)x]);
                } else {
                    break;
                }
            }
        }
    } else {
        for (int y = 0; y < altura; y++) {
            for (int x = 0; x < largura; x++) {
                mvwaddch(win, y, x, map[y][x]);
            }
        }
    }
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
        //SetColor();

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


            // Cria uma nova janela
    WINDOW *win = newwin(MAP_HEIGHT, MAP_WIDTH, 0, 0);



    // Habilita o uso de teclas especiais (como as setas do teclado)
    keypad(win, TRUE);

    // Oculta a exibição do cursor
    curs_set(0);

    // Configura as cores para usar o padrão do terminal
    start_color();
    use_default_colors();

    // Define as cores para a parede e o jogador
    init_pair(1, COLOR_WHITE, -1);     // Padrão (parede)
    init_pair(2, COLOR_YELLOW, -1);    // Jogador
    init_pair(3, COLOR_RED, -1);       // Mobs 

    char **map = (char **)malloc(MAP_HEIGHT * sizeof(char *));
    for (int i = 0; i < MAP_HEIGHT; i++) {
        map[i] = (char *)malloc(MAP_WIDTH * sizeof(char));
        memset(map[i], '.', MAP_WIDTH); // Preenche a linha com pontos
    }
    addBorder(MAP_WIDTH, MAP_HEIGHT, map);
    generateMap(MAP_WIDTH, MAP_HEIGHT, map);

    int playerX = MAP_WIDTH / 2;
    int playerY = MAP_HEIGHT / 2;
    int visaoAtivada = 0;
    int move_Count = 5;

    int c;
    do {
        // Limpa a janela
        werase(win);
        wattron(win, COLOR_PAIR(1));
        // Desenha a visão do jogador
        desenhaVisao(playerX, playerY, map, win, visaoAtivada);
        wattroff(win, COLOR_PAIR(1));
        // Desenha o jogador na posição atual
        wattron(win, COLOR_PAIR(2));
        mvwprintw(win, playerY, playerX, "@");
        wattroff(win, COLOR_PAIR(2));
        if (move_Count == 5)
        {   
            spawnMob(playerX, playerY, map, win);
            move_Count = 0;    
        }
        wattron(win, COLOR_PAIR(3));
        desenhaMobs(map, win);
        wattroff(win, COLOR_PAIR(3));

        // Atualiza a janela
        wrefresh(win);

        // Espera pela entrada do usuário
        c = wgetch(win);
        // Limpa a posição atual do jogador no mapa
        map[playerY][playerX] = '.';
        // Atualiza a posição do jogador com base na entrada do usuário
        switch (c) {
    case KEY_UP:
        if (playerY > 2 && map[playerY - 1][playerX] != '#') {
            playerY--;
            move_Count++;
        }
        break;
    case KEY_DOWN:
        if (playerY < MAP_HEIGHT - 3 && map[playerY + 1][playerX] != '#') {
            playerY++;
            move_Count++;
        }
        break;
    case KEY_LEFT:
        if (playerX > 2 && map[playerY][playerX - 1] != '#') {
            playerX--;
            move_Count++;
        }
        break;
    case KEY_RIGHT:
        if (playerX < MAP_WIDTH - 3 && map[playerY][playerX + 1] != '#') {
            playerX++;
            move_Count++;
        }
        break;
    case 'p':
        updateMap(MAP_WIDTH, MAP_HEIGHT, map);
        break;
    case 't':
        visaoAtivada = !visaoAtivada;
        break;
    default:
        break;
}
        // Atualiza a posição atual do jogador no mapa
        map[playerY][playerX] = '@';

    } while (c != 'q');
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



