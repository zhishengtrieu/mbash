<h1 align="center">MBash</h1>

# __Présentation du projet :__

Le projet, réalisé à l’IUT Charlemagne avec Alexandre Noel, consiste à créer un bash personnalisé appelé "mbash" en C. Il inclut des fonctionnalités telles que la gestion des commandes basiques (ls, touch,  pwd, … ) ainsi que leur options (ls -l, gedit file.c &, gcc, …). De plus, la commande history permet d'accéder à l'historique des commandes du mbash. Le programme permet aussi l’affichage du répertoire courant et la gestion des pipes.
Notre mbash utilise la méthode execvp() pour exécuter les commandes reçues car elle cherche la librairie où se situe l'exécutable de la commande contrairement à execve(). Cela remplace le processus courant par un nouveau processus donc nous avons utilisé la fonction fork() pour créer un processus fils dédié à l'exécution. Il gère également le caractère "&" pour lancer des commandes en arrière-plan. 
