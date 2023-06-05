#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <SDL2/SDL.h>


#define MAP_WIDTH largura
#define MAP_HEIGHT altura
#define MAP_RADIUS 7
#define space 32
#define enter 10

struct class_obj
{
    char symbol[20];
    int x, y;
    int direction;
};
struct class_obj player;
struct class_obj mob;
int life = 1;
int monstrosMortos = 0;

const short c_hud = 2;
bool EXIT = false;
int tecla_pressionada = 0;

/////////////////////////////////////////////////////////////////////////
////////////////////
// Menu
///////////////////

void game_over(int monstrosMortos, int tempo_jogado, int m) {
    // Limpa a tela
    clear();

    // Obtém as dimensões da janela
    int largura, altura;
    getmaxyx(stdscr, altura, largura);

    int score = tempo_jogado + monstrosMortos*50;

    // Define as coordenadas para exibir o texto de game over centralizado
    int x = largura / 2 - 8;
    int y = altura / 2;

    // Imprime o texto de game over na tela
    mvprintw(y, x, "GAME OVER");

    // Exibe as estatísticas
    mvprintw(y + 2, x, "Monstros Mortos: %d", monstrosMortos);
    mvprintw(y + 4, x, "Tempo Jogado: %02d:%02d:%02d", tempo_jogado / 3600, (tempo_jogado % 3600) / 60, tempo_jogado % 60);
    mvprintw(y + 6, x, "Movimentos: %d", m);
    mvprintw(y + 8, x, "Score: %d", score);


    // Atualiza a tela
    refresh();

    // Pausa o programa por 1000 segundos antes de encerrar
    sleep(1000);
}



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
////////////////////
// Player
///////////////////
void init_player(struct class_obj *player, int x, int y, int dir, char *symbol)
{
    player->x = x;
    player->y = y;
    player->direction = dir;
    strcpy(player->symbol, symbol);
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Mob
///////////////////
void init_mob(struct class_obj *mob, char **map, struct class_obj *player)
{
    int mobX, mobY;
    do
    {
        mobX = rand() % (MAP_WIDTH - 4) + 2;
        mobY = rand() % (MAP_HEIGHT - 4) + 2;
    } while (map[mobY][mobX] != '.' || (abs(mobX - player->x) <= MAP_RADIUS && abs(mobY - player->y) <= MAP_RADIUS));
    int value = rand() % 3 + 1;
    switch (value)
    {
    case 1:
        map[mobY][mobX] = 'e';
        *mob->symbol = 'e';
        break;
    case 2:
        map[mobY][mobX] = 'b';
        *mob->symbol = 'b';
        break;
    case 3:
        map[mobY][mobX] = 'c';
        *mob->symbol = 'c';
        break;
    default:
        break;
    }
}

int calculaDistancia(struct class_obj *obj, int x, int y)
{
    int distX = abs(obj->x - x);
    int distY = abs(obj->y - y);
    return distX + distY;
}

void moveMob(struct class_obj *player, char **map, int mobx, int moby)
{
    int closestDist = INT_MAX;
    int closestX = -1;
    int closestY = -1;
    char temp = map[moby][mobx];

    // Coordenadas das posições acima, abaixo, à esquerda e à direita
    int positions[][2] = {
        {moby - 1, mobx}, // Acima
        {moby + 1, mobx}, // Abaixo
        {moby, mobx - 1}, // Esquerda
        {moby, mobx + 1}  // Direita
    };

    // Percorre as posições adjacentes ao mob
    for (int i = 0; i < 4; i++)
    {
        int y = positions[i][0];
        int x = positions[i][1];

        // Verifica se a posição é válida e se está vazia
        if (y >= 0 && y < MAP_HEIGHT && x >= 0 && x < MAP_WIDTH && map[y][x] == '.')
        {
            // Calcula a distância apenas para movimentos horizontais ou verticais
            if (y == moby || x == mobx)
            {
                int dist = calculaDistancia(player, x, y);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestX = x;
                    closestY = y;
                }
            }
        }
    }

    // Move o mob para a posição mais próxima do jogador
    if (closestX != -1 && closestY != -1)
    {
        map[moby][mobx] = '.'; // Remove o mob da posição atual
        mobx = closestX;       // Atualiza as coordenadas x e y do mob
        moby = closestY;
        map[moby][mobx] = temp; // Coloca o mob na nova posição

        if (player->x == mobx && player->y == moby)
            life--;
    }
}

void moveAllMobs(struct class_obj *player, char **map)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (map[y][x] == 'b' || map[y][x] == 'c' || map[y][x] == 'e')
            {
                moveMob(player, map, x, y);
            }
        }
    }
}

void desenhaMobs(char **map, WINDOW *win)
{
    int largura = getmaxx(win);
    int altura = getmaxy(win);
    for (int y = 0; y < altura; y++)
    {
        for (int x = 0; x < largura; x++)
        {
            if (map[y][x] == 'e' || map[y][x] == 'b' || map[y][x] == 'c')
                mvwaddch(win, y, x, map[y][x]);
        }
    }
}

bool mobProx(int x, int y, char **mapa, int raio)
{
    int startX = x - raio;
    int endX = x + raio;
    int startY = y + raio;
    int endY = y - raio;

    for (int i = startX; i <= endX; i++)
    {
        for (int j = startY; j >= endY; j--)
        {
            // Verifica se as coordenadas estão dentro do mapa
            if (i >= 0 && i < MAP_WIDTH && j >= 0 && j < MAP_HEIGHT)
            {
                if (mapa[j][i] == 'b' || mapa[j][i] == 'c' || mapa[j][i] == 'e')
                {
                    // Mob encontrado dentro do raio
                    return true;
                }
            }
        }
    }

    // Nenhum mob encontrado dentro do raio
    return false;
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Combate
///////////////////
bool ataque_ranged(int px, int py, int dir, char **map, WINDOW *win)
{
    int playerX = px;
    int playerY = py;
    int targetX = -1;
    int targetY = -1;
    int direction = dir;

    switch (direction)
    {
    case 1: // Cima
        targetX = playerX;
        targetY = playerY - 1;
        break;
    case 2: // Baixo
        targetX = playerX;
        targetY = playerY + 1;
        break;
    case 3: // Esquerda
        targetX = playerX - 1;
        targetY = playerY;
        break;
    case 4: // Direita
        targetX = playerX + 1;
        targetY = playerY;
        break;
    default:
        return false; // Direção inválida, não faz nada
    }

    char targetBlock = map[targetY][targetX];

    while (targetBlock == '.')
    {
        int prevX = targetX;
        int prevY = targetY;

        map[prevY][prevX] = '.';
        mvwaddch(win, prevY, prevX, map[prevY][prevX]); // Atualiza a exibição na janela

        targetX += (direction == 3) ? -1 : (direction == 4) ? 1 : 0;
        targetY += (direction == 1) ? -1 : (direction == 2) ? 1 : 0;
        
        targetBlock = map[targetY][targetX];
        
        if (targetBlock == '.')
        {
            map[targetY][targetX] = '*';
            mvwaddch(win, targetY, targetX, map[targetY][targetX]); // Atualiza a exibição na janela
        }
    }

    if (targetBlock == '#')
    {
        // Bloco sólido, não faz nada
        return false;
    }
    else if (targetBlock == 'b' || targetBlock == 'e' || targetBlock == 'c')
    {
        map[targetY][targetX] = '*';
        mvwaddch(win, targetY, targetX, map[targetY][targetX]); // Atualiza a exibição na janela
        
        monstrosMortos++;
        return true;
    }

    return false;
}





bool ataque_melee(int px, int py, int dir, char **map, WINDOW *win)
{
    int playerX = px;
    int playerY = py;
    int targetX = -1;
    int targetY = -1;
    int direction = dir;

    switch (direction)
    {
    case 1: // Cima
        targetX = playerX;
        targetY = playerY - 1;
        break;
    case 2: // Baixo
        targetX = playerX;
        targetY = playerY + 1;
        break;
    case 3: // Esquerda
        targetX = playerX - 1;
        targetY = playerY;
        break;
    case 4: // Direita
        targetX = playerX + 1;
        targetY = playerY;
        break;
    default:
        return false; // Direção inválida, melee attack não faz nada
    }

    char targetBlock = map[targetY][targetX];

    if (targetBlock == '#')
    {
        // Bloco sólido, melee attack não faz nada
        return false;
    }
    else if (targetBlock == 'b' || targetBlock == 'e' || targetBlock == 'c')
    {
        map[targetY][targetX] = '.';
        mvwaddch(win, targetY, targetX, map[targetY][targetX]);
        monstrosMortos++;
        return true;
    }
    
    return false;
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Mapa
///////////////////

void limpa_mortos(char **mapa, WINDOW *win)
{
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            if (mapa[i][j] == '*')
            {
                mapa[i][j] = '.';
                mvwaddch(win, i, j, '.'); // Atualiza a exibição na janela
            }
        }
    }
}


void addBorder(int width, int height, char **map)
{
    for (int i = 0; i < width; i++)
    {
        map[0][i] = '#';
        map[1][i] = '#';
        map[height - 2][i] = '#';
        map[height - 1][i] = '#';
    }

    // Criação das paredes nas primeiras e últimas colunas
    for (int i = 0; i < height; i++)
    {
        map[i][0] = '#';
        map[i][1] = '#';
        map[i][width - 2] = '#';
        map[i][width - 1] = '#';
    }
}

void generateMap(int width, int height, char **map)
{
    // Preenchimento do restante do mapa com paredes probabilísticas
    srand(time(NULL)); // Inicialização do gerador de números aleatórios
    for (int i = 2; i < height - 2; i++)
    {
        for (int j = 2; j < width - 2; j++)
        {
            int randomValue = rand() % 101; // Gera um número aleatório entre 0 e 100
            if (randomValue <= 40)
            {
                map[i][j] = '#';
            }
        }
    }
}

void updateMap(int width, int height, char **map)
{
    char **tempMap = (char **)malloc(height * sizeof(char *));
    for (int i = 0; i < height; i++)
    {
        tempMap[i] = (char *)malloc(width * sizeof(char));
        memcpy(tempMap[i], map[i], width); // Copia o mapa original para o mapa temporário
    }

    for (int i = 2; i < height - 2; i++)
    {
        for (int j = 2; j < width - 2; j++)
        {
            if (map[i][j] != 'b' && map[i][j] != 'c' && map[i][j] != 'e')
            {
                // Verifica as 9 células ao redor
                int count = 0;
                for (int k = i - 1; k <= i + 1; k++)
                {
                    for (int l = j - 1; l <= j + 1; l++)
                    {
                        if (map[k][l] == '#')
                        {
                            count++;
                        }
                    }
                }
                // Define como parede se tiver 5 ou mais paredes vizinhas
                if (count >= 5)
                {
                    tempMap[i][j] = '#';
                }
                else
                {
                    tempMap[i][j] = '.';
                }
            }
        }
    }
    // Copia o mapa atualizado de volta para o mapa original
    for (int i = 2; i < height - 2; i++)
    {
        for (int j = 2; j < width - 2; j++)
        {
            map[i][j] = tempMap[i][j];
        }
    }

    // Libera a memória do mapa temporário
    for (int i = 0; i < height; i++)
    {
        free(tempMap[i]);
    }
    free(tempMap);
}

bool verificaParedeNaDirecao(double x, double y, char **map, WINDOW *win)
{
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
    {
        if (map[(int)y][(int)x] == '#')
        {
            mvwaddch(win, y, x, map[(int)y][(int)x]);
            return false;
        }
        if (map[(int)y][(int)x] == 'e' || map[(int)y][(int)x] == 'b' || map[(int)y][(int)x] == 'c')
            return false;
    }
    mvwaddch(win, y, x, map[(int)y][(int)x]);
    return true;
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Visao
///////////////////
void desenhaVisao(struct class_obj *player, char **map, WINDOW *win, int visaoAtivada)
{
    int largura = getmaxx(win);
    int altura = getmaxy(win);

    if (visaoAtivada)
    {
        double anguloIncremento = M_PI / 64.0;
        for (double angulo = 0; angulo < 2.0 * M_PI; angulo += anguloIncremento)
        {
            for (int i = 1; i < MAP_RADIUS; i++)
            {
                double x = player->x + i * cos(angulo);
                double y = player->y + i * sin(angulo);
                if (verificaParedeNaDirecao(x, y, map, win))
                {
                    if (map[(int)y][(int)x] != 'e' && map[(int)y][(int)x] != 'b' && map[(int)y][(int)x] != 'c')
                        mvwaddch(win, y, x, map[(int)y][(int)x]);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
    {
        for (int y = 0; y < altura; y++)
        {
            for (int x = 0; x < largura; x++)
            {
                if (map[(int)y][(int)x] != 'e' && map[(int)y][(int)x] != 'b' && map[(int)y][(int)x] != 'c')
                    mvwaddch(win, y, x, map[y][x]);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Main
///////////////////
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
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec wavSpec1;
    Uint32 wavLength1;
    Uint8 *wavBuffer1;
    SDL_AudioSpec wavSpec2;
    Uint32 wavLength2;
    Uint8 *wavBuffer2;
    SDL_AudioSpec wavSpec3;
    Uint32 wavLength3;
    Uint8 *wavBuffer3;
    SDL_AudioSpec wavSpec4;
    Uint32 wavLength4;
    Uint8 *wavBuffer4;
    SDL_LoadWAV("gunsound.wav", &wavSpec1, &wavBuffer1, &wavLength1);
    SDL_AudioDeviceID deviceId1 = SDL_OpenAudioDevice(NULL, 0, &wavSpec1, NULL, 0);
    SDL_LoadWAV("sword.wav", &wavSpec2, &wavBuffer2, &wavLength2);
    SDL_AudioDeviceID deviceId2 = SDL_OpenAudioDevice(NULL, 0, &wavSpec2, NULL, 0);
    SDL_LoadWAV("zombie.wav", &wavSpec3, &wavBuffer3, &wavLength3);
    SDL_AudioDeviceID deviceId3 = SDL_OpenAudioDevice(NULL, 0, &wavSpec3, NULL, 0);
    SDL_LoadWAV("kill.wav", &wavSpec4, &wavBuffer4, &wavLength4);
    SDL_AudioDeviceID deviceId4 = SDL_OpenAudioDevice(NULL, 0, &wavSpec4, NULL, 0);

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
            init_pair(1, COLOR_WHITE, -1);  // Padrão (parede)
            init_pair(2, COLOR_YELLOW, -1); // Jogador
            init_pair(3, COLOR_RED, -1);    // Mobs

            char **map = (char **)malloc(MAP_HEIGHT * sizeof(char *));
            for (int i = 0; i < MAP_HEIGHT; i++)
            {
                map[i] = (char *)malloc(MAP_WIDTH * sizeof(char));
                memset(map[i], '.', MAP_WIDTH); // Preenche a linha com pontos
            }
            addBorder(MAP_WIDTH, MAP_HEIGHT, map);
            generateMap(MAP_WIDTH, MAP_HEIGHT, map);
            init_player(&player, MAP_WIDTH / 2, MAP_HEIGHT / 2, 1, "@");
            int tempo_jogado = 0;  // Inicializa o contador de tempo jogado
            int visaoAtivada = 1;
            int move_Count = 4;
            int m = 0;

            int c;
            do
            {
                // Limpa a janela
                werase(win);
                wattron(win, COLOR_PAIR(1));
                // moveAllMobs(&player, map, win);
                desenhaMobs(map, win);

                // Desenha a visão do jogador
                wattron(win, COLOR_PAIR(1));
                desenhaVisao(&player, map, win, visaoAtivada);
                wattroff(win, COLOR_PAIR(1));
                // Desenha o jogador na posição atual
                wattron(win, COLOR_PAIR(2));
                mvwprintw(win, player.y, player.x, "@");
                wattroff(win, COLOR_PAIR(2));
                if (move_Count == 5)
                {
                    init_mob(&mob, map, &player);
                    move_Count = 0;
                }

                // Atualiza a janela
                wrefresh(win);

                tempo_jogado++;


                // Espera pela entrada do usuário
                c = wgetch(win);
                // Limpa a posição atual do jogador no mapa
                map[player.y][player.x] = '.';
                // Atualiza a posição do jogador com base na entrada do usuário
                switch (c)
                {
                case KEY_UP:
                    if (player.y > 2 && map[player.y - 1][player.x] != '#')
                    {
                        player.y--;
                        move_Count++;
                        player.direction = 1;
                        if (mobProx(player.x, player.y, map, MAP_RADIUS) == true)
                        {
                            SDL_QueueAudio(deviceId3, wavBuffer3, wavLength3);
                            SDL_PauseAudioDevice(deviceId3, 0);
                        }
                        m++;
                    }
                    break;
                case KEY_DOWN:
                    if (player.y < MAP_HEIGHT - 3 && map[player.y + 1][player.x] != '#')
                    {
                        player.y++;
                        move_Count++;
                        player.direction = 2;
                        if (mobProx(player.x, player.y, map, MAP_RADIUS) == true)
                        {
                            SDL_QueueAudio(deviceId3, wavBuffer3, wavLength3);
                            SDL_PauseAudioDevice(deviceId3, 0);
                        }
                        m++;

                    }
                    break;
                case KEY_LEFT:
                    if (player.x > 2 && map[player.y][player.x - 1] != '#')
                    {
                        player.x--;
                        move_Count++;
                        player.direction = 3;
                        if (mobProx(player.x, player.y, map, MAP_RADIUS) == true)
                        {
                            SDL_QueueAudio(deviceId3, wavBuffer3, wavLength3);
                            SDL_PauseAudioDevice(deviceId3, 0);
                        }
                        m++;

                    }
                    break;
                case KEY_RIGHT:
                    if (player.x < MAP_WIDTH - 3 && map[player.y][player.x + 1] != '#')
                    {
                        player.x++;
                        move_Count++;
                        player.direction = 4;
                        if (mobProx(player.x, player.y, map, MAP_RADIUS) == true)
                        {
                            SDL_QueueAudio(deviceId3, wavBuffer3, wavLength3);
                            SDL_PauseAudioDevice(deviceId3, 0);
                        }
                        m++;

                    }
                    break;
                case 'p':
                    updateMap(MAP_WIDTH, MAP_HEIGHT, map);
                    break;
                case 't':
                    visaoAtivada = !visaoAtivada;
                    break;
                case 'a':
                    if (ataque_melee(player.x, player.y, player.direction, map, win) == true)
                    {
                        SDL_QueueAudio(deviceId4, wavBuffer4, wavLength4);
                        SDL_PauseAudioDevice(deviceId4, 0);
                    }
                    else
                    {
                        SDL_QueueAudio(deviceId2, wavBuffer2, wavLength2);
                        SDL_PauseAudioDevice(deviceId2, 0);
                    }
                    if (mobProx(player.x, player.y, map, MAP_RADIUS) == true)
                    {
                        SDL_QueueAudio(deviceId3, wavBuffer3, wavLength3);
                        SDL_PauseAudioDevice(deviceId3, 0);
                    }
                    break;
                case 's':
                    ataque_ranged(player.x, player.y, player.direction, map, win);
                    //desenha_bala(&player, map, win);
                    SDL_QueueAudio(deviceId1, wavBuffer1, wavLength1);
                    SDL_PauseAudioDevice(deviceId1, 0);
                    //tiro(&player, map, win);
                    if (mobProx(player.x, player.y, map, MAP_RADIUS) == true)
                    {
                        SDL_QueueAudio(deviceId3, wavBuffer3, wavLength3);
                        SDL_PauseAudioDevice(deviceId3, 0);
                    }
                    break;
                case 'l':
                    limpa_mortos(map, win);break;
                case 'q':
                    endwin();
                    return 0;
                default:
                    break;
                }

                // Verifica colisão com mobs
                if (map[player.y][player.x] == 'e' || map[player.y][player.x] == 'b' || map[player.y][player.x] == 'c')
                {
                    life--;
                    game_over(monstrosMortos, tempo_jogado, m);
                }

                // Atualiza a posição atual do jogador no mapa
                map[player.y][player.x] = '@';
                //moveAllMobs(&player, map);
            } while (c != 'q' && life > 0);
            // Finaliza a biblioteca ncurses

            endwin();

            // Libera a memória do mapa
            for (int i = 0; i < MAP_HEIGHT; i++)
            {
                free(map[i]);
            }
            endwin();
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
    return 0;
}