


#define BUFFER_SIZE 4096 /* on définie une constante pour les buffers */


int copieChemin (char* source, char* dest, char* nomFichierSource, char buf4[BUFFER_SIZE],char buf5[BUFFER_SIZE]);
int modifDate(char*nomFichierSource, char*nomFichierDestination);
int confirmation (char * nomFichier);
int copie(const char* dest, const char* source);
int exist (char* option,char**tab,int n);
int syncro (char* dest, char* source, int tab[4]);
