#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#define MAXLI 2048
//chaine de caractere pour stocker la commande courante
char cmd[MAXLI];
//tableau de chaine de caractere pour stocker les arguments de la commande courante
char* args[MAXLI];
//tableau de chaine de caractere pour stocker l'historique des commandes
char history[MAXLI][MAXLI];
//longueur de l'historique
int history_len = 0;
//index de l'historique pour parcourir les commandes
int history_index = 0;
//chaine de caractere pour stocker le repertoire courant
char dir[MAXLI];

void mbash(char* cmd) {
    // On vérifie si c'est une commande cd
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
        // On utilise la fonction strtok pour séparer la commande de ses arguments
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
            //execvp permet de chercher la librerie correspondant a la commande 
            execvp(args[0], args);
            //le fils execute la commande
            int ret = execve(args[0], args, NULL);
            //s'il y a une erreur on l'affiche
            if (ret == -1) {
                perror("execve");
                //on tue le fils
                exit(1);
            }
        } else {
            //le pere attend que le fils se termine
            wait(NULL);
        }
    }
}

//fonction pour enregistrer une commande dans l'historique
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
    //on 
}

//fonction pour afficher l'historique
void show_history() {
    //on affiche les commandes en parcourant l'historique
    for (int i = 0; i < history_len; i++) {
        printf("%d: %s\n", i+1, history[i]);
    }
}

// Le main va stocker les entrers de commandes de l'utilisateur
int main(int argc, char** argv) {
  // Boucle infinie permettant faire fonctionner le programme
  // Elle peut s'arrêter quand l'utilisateur utilise ctrl-D ou ctrl-C 
    while (1) {
    	//on recupere la racine de l'user courant
    	char * home = getenv("HOME");
    	//on recupere le repertoire courant
    	getcwd(dir, sizeof(dir));
    	//si le repertoire courant contient  tout le home on decale le pointeur du path a afficher
    	if (strstr(dir, home) != NULL){
        	printf("mbash: ~%s$ ", dir+strlen(home));
    	}else{
        	printf("mbash: %s$ ", dir);
        }

        /**
         *  }else if(strcmp(cmd, "\033[A") == 0){
            //si c'est la fleche du haut, on affiche la commande qui precede la commande de l'index
            printf("%s" , history[history_index--]);
        }else if (strcmp(cmd, "\033[B") == 0) {
            //pour la flèche bas, on affiche la commande qui suit la commande de l'index
            history_index++;
            //on verifie que l'index n'est pas superieur a la longueur de l'historique
            if (history_index >= history_len){
                history_index = history_len-1;
            }
            printf("%s" , history[history_index]);
        }else if (strcmp(cmd, "\033[D") == 0) {
            //la flèche gauche, on deplace le curseur vers la gauche
            
        }else if (strcmp(cmd, "\033[C") == 0) {
            // la flèche droite, on deplace le curseur vers la droite
        }else if(strcmp(cmd, "\t") == 0){
            //si c'est la touche tab, on fait de l'autocompletion
            
            //si on en a plus de 1, on affiche les commandes possibles
        */
        int ch;
        while ((ch = getch()) != KEY_F(1)) {
            if (ch == KEY_LEFT) {
                // code pour la flèche gauche
            } else if (ch == KEY_RIGHT) {
                // code pour la flèche droite
            }
        }
        // On attend que l'utilisateur entre une commande bash
    	
        //si on a pas de commande, on quitte (vient du ctrl-D)
        if (fgets(cmd, MAXLI, stdin) == NULL ) {
            printf("\nAu revoir !\n");
            break;
        }
        //supprimer le dernier caractère qui est le retour à la ligne "\n"
        cmd[strcspn(cmd, "\n")] = 0;

        //si la commande est exit on quitte
        if (strcmp(cmd,"exit") == 0){
            printf("Au revoir !\n");
        }else if(strcmp(cmd,"history") == 0){
            //si c'est history, on affiche l'historique
            show_history();
        }else{
            //sinon on enregistre la commande dans l'historique et on l'execute
            save_history(cmd);
            mbash(cmd);
        }
    }
    return 0;
}

