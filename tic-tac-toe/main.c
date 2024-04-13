#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <sys/wait.h>
#include <sys/shm.h>

char getnextchar(void) {
    char inp;
    int n = 0, c = 1;
    static int f = 0;
    if (f == 0)
        n = 1;
    f = 1;
    while (n == 0 || c == 1) {
        if (n == 0)
            n = inp == '\n';    
        inp = getchar();
        if (n == 1 && inp != '\n')
            c = 0;
    }
    return inp;
}

void game_print_board(char *buf) {
    puts("-------");
    for (char i = 0; i < 7; i++) {
        printf(i % 2 == 0 ? "|" : "%c", buf[i/2+1]);
    }
    puts("");
    puts("-------");
    for (char i = 0; i < 7; i++) {
        printf(i % 2 == 0 ? "|" : "%c", buf[i/2+4]);
    }
    puts("");
    puts("-------");
    for (char i = 0; i < 7; i++) {
        printf(i % 2 == 0 ? "|" : "%c", buf[i/2+7]);
    }
    puts("");
    puts("-------");
}

void game_clear_board(char *buf) {
    for (int i = 1; i < 10; i++)
        buf[i] = ' ';
}

void game_print_rules(char *buf) {
    for (int i = 0; i < 9; i++)
        buf[i+1] = '0' + i;
    game_print_board(buf);
}


void game_link(int player, char *buf) {
    while (!((buf[0] >> 1) & 1) || !((buf[0] >> 2) & 1)) {
        puts("Waiting for another player");
        if (fork() == 0) {
            execlp("sleep", "sleep", "5", NULL);
            exit(0);
        }
        wait(NULL);
    }
    buf[0] |= 1 << (player + 3);
    puts("Setting up game");
    while (!((buf[0] >> 3) & 1) || !((buf[0] >> 4) & 1))
        ;
}

void game_wait(int player, char *buf) {
    while ((buf[0] & 1) != player)
        ;
}

int game_winner(char *buf) {
    for (int i = 0; i < 3; i++) {
        if (buf[i*3+1] != ' ' && buf[i*3+1] == buf[i*3+2] && buf[i*3+2] == buf[i*3+3])
            return buf[i*3+1] == 'X' ? 1 : 2;
        if (buf[i+1] != ' ' && buf[i+1] == buf[i+4] && buf[i+4] == buf[i+7])
            return buf[i*3+1] == 'X' ? 1 : 2;
    }
    if (buf[1] != ' ' && buf[1] == buf[5] && buf[5] == buf[9])
        return buf[1] == 'X' ? 1 : 2;
    if (buf[3] != ' ' && buf[3] == buf[5] && buf[5] == buf[7])
        return buf[1] == 'X' ? 1 : 2;
    return 0;
}

int game_draw(char *buf) {
    for (int i = 0; i < 9; i++)
        if (buf[i+1] == ' ')
            return 0;
    return 1;
}

void game_loop(int player, char *buf) {
    char inp;
    buf[0] |= 1 << (player + 1);
    game_print_rules(buf);
    game_link(player, buf);
    game_clear_board(buf);
    while (inp != 'q') {
        int winner;
        if (winner = game_winner(buf)) {
            puts("You win!");
            buf[0] |= 1 << 5;
            return;
        }
        if (game_draw(buf)) {
            puts("It's a draw!");
            return;
        }
        game_wait(player, buf);
        game_print_board(buf);
        puts("Your turn");
        do { 
            inp = getnextchar(); 
            if (inp == 'q')
                break;
            if (inp - '0' < 9 && inp - '0' >= 0 && buf[inp - '0' + 1] == ' ')
                break;
            if (game_draw(buf)) {
                puts("It's a draw!");
                return;
            }
        } while (1);
        if ((buf[0] >> 5) & 1) {
            puts("You lose!");
            return;
        }
        buf[inp - '0' + 1] = player == 0 ? 'X' : 'O';
        buf[0] ^= 1;
    }
}

void game_write(int player, int shmid) {
    FILE *fptr = fopen("status.bin", "wb");
    fputc(player, fptr);
    for (int i = 0; i < sizeof(int) * 8; i += 8)
        fputc(shmid >> i, fptr);
    fclose(fptr);
}

void game(int status, int shmid) {
    char *buf;
    switch (status) {
        case -1:
            shmid = shmget(IPC_PRIVATE, 10 * sizeof(char), IPC_CREAT | 0666);
            if (shmid == -1) {
                puts("Couldn't get valid shared memory segment");
                exit(1);
            }
            buf = shmat(shmid, NULL, 0);
            if (buf == (void*)-1) {
                puts("Couldn't attach shared memory segment");
                exit(2);
            }
            buf[0] = 0;
            game_write(0, shmid);
            game_loop(0, buf);
            if (shmdt(buf) == -1) {
                puts("Couldn't detach shared memory segment");
                exit(3);
            }
            if (shmctl(shmid, IPC_RMID, NULL) == -1) {
                puts("Couldn't destroy shared memory segment");
                exit(4);
            }
            remove("status.bin");
            break;
        case 0:
            buf = shmat(shmid, NULL, 0);
            if (buf == (void*)-1) {
                puts("Couldn't attach shared memory segment");
                exit(2);
            }
            game_write(1, shmid);
            game_loop(1, buf);
            if (shmdt(buf) == -1) {
                puts("Couldn't detach shared memory segment");
                exit(3);
            }
            break;
        case 1:
            game_write(1, shmid);
            puts("Game is full");
    }
}

int main() {
    FILE *fptr = fopen("status.bin", "rb");
    int status = -1, shmid = 0;
    if (fptr != NULL) {
        status = fgetc(fptr);
        for (int i = 0; i < sizeof(int) * 8; i += 8)
            shmid += fgetc(fptr) << i;
        fclose(fptr);
    }
    game(status, shmid);
    return 0;
}