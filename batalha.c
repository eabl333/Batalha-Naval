/* batalha.c
   Batalha Naval 10x10 (Usuário vs CPU)
   Suporta posições horizontais, verticais e diagonais.
   Compile: gcc -std=c11 -O2 -o batalha batalha.c
   Execute: ./batalha
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define N 10
#define MAX_SHIPS 5

const char *ship_names[MAX_SHIPS] = {"Porta-aviões", "Encouraçado", "Cruzador", "Submarino", "Destroyer"};
const int ship_sizes[MAX_SHIPS] = {5, 4, 3, 3, 2};

/* direções: 0 = horizontal para direita, 1 = vertical para baixo,
   2 = diagonal para baixo-direita, 3 = diagonal para cima-direita */
int dr[4] = {0, 1, 1, -1};
int dc[4] = {1, 0, 1, 1};

/* Inicializa tabuleiro com '~' (água) */
void init_board(char b[N][N]) {
    for (int i=0;i<N;i++) for (int j=0;j<N;j++) b[i][j] = '~';
}

/* Imprime tabuleiro; se hideShips==1, 'S' fica escondido como '~' */
void print_board(char b[N][N], int hideShips) {
    printf("   ");
    for (int j=0;j<N;j++) printf(" %c", 'A'+j);
    printf("\n");
    for (int i=0;i<N;i++) {
        printf("%2d ", i+1);
        for (int j=0;j<N;j++) {
            char ch = b[i][j];
            if (hideShips && ch == 'S') printf(" ~");
            else printf(" %c", ch);
        }
        printf("\n");
    }
}

/* Verifica se é possível colocar um navio de 'size' em (r,c) na direção 'dir' */
int can_place(char b[N][N], int r, int c, int size, int dir) {
    for (int k=0;k<size;k++) {
        int rr = r + dr[dir]*k;
        int cc = c + dc[dir]*k;
        if (rr < 0 || rr >= N || cc < 0 || cc >= N) return 0;
        if (b[rr][cc] != '~') return 0;
    }
    return 1;
}

/* Coloca navio (marca 'S') */
void place_ship(char b[N][N], int r, int c, int size, int dir) {
    for (int k=0;k<size;k++) {
        int rr = r + dr[dir]*k;
        int cc = c + dc[dir]*k;
        b[rr][cc] = 'S';
    }
}

/* Tenta posicionar aleatoriamente um navio de tamanho 'size' */
void place_ship_random(char b[N][N], int size) {
    int placed = 0;
    int attempts = 0;
    while (!placed && attempts < 1000) {
        int dir = rand() % 4;
        int r = rand() % N;
        int c = rand() % N;
        if (can_place(b,r,c,size,dir)) {
            place_ship(b,r,c,size,dir);
            placed = 1;
        }
        attempts++;
    }
    if (!placed) {
        // fallback: tentar força bruta por todas as posições
        for (int dir=0; dir<4 && !placed; dir++) {
            for (int r=0; r<N && !placed; r++) {
                for (int c=0; c<N && !placed; c++) {
                    if (can_place(b,r,c,size,dir)) {
                        place_ship(b,r,c,size,dir);
                        placed = 1;
                    }
                }
            }
        }
    }
}

/* Conta quantas células de navio ('S') restam */
int count_ships_left(char b[N][N]) {
    int cnt = 0;
    for (int i=0;i<N;i++) for (int j=0;j<N;j++) if (b[i][j] == 'S') cnt++;
    return cnt;
}

/* Converte entrada tipo "A5" para r,c; retorna 1 se ok, 0 se inválido */
int parse_coord(const char *s, int *r, int *c) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) return 0;
    char col = toupper((unsigned char)*s);
    if (!(col >= 'A' && col <= 'J')) return 0;
    s++;
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) return 0;
    int num = 0;
    if (!isdigit((unsigned char)*s)) return 0;
    while (*s && isdigit((unsigned char)*s)) { num = num*10 + (*s - '0'); s++; }
    if (num < 1 || num > 10) return 0;
    *c = col - 'A';
    *r = num - 1;
    return 1;
}

/* Lê uma linha do stdin com fgets e remove newline */
void read_line(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) { buf[0] = '\0'; return; }
    size_t L = strlen(buf);
    if (L > 0 && buf[L-1] == '\n') buf[L-1] = '\0';
}

/* Turno do jogador: retorna 1 se acertou, 0 se errou */
int player_attack(char cpu_board[N][N]) {
    char line[64];
    int r,c;
    while (1) {
        printf("Digite coordenada para atacar (ex: A5): ");
        read_line(line, sizeof(line));
        if (!parse_coord(line, &r, &c)) {
            printf("Entrada inválida. Use letra A-J seguida do número 1-10 (ex: B7).\n");
            continue;
        }
        if (cpu_board[r][c] == 'X' || cpu_board[r][c] == 'o') {
            printf("Você já atirou nessa posição. Escolha outra.\n");
            continue;
        }
        break;
    }
    if (cpu_board[r][c] == 'S') {
        cpu_board[r][c] = 'X';
        printf(">>> Acerto! (%c%d)\n", 'A'+c, r+1);
        return 1;
    } else {
        cpu_board[r][c] = 'o';
        printf(">>> Água. (%c%d)\n", 'A'+c, r+1);
        return 0;
    }
}

/* Turno do CPU: escolhe posição aleatória não atacada */
int cpu_attack(char player_board[N][N]) {
    int r,c;
    do {
        r = rand() % N;
        c = rand() % N;
    } while (player_board[r][c] == 'X' || player_board[r][c] == 'o');
    printf("CPU atira em %c%d ... ", 'A'+c, r+1);
    if (player_board[r][c] == 'S') {
        player_board[r][c] = 'X';
        printf("ACERTO!\n");
        return 1;
    } else {
        player_board[r][c] = 'o';
        printf("água.\n");
        return 0;
    }
}

int main() {
    srand((unsigned) time(NULL));
    char player[N][N], cpu[N][N];
    init_board(player); init_board(cpu);

    printf("=== BATALHA NAVAL (10x10) ===\n");
    printf("Navios disponíveis:\n");
    for (int i=0;i<MAX_SHIPS;i++)
        printf("  %s (%d células)\n", ship_names[i], ship_sizes[i]);
    printf("\nDeseja posicionar seus navios manualmente ou aleatoriamente?\n");
    printf("1) Aleatório  2) Manual\nEscolha (1-2): ");
    char line[64]; read_line(line, sizeof(line));
    int mode = 1;
    if (line[0] == '2') mode = 2;

    /* Posicionamento do jogador */
    for (int s=0;s<MAX_SHIPS;s++) {
        int size = ship_sizes[s];
        if (mode == 1) {
            place_ship_random(player, size);
        } else {
            int placed = 0;
            while (!placed) {
                printf("\nSeu tabuleiro atual:\n");
                print_board(player, 0);
                printf("Posicione %s (%d):\n", ship_names[s], size);
                printf("Digite a coordenada inicial (ex: A5): ");
                read_line(line, sizeof(line));
                int r,c;
                if (!parse_coord(line, &r, &c)) {
                    printf("Coordenada inválida.\n");
                    continue;
                }
                printf("Escolha direção: 1=Horizontal→ 2=Vertical↓ 3=Diagonal↘ 4=Diagonal↗\nDireção: ");
                read_line(line, sizeof(line));
                int dir = 0;
                if (line[0] == '1') dir = 0;
                else if (line[0] == '2') dir = 1;
                else if (line[0] == '3') dir = 2;
                else if (line[0] == '4') dir = 3;
                else { printf("Direção inválida.\n"); continue; }
                if (!can_place(player, r, c, size, dir)) {
                    printf("Não é possível colocar o navio nessa posição/direção (sai do tabuleiro ou sobrepõe).\n");
                    continue;
                }
                place_ship(player, r, c, size, dir);
                placed = 1;
            }
        }
    }

    /* Posicionamento do CPU (aleatório sempre) */
    for (int s=0;s<MAX_SHIPS;s++) {
        place_ship_random(cpu, ship_sizes[s]);
    }

    printf("\nComeça o jogo! Você inicia.\n");

    /* Laço principal do jogo */
    while (1) {
        printf("\n--- Seu tabuleiro ---\n");
        print_board(player, 0);
        printf("\n--- Tabuleiro CPU (oculto) ---\n");
        print_board(cpu, 1);

        /* Jogador ataca */
        player_attack(cpu);
        if (count_ships_left(cpu) == 0) {
            printf("\n=== Parabéns — você afundou todos os navios do CPU! Vitória! ===\n");
            break;
        }

        /* CPU ataca */
        cpu_attack(player);
        if (count_ships_left(player) == 0) {
            printf("\n=== CPU afundou todos os seus navios. Você perdeu. ===\n");
            break;
        }
    }

    printf("\nEstado final — seu tabuleiro:\n");
    print_board(player, 0);
    printf("\nEstado final — tabuleiro CPU (revelado):\n");
    print_board(cpu, 0);
    return 0;
}
