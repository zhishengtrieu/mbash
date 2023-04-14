#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#define MAXLI 2048

char cmd[MAXLI];
char* args[MAXLI];
char history[MAXLI][MAXLI];
int history_len = 0;
int history_index = 0;
char dir[MAXLI];

/**
 * fontion pour gerer les commandes du mbash
*/
void mbash(char* cmd) {
    // On verifie si c'est une commande cd
    if (strncmp(cmd, "cd", 2) == 0) {
        // extraire le chemin du fichier en decalant le pointeur
        char* path = cmd + 3;
        //s'il n'y a pas de chemin on retourne au home de l'user
        if (strcmp(path,"") == 0){
        	path = getenv("HOME");
        }
        //on change de repertoire
        int ret = chdir(path);
        //s'il y a une erreur on l'affiche
        if (ret == -1) {
            perror("chdir");
        }
    } else {
        // si c'est une commande normale
        // On utilise la fonction strtok pour separer la commande de ses arguments
        char *token = strtok(cmd, " ");
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        //on cree un processus fils puisque execvp et execve remplace le processus courant
        int pid = fork();
        if (pid == 0) {
            // on verifie si la commande se termine par "&"
            if (strcmp(args[i-1], "&") == 0) {
                // Supprimer "&" de la liste des arguments
                args[i-1] = NULL;
            }
            //execvp permet de chercher la librerie pour executer la commande
            int ret = execvp(args[0], args);
            //s'il y a une erreur on l'affiche
            if (ret == -1) {
                perror("execvp");
                exit(1);
            }
        } else {
            //le pere attend que le fils se termine sauf si la commande se termine par "&"
            if (strcmp(args[i-1], "&") != 0) {
                wait(NULL);
            }
        }
    }
}

/**
 * fonction pour enregistrer une commande dans l'historique
*/
void save_history(char* cmd) {
    //si l'historique est plein on decale les commandes
    if (history_len == MAXLI) {
        for (int i = 0; i < MAXLI-1; i++) {
            strcpy(history[i], history[i+1]);
        }
        history_len--;
    }
    //on enregistre la commande dans la derniere case de l'historique
    //et cela en incrementant la longueur de l'historique
    strcpy(history[history_len++], cmd);
    //on sauvegarde l'index de la commande courante
    history_index = history_len;
}

/**
 * fonction pour afficher l'historique
*/
void show_history() {
    //on affiche les commandes en parcourant l'historique
    for (int i = 0; i < history_len; i++) {
        printf("%d: %s\n", i+1, history[i]);
    }
}

/**
 * fonction pour exectuter plusieurs mbash dans un pipe
*/
void pipe_mbash(char** commandes){
    //on cree un tableau de descripteur de fichier
    int fd[2];
    //on cree un processus fils
    int pid = fork();
    if (pid == 0) {
        //on cree un pipe
        pipe(fd);
        //on cree un processus fils
        int pid2 = fork();
        if (pid2 == 0) {
            //on ferme l'entree du pipe
            close(fd[0]);
            //on redirige la sortie standard vers l'entree du pipe
            dup2(fd[1], STDOUT_FILENO);
            //on execute la premiere commande
            mbash(commandes[0]);
            //on ferme la sortie du pipe
            close(fd[1]);
        } else {
            //on ferme la sortie du pipe
            close(fd[1]);
            //on redirige l'entree standard vers la sortie du pipe
            dup2(fd[0], STDIN_FILENO);
            //on execute la deuxieme commande
            mbash(commandes[1]);
            //on ferme l'entree du pipe
            close(fd[0]);
        }
        //une fois le pipe terminee on retourne au mbash
        exit(1);  
    } else {
        //le pere attend que le fils se termine
        wait(NULL);
    }
}

void display_path(){
    char* home = getenv("HOME");
    getcwd(dir, sizeof(dir));

    if (strstr(dir, home) != NULL) {
        printf("mbash: ~%s$ ", dir + strlen(home));
    } else {
        printf("mbash: %s$ ", dir);
    }
}

int getch_noblock() {
    int ch;
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

void handle_input() {
    int ch;
    int pos = 0; // Position du curseur dans la ligne de commande

    while (1) {
        ch = getch_noblock();

        if (ch == '\n') {
            putchar('\n');
            cmd[pos] = '\0';
            break;
        }

        if (ch == 27) { // Touche échappement pour les séquences de touches fléchées
            getchar(); // Lire le caractère '[' après le caractère d'échappement
            ch = getchar(); // Lire le caractère de code de la touche fléchée

            switch (ch) {
                case 'A': // up arrow
                    if (history_index > 0) {
                        history_index--;
                    }
                    strcpy(cmd, history[history_index]);
                    int len = strlen(cmd);
                    for (int i = 0; i < len + pos + 3; i++) {
                        putchar('\b');
                    }
                    printf("\rmbash: %s$ %s", dir, cmd);
                    pos = len;
                    break;
                case 'B': // down arrow
                    if (history_index < history_len - 1) {
                        history_index++;
                    } else {
                        history_index = history_len;
                        cmd[0] = '\0';
                    }
                    strcpy(cmd, history[history_index]);
                    int len2 = strlen(cmd);
                    for (int i = 0; i < len2 + pos + 3; i++) {
                        putchar('\b');
                    }
                    printf("\rmbash: %s$ %s", dir, cmd);
                    pos = len2;
                    break;
                case 'C': // right arrow
                    if (pos < strlen(cmd)) {
                        putchar(cmd[pos]);
                        pos++;
                    }
                    break;
                case 'D': // left arrow
                    if (pos > 0) {
                        pos--;
                        putchar('\b');
                    }
                    break;
            }
        } else if (ch == 127 || ch == '\b') { // Suppr ou backspace
            if (pos > 0) {
                for (int i = pos; i < strlen(cmd); i++) {
                    cmd[i - 1] = cmd[i];
                }
                cmd[strlen(cmd) - 1] = '\0';
                pos--;
                putchar('\b');
                printf("%s ", cmd + pos);
                for (int i = pos; i <= strlen(cmd); i++) {
                    putchar('\b');
                }
            }
        } else if (isprint(ch)) {
            for (int i = strlen(cmd); i >= pos; i--) {
                cmd[i + 1] = cmd[i];
            }
            cmd[pos] = ch;
            printf("%s", cmd + pos);
            pos++;
            for (int i = pos; i < strlen(cmd); i++) {
                putchar('\b');
            }
        }
    }
}

/**
 * Le main va stocker les entrers de commandes de l'utilisateur
*/
int main(int argc, char** argv) {
    // Boucle infinie permettant faire fonctionner le programme
    while (1) {
        display_path();

        handle_input();

        if (cmd[0] == '\0') {
            continue;
        }
        
        //si la commande est exit on quitte
        if (strcmp(cmd, "exit") == 0) {
            printf("Au revoir !\n");
            break;
        } else if (strcmp(cmd, "history") == 0) {
            //si c'est history, on affiche l'historique
            show_history();
        } else {
            //sinon on enregistre la commande dans l'historique et on l'execute
            save_history(cmd);
            //on verifie si la commande contient un pipe
            char* commandes[MAXLI];
            int i = 0;
            char *token = strtok(cmd, "|");
            while (token != NULL) {
                commandes[i++] = token;
                token = strtok(NULL, "|");
            }
            commandes[i]= NULL;

            //si on a plus d'une commande, on execute le pipe
            if (i > 1) {
                pipe_mbash(commandes);
            } else {
                //sinon on execute la commande
                mbash(cmd);
            }
        }
    }

    return 0;
}