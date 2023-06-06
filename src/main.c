#include <ncurses.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
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

void
game_over (int monstrosMortos, int tempo_jogado, int m)
{
  // Limpa a tela
  clear ();

  // ObtC)m as dimensC5es da janela
  int largura, altura;
  getmaxyx (stdscr, altura, largura);

  int score = tempo_jogado + monstrosMortos * 50;

  // Define as coordenadas para exibir o texto de game over centralizado
  int x = largura / 2 - 8;
  int y = altura / 2;

  // Imprime o texto de game over na tela
  mvprintw (y, x, "GAME OVER");

  // Exibe as estatC-sticas
  mvprintw (y + 2, x, "Monstros Mortos: %d", monstrosMortos);
  mvprintw (y + 4, x, "Tempo Jogado: %02d:%02d:%02d", tempo_jogado / 3600,
	    (tempo_jogado % 3600) / 60, tempo_jogado % 60);
  mvprintw (y + 6, x, "Movimentos: %d", m);
  mvprintw (y + 8, x, "Score: %d", score);


  // Atualiza a tela
  refresh ();

  // Pausa o programa por 5 segundos antes de encerrar
  sleep (5);
  // depois dos 5 segundos, o jogo recomeC'a
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

int
str_len (const char *str)
{
  int tamanho = 0;
  while (*str++)
    ++tamanho;
  return tamanho;
}

int altura_logo = sizeof (menu_logo) / sizeof (menu_logo[0]);

int
calcula_largura_logo ()
{
  int largura_logo = 1;
  for (int i = 0; i < altura_logo; i++)
    {
      int aux = str_len (menu_logo[i]);
      if (aux > largura_logo)
	{
	  largura_logo = str_len (menu_logo[i]);
	}
    }
  return largura_logo;
}

int largura_logo = 1;
void
desenha_logo (int largura)
{
  // Calcula largura
  if (largura_logo == 1)
    {
      largura_logo = calcula_largura_logo () / 2;
    }

  // Desenha
  attron (COLOR_PAIR (c_hud));
  for (int i = 0; i < altura_logo; i++)
    {
      mvprintw (3 + i, largura / 2 - largura_logo, "%s", menu_logo[i]);
    }
  attroff (COLOR_PAIR (c_hud));
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Player
///////////////////
void
init_player (struct class_obj *player, int x, int y, int dir, char *symbol)
{
  player->x = x;
  player->y = y;
  player->direction = dir;
  strcpy (player->symbol, symbol);
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Mob
///////////////////
void
init_mob (struct class_obj *mob, char **map, struct class_obj *player)
{
  int mobX, mobY;
  do
    {
      mobX = rand () % (MAP_WIDTH - 4) + 2;
      mobY = rand () % (MAP_HEIGHT - 4) + 2;
    }
  while (map[mobY][mobX] != '.'
	 || (abs (mobX - player->x) <= MAP_RADIUS
	     && abs (mobY - player->y) <= MAP_RADIUS));
  // procura uma posiC'C#o vC!lida para colocar o mob
  int value = rand () % 3 + 1;	// escolhe aleatoriamente qual o mob que vai ser escolhido
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

int
calculaDistancia (struct class_obj *obj, int x, int y)
{
  int distX = abs (obj->x - x);
  int distY = abs (obj->y - y);
  return distX + distY;
}

void
moveMob (struct class_obj *player, char **map, int mobx, int moby)
{
  int closestDist = INT_MAX;
  int closestX = -1;
  int closestY = -1;
  char temp = map[moby][mobx];

  // Coordenadas das posiC'C5es acima, abaixo, C  esquerda e C  direita
  int positions[][2] = {
    {moby - 1, mobx},		// Acima
    {moby + 1, mobx},		// Abaixo
    {moby, mobx - 1},		// Esquerda
    {moby, mobx + 1}		// Direita
  };

  // Percorre as posiC'C5es adjacentes ao mob
  for (int i = 0; i < 4; i++)
    {
      int y = positions[i][0];
      int x = positions[i][1];

      // Verifica se a posiC'C#o C) vC!lida e se estC! vazia
      if (y >= 0 && y < MAP_HEIGHT && x >= 0 && x < MAP_WIDTH
	  && map[y][x] == '.')
	{
	  // Calcula a distC"ncia apenas para movimentos horizontais ou verticais
	  if (y == moby || x == mobx)
	    {
	      int dist = calculaDistancia (player, x, y);
	      if (dist < closestDist)
		{
		  closestDist = dist;
		  closestX = x;
		  closestY = y;
		}
	    }
	}
    }

  // Move o mob para a posiC'C#o mais prC3xima do jogador
  if (closestX != -1 && closestY != -1)
    {
      map[moby][mobx] = '.';	// Remove o mob da posiC'C#o atual
      mobx = closestX;		// Atualiza as coordenadas x e y do mob
      moby = closestY;
      map[moby][mobx] = temp;	// Coloca o mob na nova posiC'C#o

      if (player->x == mobx && player->y == moby)
	life--;			// se o mob estiver "em cima" do jogador, decrementa a vida do jogador, ou seja, ele morre e acaba o jogo
    }
}

void
moveAllMobs (struct class_obj *player, char **map)
{				// funC'C#o responsC!vel por mover todos os mobs
  for (int y = 0; y < MAP_HEIGHT; y++)
    {
      for (int x = 0; x < MAP_WIDTH; x++)
	{			// percorre o mapa C  procura de um mob, e quando encontra, chama a funC'C#o que move o mob
	  if (map[y][x] == 'b' || map[y][x] == 'c' || map[y][x] == 'e')
	    {
	      moveMob (player, map, x, y);
	    }
	}
    }
}

void
desenhaMobs (char **map, WINDOW * win)
{
  int largura = getmaxx (win);
  int altura = getmaxy (win);
  for (int y = 0; y < altura; y++)
    {
      for (int x = 0; x < largura; x++)
	{
	  if (map[y][x] == 'e' || map[y][x] == 'b' || map[y][x] == 'c')
	    mvwaddch (win, y, x, map[y][x]);	// coloca no screen o caracter especC-fico de cada mob
	}
    }
}

bool
mobProx (int x, int y, char **mapa, int raio)
{
  int startX = x - raio;
  int endX = x + raio;
  int startY = y + raio;
  int endY = y - raio;

  for (int i = startX; i <= endX; i++)	// percorre a matriz em volta do jogador para saber se existem mobs prC3ximos
    {
      for (int j = startY; j >= endY; j--)
	{
	  // Verifica se as coordenadas estC#o dentro do mapa
	  if (i >= 0 && i < MAP_WIDTH && j >= 0 && j < MAP_HEIGHT)
	    {
	      if (mapa[j][i] == 'b' || mapa[j][i] == 'c' || mapa[j][i] == 'e')
		{
		  // Mob encontrado dentro do raio, ou seja, isto indica que se pode dar queue ao som do mob
		  return true;
		}
	    }
	}
    }

  // Nenhum mob encontrado dentro do raio, ou seja, nC#o se pode dar queue ao som do mob
  return false;
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Combate
///////////////////
bool
ataque_ranged (int px, int py, int dir, char **map, WINDOW * win)
{
  int playerX = px;
  int playerY = py;
  int targetX = -1;
  int targetY = -1;
  int direction = dir;

  switch (direction)
    {
    case 1:			// Cima
      targetX = playerX;
      targetY = playerY - 1;
      break;
    case 2:			// Baixo
      targetX = playerX;
      targetY = playerY + 1;
      break;
    case 3:			// Esquerda
      targetX = playerX - 1;
      targetY = playerY;
      break;
    case 4:			// Direita
      targetX = playerX + 1;
      targetY = playerY;
      break;
    default:
      return false;		// DireC'C#o invC!lida, nC#o faz nada
    }

  char targetBlock = map[targetY][targetX];

  while (targetBlock == '.')	// percorre o mapa enquanto o bloco for '.', ou seja, for vazio
    {
      int prevX = targetX;
      int prevY = targetY;

      map[prevY][prevX] = '.';
      mvwaddch (win, prevY, prevX, map[prevY][prevX]);	// Atualiza a exibiC'C#o na janela

      targetX += (direction == 3) ? -1 : (direction == 4) ? 1 : 0;
      targetY += (direction == 1) ? -1 : (direction == 2) ? 1 : 0;

      targetBlock = map[targetY][targetX];

      if (targetBlock == '.')
	{
	  map[targetY][targetX] = '*';
	  mvwaddch (win, targetY, targetX, map[targetY][targetX]);	// Atualiza a exibiC'C#o na janela
	}
    }

  if (targetBlock == '#')
    {
      // Bloco sC3lido, nC#o faz nada
      return false;
    }
  else if (targetBlock == 'b' || targetBlock == 'e' || targetBlock == 'c')
    {
      map[targetY][targetX] = '*';
      mvwaddch (win, targetY, targetX, map[targetY][targetX]);	// Atualiza a exibiC'C#o na janela

      monstrosMortos++;
      return true;
    }

  return false;
}

bool
ataque_melee (int px, int py, int dir, char **map, WINDOW * win)
{
  int playerX = px;
  int playerY = py;
  int targetX = -1;
  int targetY = -1;
  int direction = dir;

  switch (direction)		// verifica para que direC'C#o o jogador se encontra virado e atualiza guarda a posiC'C#o na direC'C#o em frente ao jogador
    {
    case 1:			// Cima
      targetX = playerX;
      targetY = playerY - 1;
      break;
    case 2:			// Baixo
      targetX = playerX;
      targetY = playerY + 1;
      break;
    case 3:			// Esquerda
      targetX = playerX - 1;
      targetY = playerY;
      break;
    case 4:			// Direita
      targetX = playerX + 1;
      targetY = playerY;
      break;
    default:
      return false;		// DireC'C#o invC!lida, melee attack nC#o faz nada
    }

  char targetBlock = map[targetY][targetX];	// guarda a posiC'C#o onde se vai dar o ataque

  if (targetBlock == '#')
    {
      // Bloco sC3lido, melee attack nC#o faz nada
      return false;
    }
  else if (targetBlock == 'b' || targetBlock == 'e' || targetBlock == 'c')
    {
      map[targetY][targetX] = '*';	// coloca na posiC'C#o do mob o '*' que simboliza o cadC!ver do morto
      mvwaddch (win, targetY, targetX, map[targetY][targetX]);	// coloca na janela o '*'
      monstrosMortos++;		// incrementa o nC:mero de mortos pelo utilizador
      return true;		// retorna true, o que significa que o jogador matou um mob, o que C) necessC!rio para saber qual som emitir
    }

  return false;			// retorna false se nC#o matou mob nenhum
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Mapa
///////////////////

void
limpa_mortos (char **mapa, WINDOW * win)
{
  for (int i = 0; i < MAP_HEIGHT; i++)
    {
      for (int j = 0; j < MAP_WIDTH; j++)
	{
	  if (mapa[i][j] == '*')
	    {
	      mapa[i][j] = '.';
	      mvwaddch (win, i, j, '.');	// Atualiza a exibiC'C#o na janela
	    }
	}
    }
}


void
addBorder (int width, int height, char **map)
{
  for (int i = 0; i < width; i++)
    {
      map[0][i] = '#';
      map[1][i] = '#';
      map[height - 2][i] = '#';
      map[height - 1][i] = '#';
    }

  // CriaC'C#o das paredes nas primeiras e C:ltimas colunas
  for (int i = 0; i < height; i++)
    {
      map[i][0] = '#';
      map[i][1] = '#';
      map[i][width - 2] = '#';
      map[i][width - 1] = '#';
    }
}

void
generateMap (int width, int height, char **map)
{
  srand (time (NULL));		// InicializaC'C#o do gerador de nC:meros aleatC3rios
  for (int i = 2; i < height - 2; i++)
    {
      for (int j = 2; j < width - 2; j++)
	{
	  int randomValue = rand () % 101;	// Gera um nC:mero aleatC3rio entre 0 e 100
	  if (randomValue <= 40)
	    {
	      map[i][j] = '#';
	    }
	}
    }
}

void
updateMap (int width, int height, char **map)
{
  char **tempMap = (char **) malloc (height * sizeof (char *));	// cria um mapa temporC!rio
  for (int i = 0; i < height; i++)
    {
      tempMap[i] = (char *) malloc (width * sizeof (char));
      memcpy (tempMap[i], map[i], width);	// Copia o mapa original para o mapa temporC!rio
    }

  for (int i = 2; i < height - 2; i++)
    {
      for (int j = 2; j < width - 2; j++)
	{
	  if (map[i][j] != 'b' && map[i][j] != 'c' && map[i][j] != 'e')	// nC#o pode alterar as poiC'C5es dos mobs
	    {
	      // Verifica as 9 cC)lulas ao redor
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

  // Liberta a memC3ria do mapa temporC!rio
  for (int i = 0; i < height; i++)
    {
      free (tempMap[i]);
    }
  free (tempMap);
}

bool
verificaParedeNaDirecao (double x, double y, char **map, WINDOW * win)
{
  if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)	// verifica se as coordenadas estC#o dentro dos limites do mapa
    {
      if (map[(int) y][(int) x] == '#')	// se tiver uma parede em frente, pinta a parede e retorna false
	{
	  mvwaddch (win, y, x, map[(int) y][(int) x]);
	  return false;
	}
      if (map[(int) y][(int) x] == 'e' || map[(int) y][(int) x] == 'b'
	  || map[(int) y][(int) x] == 'c')
	return false;		// se tiver um mob em frente, retorna sC3 false
    }
  mvwaddch (win, y, x, map[(int) y][(int) x]);	// pinta a posiC'C#o e retorna true se nC#o encontrar obstC!culos
  return true;
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Visao
///////////////////
void
desenhaVisao (struct class_obj *player, char **map, WINDOW * win,
	      int visaoAtivada)
{
  int largura = getmaxx (win);
  int altura = getmaxy (win);

  if (visaoAtivada)		// se visC#o estiver ativada, desenha o campo de visC#o do jogador
    {
      double anguloIncremento = M_PI / 64.0;
      for (double angulo = 0; angulo < 2.0 * M_PI; angulo += anguloIncremento)	// percorre todos os angulos de 0 atC) 2*pi
	{
	  for (int i = 1; i < MAP_RADIUS; i++)	// percorre para cada C"ngulo, o raio da visC#o
	    {
	      double x = player->x + i * cos (angulo);	// atualiza a posiC'C#o x do jogador
	      double y = player->y + i * sin (angulo);	// atualiza a posiC'C#o x do jogador
	      if (verificaParedeNaDirecao (x, y, map, win))	// verifica se tem obstC!culos em frente
		{
		  if (map[(int) y][(int) x] != 'e'
		      && map[(int) y][(int) x] != 'b'
		      && map[(int) y][(int) x] != 'c')
		    mvwaddch (win, y, x, map[(int) y][(int) x]);	// se nC#o tiver mobs C  frente, "pinta" a posiC'C#o
		}
	      else
		{
		  break;
		}
	    }
	}
    }
  else				// senC#o estiver atuvada, desenha o mapa todo
    {
      for (int y = 0; y < altura; y++)
	{
	  for (int x = 0; x < largura; x++)
	    {
	      if (map[(int) y][(int) x] != 'e' && map[(int) y][(int) x] != 'b'
		  && map[(int) y][(int) x] != 'c')
		mvwaddch (win, y, x, map[y][x]);
	    }			// sC3 C) responsC!vel por desenhar as paredes do mapa
	}
    }
}

/////////////////////////////////////////////////////////////////////////
////////////////////
// Main
///////////////////
int
main ()
{
  // Inicializa a libraria ncurses
  initscr ();
  raw ();			// processamento dos inputs sem utilizar buffer
  keypad (stdscr, TRUE);	// habilita a utilizaC'C#o das setas
  savetty ();			// salva as configuraC'C5es atuais do terminal
  cbreak ();			// processamento das teclas especiais(ctrl+C) etc.
  noecho ();			// impede o print dos inputs do utilizador
  timeout (0);			// faz com que o programa corra independentemente do input do utilizador
  leaveok (stdscr, TRUE);	//garante que temos o controlo total da posiC'C#o na tela 
  curs_set (0);			// oculta o cursor na tela
  SDL_Init (SDL_INIT_AUDIO);	// inicializa a libraria SDL responsC!vel pelo audio 
  SDL_AudioSpec wavSpec1;	// armazena as especificaC'C5es do ficheiro de som
  Uint32 wavLength1;		// armazena o tamanho do buffer em bytes
  Uint8 *wavBuffer1;		// ponteiro para o buffer
  SDL_AudioSpec wavSpec2;
  Uint32 wavLength2;
  Uint8 *wavBuffer2;
  SDL_AudioSpec wavSpec3;
  Uint32 wavLength3;
  Uint8 *wavBuffer3;
  SDL_AudioSpec wavSpec4;
  Uint32 wavLength4;
  Uint8 *wavBuffer4;
  SDL_LoadWAV ("gunsound.wav", &wavSpec1, &wavBuffer1, &wavLength1);	// carrega o ficheiro de som
  SDL_AudioDeviceID deviceId1 = SDL_OpenAudioDevice (NULL, 0, &wavSpec1, NULL, 0);	// abre o ficheiro para utilizaC'C#o
  SDL_LoadWAV ("sword.wav", &wavSpec2, &wavBuffer2, &wavLength2);
  SDL_AudioDeviceID deviceId2 =
    SDL_OpenAudioDevice (NULL, 0, &wavSpec2, NULL, 0);
  SDL_LoadWAV ("zombie.wav", &wavSpec3, &wavBuffer3, &wavLength3);
  SDL_AudioDeviceID deviceId3 =
    SDL_OpenAudioDevice (NULL, 0, &wavSpec3, NULL, 0);
  SDL_LoadWAV ("kill.wav", &wavSpec4, &wavBuffer4, &wavLength4);
  SDL_AudioDeviceID deviceId4 =
    SDL_OpenAudioDevice (NULL, 0, &wavSpec4, NULL, 0);
  WINDOW *win;

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
      getmaxyx (stdscr, altura, largura);

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
	  desenha_logo (largura);

	  int select_start_game = item_menu == 0 ? 0 : 1;
	  mvprintw (altura / 2 + 3,
		    largura / 2 -
		    str_len (item_start_game[select_start_game]) / 2, "%s",
		    item_start_game[select_start_game]);

	  int select_exit = item_menu == 1 ? 0 : 1;
	  mvprintw (altura / 2 + 5,
		    largura / 2 - str_len (item_exit[select_exit]) / 2, "%s",
		    item_exit[select_exit]);

	  mvprintw (altura - 5, largura / 2 - str_len ("DEVELOPERS") / 2,
		    "DEVELOPERS");

	  mvprintw (altura - 3,
		    largura / 2 -
		    str_len
		    (" > JoC#o Veloso   > GonC'alo GonC'alves     > Pedro Figueiredo   > Rui Ribeiro")
		    / 2,
		    " > JoC#o Veloso   > GonC'alo GonC'alves     > Pedro Figueiredo   > Rui Ribeiro ");

	  // Desenha caixa em volta do menu
	  attron (COLOR_PAIR (c_hud));
	  box (stdscr, 0, 0);
	  attron (COLOR_PAIR (c_hud));

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
	  win = newwin (MAP_HEIGHT, MAP_WIDTH, 0, 0);

	  // Habilita o uso de teclas especiais (como as setas do teclado)
	  keypad (win, TRUE);

	  // Oculta a exibiC'C#o do cursor
	  curs_set (0);

	  // Configura as cores para usar o padrC#o do terminal
	  start_color ();
	  use_default_colors ();

	  // Define as cores para a parede e o jogador
	  init_pair (1, COLOR_WHITE, -1);	// PadrC#o
	  init_pair (2, COLOR_YELLOW, -1);	// Jogador
	  // alocaC'C#o do mapa em memC3ria
	  char **map = (char **) malloc (MAP_HEIGHT * sizeof (char *));	// cria *altura* array de apontadores
	  for (int i = 0; i < MAP_HEIGHT; i++)
	    {
	      map[i] = (char *) malloc (MAP_WIDTH * sizeof (char));	// cria *largura* array de chars '.'
	      memset (map[i], '.', MAP_WIDTH);	// Preenche a linha com pontos
	    }
	  addBorder (MAP_WIDTH, MAP_HEIGHT, map);	// adiciona a borda ao mapa
	  generateMap (MAP_WIDTH, MAP_HEIGHT, map);	// gera o mapa
	  init_player (&player, MAP_WIDTH / 2, MAP_HEIGHT / 2, 1, "@");	// inicializa o jogador no ponto central do mapa
	  int tempo_jogado = 0;	// Inicializa o contador de tempo jogado
	  int visaoAtivada = 1;	// valor que determina se o jogador tem a visC#o ativada ou nC#o
	  int move_Count = 4;	// variC!vel que define se vai spawnar um mob ou nC#o
	  int m = 0;		// nC:mero de moves feitos pelo jogador

	  int c;		// variC!vel que vai ter o input do utilizador
	  do
	    {
	      // Limpa a janela
	      werase (win);
	      wattron (win, COLOR_PAIR (1));	// liga o par de cores 1
	      moveAllMobs (&player, map);	// move todos os mobs do mapa
	      desenhaMobs (map, win);	// desenha todos os mobs nas suas novas posiC'C5es
	      // Desenha a visC#o do jogador
	      desenhaVisao (&player, map, win, visaoAtivada);
	      wattroff (win, COLOR_PAIR (1));	// desliga o par de cores 1
	      // Desenha o jogador na posiC'C#o atual
	      wattron (win, COLOR_PAIR (2));	// liga o par de cores para desenhar o jogador
	      mvwprintw (win, player.y, player.x, "@");	// desenha o jogador nas coordenadas
	      wattroff (win, COLOR_PAIR (2));	// desliga o par de cores do jogador
	      if (move_Count == 5)	// verifica se o move_count C) 5 para spawnar o mob
		{
		  init_mob (&mob, map, &player);	// chama a funC'C#o que spawna o mob
		  move_Count = 0;
		}

	      // Atualiza a janela
	      wrefresh (win);

	      tempo_jogado++;	// aumenta o tempo jogado


	      // Espera pela entrada do usuC!rio
	      c = wgetch (win);
	      // Limpa a posiC'C#o atual do jogador no mapa
	      map[player.y][player.x] = '.';
	      // Atualiza a posiC'C#o do jogador com base na entrada do usuC!rio
	      switch (c)
		{
		case KEY_UP:
		  if (player.y > 2 && map[player.y - 1][player.x] != '#')
		    {
		      player.y--;	// sobe o jogador
		      move_Count++;	// incrementa o move_count
		      player.direction = 1;	// atualiza a direC'C#o do jogador
		      if (mobProx (player.x, player.y, map, MAP_RADIUS) ==
			  true)
			{
			  SDL_QueueAudio (deviceId3, wavBuffer3, wavLength3);
			  SDL_PauseAudioDevice (deviceId3, 0);
			}	// os mobs sC3 fazem barulho se estiverem no campo de visC#o do jogador
		      m++;	// incrementa o numero de moves realizado pelo jogador
		    }
		  break;
		case KEY_DOWN:
		  if (player.y < MAP_HEIGHT - 3
		      && map[player.y + 1][player.x] != '#')
		    {
		      player.y++;
		      move_Count++;
		      player.direction = 2;
		      if (mobProx (player.x, player.y, map, MAP_RADIUS) ==
			  true)
			{
			  SDL_QueueAudio (deviceId3, wavBuffer3, wavLength3);
			  SDL_PauseAudioDevice (deviceId3, 0);
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
		      if (mobProx (player.x, player.y, map, MAP_RADIUS) ==
			  true)
			{
			  SDL_QueueAudio (deviceId3, wavBuffer3, wavLength3);
			  SDL_PauseAudioDevice (deviceId3, 0);
			}
		      m++;

		    }
		  break;
		case KEY_RIGHT:
		  if (player.x < MAP_WIDTH - 3
		      && map[player.y][player.x + 1] != '#')
		    {
		      player.x++;
		      move_Count++;
		      player.direction = 4;
		      if (mobProx (player.x, player.y, map, MAP_RADIUS) ==
			  true)
			{
			  SDL_QueueAudio (deviceId3, wavBuffer3, wavLength3);
			  SDL_PauseAudioDevice (deviceId3, 0);
			}
		      m++;

		    }
		  break;
		case 'p':
		  updateMap (MAP_WIDTH, MAP_HEIGHT, map);	// dC! update ao mapa
		  break;
		case 't':
		  visaoAtivada = !visaoAtivada;	// ativa ou desativa a visC#o
		  break;
		case 'a':
		  if (ataque_melee (player.x, player.y, player.direction, map, win) == true)	// devolve true se matou
		    {
		      SDL_QueueAudio (deviceId4, wavBuffer4, wavLength4);
		      SDL_PauseAudioDevice (deviceId4, 0);
		    }		// se matou dC! queue ao som de matar
		  else		// se nC#o matou dC! queue ao som default
		    {
		      SDL_QueueAudio (deviceId2, wavBuffer2, wavLength2);	// coloca na fila os dados ara reproduzir o som
		      SDL_PauseAudioDevice (deviceId2, 0);	// comeC'a a reproduC'C#o do som
		    }
		  if (mobProx (player.x, player.y, map, MAP_RADIUS) == true)
		    {
		      SDL_QueueAudio (deviceId3, wavBuffer3, wavLength3);
		      SDL_PauseAudioDevice (deviceId3, 0);
		    }
		  break;
		case 's':
		  ataque_ranged (player.x, player.y, player.direction, map,
				 win);
		  SDL_QueueAudio (deviceId1, wavBuffer1, wavLength1);
		  SDL_PauseAudioDevice (deviceId1, 0);
		  if (mobProx (player.x, player.y, map, MAP_RADIUS) == true)
		    {
		      SDL_QueueAudio (deviceId3, wavBuffer3, wavLength3);
		      SDL_PauseAudioDevice (deviceId3, 0);
		    }
		  break;
		case 'l':
		  limpa_mortos (map, win);
		  break;	// remove os corpos dos mobs mortos
		case 'q':	// sai do jogo
		  endwin ();
		  return 0;
		default:
		  break;
		}

	      // Verifica colisC#o com mobs
	      if (map[player.y][player.x] == 'e'
		  || map[player.y][player.x] == 'b'
		  || map[player.y][player.x] == 'c')
		{		// se o jogador colidir com um mob, o jogo acaba e vai para o end screen
		  life--;
		  game_over (monstrosMortos, tempo_jogado, m);
		}

	      // Atualiza a posiC'C#o atual do jogador no mapa
	      map[player.y][player.x] = '@';
	    }
	  while (c != 'q' && life > 0);
	  // Finaliza a biblioteca ncurses

	  endwin ();

	  // Liberta a memC3ria do mapa
	  for (int i = 0; i < MAP_HEIGHT; i++)
	    {
	      free (map[i]);
	    }
	  endwin ();
	  break;

	  // Sair
	case ESTADO_EXIT:
	  endwin ();
	  EXIT = TRUE;
	  break;
	}

      // Sair para o menu
      if (tecla_pressionada == 'q' || tecla_pressionada == 'Q')
	estado_atual = ESTADO_MENU;

      // Ler tecla pressionada
      tecla_pressionada = wgetch (stdscr);
      napms (50);
      tecla_pressionada = wgetch (stdscr);

      // dC! clear da tela
      erase ();
    }
  return 0;
}
