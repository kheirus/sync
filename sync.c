
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




/* dès qu'un flag est mis à 1 cela veut dire que l'option est activé */
int flagOption [4]; 

/* buf4 et buf5 nous servent pour manipuler les chemin de source et destination, (une petite bricole) */ 
/* on les déclare comme des variables globales puisque elle sont utilisées dans la fonction copieChemin */
char buf4[BUFFER_SIZE];
char buf5[BUFFER_SIZE];

/* on l'utilise pour les liens symbolique */
char buf6[BUFFER_SIZE];



int main (int argc, char** argv) {

  /* initialisation des flags */
  flagOption [0]= 0; /* pour l'option -r */  
  flagOption [1]= 0; /* pour l'option -n */  
  flagOption [2]= 0; /* pour l'option -i */  
  flagOption [3]= 0; /* pour l'option -s */  
  
  
  
  /* on doit entrer au moin trois arguments (nom de la commande [chemin destination] [chemin source]) */
  if (argc <3) {
    printf ("\nVEUILLEZ UTILISER LA COMMANDE COMME SUIT :\n\n synchronise [options] [chemin destination] [chemin source]\n\n");
    printf ("options :\n");
    printf ("-r\tsynchronise récursivement les sous-répertoires de source et destination, les sous-répertoires de sous-répertoires...etc\n");
    printf ("-n\tcopie tout fichier qui existe dans source mais pas dans destination.\n");
    printf ("-i\tdemande une confirmation de l'utilisateur pour chaque fichier que le programme s'apprête à synchroniser.\n");
    printf ("-s\tsynchronise les liens symboliques.\n\n");
    return EXIT_FAILURE;
  }
  
  /* on cherche quelles options on été déclenchées */
  else {
    if (argc > 3  ) {
    
      if (exist ("-r",argv,argc)==0) 
	flagOption[0]=1;
    
      if (exist ("-n",argv,argc)==0)
	flagOption[1]=1;
    
      if (exist ("-i",argv,argc)==0)
	flagOption[2]=1;
    
      if (exist ("-s",argv,argc)==0)
	flagOption[3]=1;
    } 
    
  }
  
  /* argc-2 pour destination  et argc -1 pour source */
  syncro (argv[argc-2],argv[argc-1],flagOption);
       
  return 0;
}

/* c'est notre fonction principale elle synchronise le repertoire source et destination */
int syncro (char* dest, char* source, int tab[4]) {
  /* on aura besoin de deux repertoires ddest pour destination */
  /* et dsource pour source */
  DIR* ddest;
  DIR* dsource;

  /* on aura besoin de deux structures dirent pour recuperer le nom des fichiers */
  struct dirent* entd;
  struct dirent* ents;
  
  /* En plus des noms on aura besoin aussi de la date de derniere modification et du mode, donc du statut du fichier */
  struct stat sd;
  struct stat ss;
  
  /* buf2 et buf3 servent à recuperer le chemin des fichiers sources et destinations, on en aura besoin pour la gestion d'erreur de stat  */
  char buf2[BUFFER_SIZE];
  char buf3[BUFFER_SIZE];
  char buf7[BUFFER_SIZE];
  /* creation d'un nouveau repertoire */
  /* char buf8[BUFFER_SIZE]; */
  /* char buf9[BUFFER_SIZE]; */
  char bufLink [256];
  
  /* trouve permet de determiner si fichier existe dans destination  */
  int trouve; 

  /* on ouvre le repertoire destination et source qu'on met dans un DIR* */
  ddest=opendir(dest);
  dsource = opendir(source);
  


  /* on teste bien évidemment si l'ouverture des répértoires à réussie, sinon on lance perror */
  if (ddest ==NULL) perror ("opendir destination");
  if (dsource ==NULL) perror ("opendir source");
  
  /* on lit le contenu du repertoire source, tant que different de NULL */
  while ((ents=readdir(dsource))!=NULL) {
    /* on ignore la lecture du repertoire courant "." et du repertoire parent ".." */
    /* il faut souligner aussi que si on est sous Mac OS X on aura besoin d'ignorer le fichier ".Ds_Store" généré automatiquement par le systeme
       et qui peut falcifier le résultat (testé sous MAC OS X 10.7)*/
    if ((strcmp(ents->d_name, ".") == 0 || strcmp(ents->d_name, "..") == 0))
      continue;
 
    sprintf(buf3,"%s%s",source,ents->d_name);
    
    /* on utilise lstat au lieu de stat car les fichiers peuvenet être aussi des lien symbolique (on test avec la macro SISLNK()) */
    if ((lstat(buf3,&ss)) <0) perror ("lstat source buf3");
    
    /* (si le fichier est bien un fichier régulier (pas un lien symbolique)) OU (le flag -s a été déclanché et que c'est un lien symbolique) 
       OU (le flag -r est déclanché et que c'est un répértoire) */
    if (((S_ISREG(ss.st_mode)) && (S_ISLNK (ss.st_mode)==0)) || ((tab[3]==1) && (S_ISLNK (ss.st_mode)==1)) ||(tab[0]==1 && (S_ISDIR(ss.st_mode)))) {         
      
      /* on initialise trouve à 0 (pas de fichier en commun pour le moment) */
      trouve = 0;
      
      /* on parcours destination */
      while ((entd=readdir(ddest))!=NULL) {
      
	/* on ignore la lecture du repertoire courant "." et du repertoire parent ".." */
	/* il faut souligner aussi que si on est sous Mac OS X on aura besoin d'ignorer le fichier ".Ds_Store" généré automatiquement par le systeme
	   et qui peut falcifier le résultat (testé sous MAC OS X 10.7)*/
	if ((strcmp(entd->d_name, ".") == 0 || strcmp(entd->d_name, "..") == 0))
	  continue;
      
	/* on remplit les buffers buf2 et buf3 réspectivement par le chemin du fichier se trouvant dans le repertoire destination et source */
	sprintf(buf2,"%s%s",dest,entd->d_name);
      
	
	if ((lstat(buf2,&sd)) <0) perror("stat destination") ;
      
	/* on teste si le fichier dans source se trouve aussi dans destination */
	if(strcmp(entd->d_name,ents->d_name)==0) {

	  /*-------------------------------------- SANS OPTION --------------------------------------*/
	  
	  /* on teste si le contenu est bien des fichiers réguliers (commençant par - avec la fonction "ls") */
	  if (((S_ISREG(sd.st_mode)) && (S_ISLNK (sd.st_mode)==0)) && ((S_ISREG(ss.st_mode)) && (S_ISLNK (ss.st_mode)==0))) { 
	   
	    /* on a trouvé un fichier le flag trouve est maintenant à 1 */
	    trouve=1;
	    
	    /* on teste si la date de modification dans source est plus récente que celle dans destination */
	    if ((int)ss.st_mtime > (int)sd.st_mtime) {
	      
	      /*-------------------------------------- OPTION -i --------------------------------------*/
	      if (tab[2]==1)  {
		
		if(confirmation(ents->d_name)==-1){
		  continue;
		}
	      }
		
	      copieChemin(source,dest,ents->d_name,buf4,buf5);
	  
	      /* on copie le fichier de source vers destination, tout en testant si cela a bien fonctionné */ 
	      if (copie(buf4,buf5)==0)
		printf("%s\t\t file synchronized \n",ents->d_name);
	  
		  
	      /* on aura besoin de récuperer le statut du nouveau fichier synchronisé */
	      modifDate(buf4,buf5);
	      
	    }
	  }
	    

	  /*-------------------------------------- OPTION -s --------------------------------------*/
	  /* si option -s déclenché et les fichiers dans source et destination sont des fichier réguliers */
	  if ((tab[3]==1) && (S_ISLNK (sd.st_mode)) && (S_ISLNK(ss.st_mode))){
	    
	    /* on a trouvé un fichier le flag trouve est maintenant à 1 */
	    trouve =1;
	    
	    /*-------------------------------------- OPTION -i -s --------------------------------------*/
	    /* en plus de synchro des liens symbolique on demande la confirmation de l'utilisateur */
	    if (tab[2]==1) { 
	      if(confirmation(ents->d_name)==-1)
		continue;
	    }
	    
	    copieChemin(source,dest,ents->d_name,buf4,buf5);
	    
	    /* recupere le fichier pointé par source (buf4) et le mettre dans bufLink */
	    readlink(buf4,bufLink,sizeof(bufLink));
	      
	    sprintf(buf6,"%s%s",source,bufLink);
	    
	    /* supprimer le lien dans destination */
	    unlink(buf5);
	    
	    /* cree un lien symbolique du même no que source et pointe vers le même fichier que celui de source */
	    if(symlink(buf6,buf5) < 0)
	      perror("symlink()");
	      
	    /* on n'oublie pas de modifer la date */
	    modifDate(buf5,buf6);
	    
	  }  
	
	
	
     

	/*-------------------------------------- OPTION -r [-i] [-s] [-n] --------------------------------------*/
	/* si le fichier qu'on a lu est en fait un répértoire et que l'option -r est déclenché */
	  if (tab[0]==1 && (S_ISDIR(ss.st_mode))) {
	  
	  if (tab[2]==1) {
	      if(confirmation(buf3)==-1)
	  	continue;
	    }
	  
	  
	  if (S_ISDIR(sd.st_mode)){
	  
	    copieChemin(source,dest,ents->d_name,buf4,buf5);
	    sprintf(buf7,"%s%s",dest,entd->d_name);
	    
	    /* on rajoute un slach à la fincar c'est un répértoire */
	    strcat(buf3,"/");
	    strcat(buf7,"/");
	    
	    
	    /* appel récursif de syncro sur le sous répértoire destination du même nom du sous répértoire source ainsi de suite (récursivement) */
	    if (syncro (buf7,buf3,flagOption)==0) {
	      modifDate(buf3,buf7);
	      printf("\n");
	      printf("%s\t\t directory synchronized\n\n",buf7);
	      
	    }
	   
	    
	  }
	}
	} /* fin fichiers (répértoires) du même nom */
	


	/********************************************************** PROBLEME AVEC LA CREATION D'UN REPERTOIRE **********************************************************/

	/* /\*-------------------------------------- OPTION -r -n [-i]--------------------------------------*\/ */
	/* /\* S'IL YA UN DOSSIER QUI N'EXISTE PAS DANS DESTINATION ALORS ON LE CREE *\/ */
	
	/* if ((strcmp(entd->d_name,ents->d_name)!=0) && ((tab[0]==1) && (tab[1]==1) && (S_ISDIR(ss.st_mode)))){ */
	   
	/*  if ((tab[0]==1) && (tab[1]==1) && (S_ISDIR(ss.st_mode))) { */
	    
	/*   if (tab[2]==1) { */
	/*       if(confirmation(buf3)==-1) */
	/*   	continue; */
	/*     } */
	  
	  
	/*   if (S_ISDIR(sd.st_mode)) { */
	  
	/*   copieChemin(source,dest,ents->d_name,buf8,buf9); */
	/*   sprintf(buf9,"%s%s",dest,entd->d_name); */
	   
	/*     /\* on rajoute un slach à la fincar c'est un répértoire *\/ */
	/*     strcat(buf8,"/"); */
	/*     strcat(buf9,"/"); */
	    
	
	

	/*   mkdir(buf9,0777); */
	   
	/*     /\* appel récursif de syncro sur le sous répértoire destination du même nom du sous répértoire source ainsi de suite (récursivement) *\/ */
	/*     if (syncro (buf9,buf8,flagOption)==0) { */
	/*       modifDate(buf8,buf9); */
	/*       printf("\n"); */
	/*       printf("%s\t\t directory synchronized\n\n",buf9); */
	      
	/*     } */
	   
	    
	/*  } */
	/*  } */
	/* } */
	/**********************************************************FIN PROBLEME AVEC LA CREATION D'UN REPERTOIRE *******************************************************/
	  
      } /* fin while destination */
    } /* fin GRAND IF */
       
     
    /*-------------------------------------- OPTION -n --------------------------------------*/
    /* si le fichier existe dans source et pas dans destination on le créé */
    if ((trouve==0) && (tab[1]==1)) {  
      
      copieChemin(source,dest,ents->d_name,buf4,buf5);
      
      /* si un fichier régulier et non pas un lien symbolique (==0) */
      if ((S_ISREG(ss.st_mode)) && (S_ISLNK(ss.st_mode)==0)) {
	/*-------------------------------------- OPTION [-i] -n --------------------------------------*/
	if (tab[2]==1)  {
	  
	  if (confirmation(ents->d_name)==-1)
	    continue;
	}
	
	/* on copie le fichier de source vers destination, tout en testant si cela a bien fonctionné */ 
	if (copie(buf4,buf5)==0)
	  printf("%s\t\t file created \n",ents->d_name);
	
      }
     
      /*-------------------------------------- OPTION -s -n --------------------------------------*/
      if ((tab[3]==1) && (S_ISLNK(ss.st_mode)==1)) {
	
	/*-------------------------------------- OPTION [-i] -s -n --------------------------------------*/
	if (tab[2]==1){

	  if (confirmation(ents->d_name)==-1)
	    continue;
	}
	copieChemin(source,dest,ents->d_name,buf4,buf5);
	    
	/* recupere le fichier pointé par source (buf4) et le mettre dans bufLink */
	readlink(buf4,bufLink,sizeof(bufLink));
	      
	sprintf(buf6,"%s%s",source,bufLink);
	    
	/* supprimer le lien dans destination */
	unlink(buf5);
	    
	/* cree un lien symbolique du même no que source et pointe vers le même fichier que celui de source */
	if(symlink(buf6,buf5) < 0)
	  perror("symlink()");
	      
	/* on n'oublie pas de modifer la date */
	    
	modifDate(buf4,buf5);
      }

      modifDate(buf4,buf5);




      
    }
    





    /* on aura besoin de se remettre au debut du repertoire à chaque itération */
    rewinddir(ddest);
  }
  
  /* comme tout bon programmeur on oublie pas de fermer les repertoire */ 
  /* Mais comme on est pas encore des supers programmeurs on teste pas si la fermeture a réussie...ça viendra */
  closedir(ddest);
  closedir(dsource);
    
  /* la fonction retourne 0 si en cas de réussite  */
  return 0;
} 
