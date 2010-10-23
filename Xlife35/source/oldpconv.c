#include <stdio.h>
#include <strings.h>
#define MAXSTRING 256

/* Filter to make old #P files read in at the same coordinates
   as in version 2.0 */

int main(unsigned argc, char *argv[]) 
{
static char convcomm[]="## converted to new format by oldpconv";
FILE *fptr,*wfptr;
char s[MAXSTRING];
char bakfile[MAXSTRING];
long x,y;
int linenum,i,converted;

  if (argc<2) {
      fprintf(stderr,
              "You must specify a list of files to be converted\n");
      return 1;
  }


  for (i=1; i<argc; i++) {
     strcpy(bakfile,argv[i]); 
     strcat(bakfile,"~");

     if (fptr=fopen(bakfile,"r")) {
         fprintf(stderr,
                 "Can't create backup (would overwrite %s) -- exiting\n",bakfile);
         fclose(fptr); 
         return 2;
     } else if (rename(argv[i],bakfile)) {
         fprintf(stderr, "Can't move %s to backup file -- exiting\n",argv[i]);
         return 3;
     }

     if (!(fptr=fopen(bakfile,"r"))) {
         fprintf(stderr, "Can't open %s for reading -- exiting\n",bakfile);
         return 4;
     }

     if (!(wfptr=fopen(argv[i],"w"))) {
         fprintf(stderr, "Can't open %s for writing -- exiting\n",argv[i]);
         fclose(fptr);
         return 5;
     }

     converted=0;
     fprintf(wfptr,"%s\n",convcomm);
     linenum=0;
     while(!converted && fgets(s,MAXSTRING,fptr)) {
        if (s[0]=='#')
        {
          switch(s[1]) {
            case 'P':
              x=y=0; 
              sscanf(s+2,"%ld %ld",&x,&y);
              fprintf(wfptr,"#P %ld %ld\n",x,y+linenum); 
              break;
            case '#':
              if (!strncmp(s,convcomm,strlen(convcomm))) {
                 fprintf(stderr,"Looks like %s was already converted\n",argv[i]);
                 converted=1;
                 rename(bakfile,argv[i]);
              } 
             /* break omitted intentionally */
            default: 
              fprintf(wfptr,"%s",s); 
          }
        }
        else fprintf(wfptr,"%s",s); 

        linenum++;
     }
     fclose(fptr);
     fclose(wfptr);
  }
  return 0;
}
