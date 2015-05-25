
#include<stdio.h> /* pour les entree sorties */
#include<stdlib.h> 
#include <string.h>
#include <sys/types.h>  
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <dirent.h> 
#include <time.h> 
#include <utime.h>
#include "functions.h" 


int exist (char* option,char**tab,int n){

  int i=0;
  
  /* pour i allant de 0 (sans option ) à n (nombre d'argument possible pour l'option) */
  while(i<n){
    
    /* si l'option existe dans argv alors retourne vrai */
    if ( strcmp(tab[i++],option) ==0) 
      return 0;
  }
  /* si non retourne faux */
  return -1;
}


/* copieChemin sert à récuperer le chemin du fichier de source et de destination dans buf4 et buf5 */
int copieChemin (char* source, char* dest, char* nomFichierSource, char buf4[BUFFER_SIZE],char buf5[BUFFER_SIZE]){

  
  
  /* on recupere dans buf4 le chemin du repertoire source */
  sprintf(buf4,"%s",source);
  
  /* on lui ajoute (concaten avec strcat) le nom du fichier à copier */
  strcat(buf4,nomFichierSource);
  
  /* on recupere dans buf5 le chemin du repertoire de destination */
  sprintf(buf5,"%s",dest);
  
  /* on lui ajoute le nom du fichier à copier (de source bien sur) */ 
  strcat(buf5,nomFichierSource);
  
  return 0;
} 

/* modifDate comme son nom l'indique modifier la date lors de la synchronisation (pour garder la même date pas celle de l'instant de la synchronisation) */
int modifDate(char*nomFichierSource, char*nomFichierDestination){
  /* stat recupere le statut des fichiers */
  struct stat ss;
  struct stat sd; 
  
  /* on aura besoin aussi de la structure utimbuf pour modifier la date (garder la même date lors de la synchronisation) */
  struct utimbuf date;
  
  /* on récupere le statut du fichier source et destination*/
  stat(nomFichierSource,&ss);
  stat(nomFichierDestination,&sd);
 
  /* on fait en sorte que le fichier garde la même date (date de modification), PAS la date du moment de la synchronisation */ 
  date.modtime = ss.st_mtime;
  
  /* on modifie la date grace a la routine utime () */
  utime(nomFichierDestination,&date);
  
    
  return 0;
}


/* confirmation sert à demander à l'utilisateur une confirmation avant chaque traitement (synchronisation) */
int confirmation (char * nomFichier){

  /* reponse donné par l'utilisatuer */
  char reponse =0;

  printf ("voulez-vous vraiment synchroniser : %s ?\n",nomFichier);
  printf ("(Y/N) ?");
  
  /* récuperer la reponse  */
  scanf("%c",&reponse);
  scanf ("%*[^\n]");
  getchar ();
  /* si c'est oui on fait le traitement naicessaire (synchronisation,copie des fichiers réguliers, copie des liens symboliques) */
  if (reponse =='Y') 
    return 0;
 
  /* si c'est non on fait rien  */
  else if ( reponse =='N')
    return -1;

  else { 
    return confirmation(nomFichier);
  }
 
}



/* copie le fichier de source vers destination */
int copie(const char* source, const char* dest){
  
  /* on aura besoin d'un tampon intermediaire */
  char buf[BUFFER_SIZE]; 
  
  /* indique la fin du tampon */
  int buf_end;
  
  /* on aura besoin de descripteurs de fichier */
  int fd1, fd2, rc; 
  
  /* apres l'ouverture du fichier source on teste si l'ouverture a réussie  */
  fd1 = open(source, O_RDONLY); 
  
  if(fd1 < 0) {
    perror("open(fd1)") ;
    exit(1) ; 
  }
  
  /* même chose pour fichier destination */
  fd2 = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
  
  if(fd2 < 0) {
    perror("open(fd2)") ;
    exit(1) ; 
  }

  buf_end = 0;
 
  /* on récupere la ce qu'on lit dans source et on le met dans destination en passant par le tampon intermediaire */
  while(1) {
    rc = read(fd1, buf, BUFFER_SIZE);
    if(rc < 0) { 
      perror("read");
      exit(1); 
    } 
    if(rc == 0) break;
    buf_end = rc;
    rc = write(fd2, buf, buf_end);
    if(rc < 0) { 
      perror("write()"); 
      exit(1); 
    } 
    if(rc!= buf_end) {
      fprintf(stderr, "l'écriture a été interrompue");
      exit(1) ; 
    }
    buf_end = 0; 
  }
  /* on n'oublie pas de fermer les fichiers */ 
  close(fd1); 
  close(fd2);
  return 0;
}
