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
//index de l'historique
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
    if (history_index == MAXLI) {
        for (int i = 0; i < MAXLI-1; i++) {
            strcpy(history[i], history[i+1]);
        }
        history_index--;
    }
    //on enregistre la commande dans la derniere case de l'historique
    //et cela en incrementant l'index
    strcpy(history[history_index++], cmd);
}

//fonction pour afficher l'historique
void show_history() {
    //on affiche les commandes en parcourant l'historique
    for (int i = 0; i < history_index; i++) {
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
            break;
        }
            //si la commande est history on affiche l'historique
        if(strcmp(cmd,"history") == 0){
            show_history();
        }else{
            //sinon on l'enregistre dans l'historique et on execute la commande
            save_history(cmd);
            mbash(cmd);
        }
    }
    return 0;
}

