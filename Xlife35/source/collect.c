/*
 * XLife Copyright 1989 Jon Bennett jb7m+@andrew.cmu.edu, jcrb@cs.cmu.edu
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* 
Enhancements to #I format added by Paul Callahan (callahan@cs.jhu.edu).
New format is backward compatible with old format.
*/

/* collect.c: Collects separate included files into one file 
              that uses pattern blocks */

#include "defs.h"
#include "data.h"
#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include <stdlib.h>

char *checktilda();
unsigned long generations;


char *gethome()
{
    return(getpwuid(getuid())->pw_dir);
}


char *checktilda(char *stng)
{
    static char full[1024];
    struct passwd *pw;
    int i;
    if (stng[0]=='~') {
	i=1;
	while(stng[i]!='/' && stng[i]!='\0')
	    full[i-1]=stng[i], i++;
	if (i==1) 
	    strcpy(full,gethome());
	else {
	    full[i-1]='\0';
	    if((pw = getpwnam(full)) == NULL){
	    }
	    else{
		strcpy(full,pw->pw_dir);
	    }
	}
	strcat(full,stng+i);
    } else
	strcpy(full,stng);
    return(full);
}

char *addlifeext(char *buf)
{
    int len=strlen(buf);
    if(strcmp(buf+len-5,".life")){
	return(".life");
    }
    return("");
}

char *deletelifeext(char *buf)
{
    int len=strlen(buf);
    if(!strcmp(buf+len-5,".life")) buf[len-5]='\0';
    
    return(buf);
}

char *seppatdir(char *filename, char *filefield) 
{
char *ffptr,*colptr;
/* Separates path from rest of pattern's name.  */

   /* Make sure "/" in pattern block name isn't confusing by temporarily
      eliminating ":" suffix */
   if (colptr=strchr(filename,':')) (*colptr)='\0';

   ffptr=strrchr(filename,'/');
   if (ffptr) {
      strcpy(filefield,ffptr+1);
      *(ffptr+1)='\0';
   }
   else {
      strcpy(filefield,filename);
      *(filename)='\0';
   } 

   /* restore ":" suffix */

   if (colptr) (*colptr)=':';
   return  colptr;
}

void add_loadreq4(LoadReq **loadqueue, char *patname, int relpath, UNS32 loadtime)
{
  LoadReq *newreq;
  /* Find last entry where request can go while maintaining time order.
     Hopefully, this will cause the resulting file to be ordered in a nice
     way.

     After loop, we have a pointer to the pointer that must be modified
     (Could be pointer to list itself). */ 

  while ((*loadqueue)!=NULL && ((*loadqueue)->loadtime <= loadtime)) 
     loadqueue= &((*loadqueue)->next); 

  /* Create new request block and load it up */
  newreq=(LoadReq *)malloc(sizeof(LoadReq)); 
  newreq->loadtime=loadtime;
  newreq->relpath=relpath;
    /* Silently truncates long file names--should probably tell user */
  strncpy(newreq->patname,patname,PATNAMESIZ);

  /* Insert new request in queue */
  newreq->next=(*loadqueue);
  (*loadqueue)=newreq; 
}

int seen(LoadReq **seenlist, char *patname)
{
  LoadReq *newpat;

  /* Return whether pattern has been seen.  If not, add to list. */ 

  while ((*seenlist)!=NULL && strcmp((*seenlist)->patname,patname)) 
     seenlist = &((*seenlist)->next); 

  if ((*seenlist)==NULL) {
     /* Create new block and store name */
     newpat=(LoadReq *)malloc(sizeof(LoadReq)); 
     strncpy(newpat->patname,patname,PATNAMESIZ);

     /* Insert new pattern on list */
     newpat->next=(*seenlist);
     (*seenlist)=newpat; 
     return(0);
  } 
  return(1);
}

void parse_patname(char *patname, char *patfield)
{
char *pfptr;
/* Breaks "<filename>:<pattern>" into two strings. */

   pfptr=strchr(patname,':');
   if (pfptr) {
       *pfptr='\0';
       strcpy(patfield,pfptr+1);
   }
   else patfield[0]='\0';
}

int do_loadfile3(LoadReq **seenlist, LoadReq **loadqueue, int ismain)
{
    char patname[PATNAMESIZ],patfield[PATNAMESIZ];
    FILE *loadfl;
    char patfile[BUFSIZ];
    LoadReq *tmpqptr;
    int relpath;

/* Get load request */
    strcpy(patname,(*loadqueue)->patname);
    relpath=(*loadqueue)->relpath;

/* Delete request from queue */
    tmpqptr=(*loadqueue)->next; 
    free(*loadqueue);
   (*loadqueue)=tmpqptr;

    /* separate filename from pattern name */
    parse_patname(patname,patfield);

    /* add .life to filename if needed */
    (void) strcat(patname, addlifeext(patname));

    strcpy(patfile,patname);
    if ((loadfl=fopen(patfile,"r")) == NULL)
      {
       /* look for included pattern in pattern directory 
          if not found in given directory */
        strcpy(patfile,DIR);
        strcat(patfile,patname+relpath);
        if ((loadfl=fopen(patfile,"r")) == NULL) {
           /* if all else fails, try current directory */
           strcpy(patfile,"./");
           strcat(patfile,patname+relpath);
           loadfl=fopen(patfile,"r");
        };
      }

    if (loadfl==NULL) return 0;
    else {
	char	buf[BUFSIZ],patid[PATNAMESIZ],pardir[PATNAMESIZ];
        int     endpattern=0,found=0,relpathoffset=0;

        /* print comment telling where pattern was found */ 
        printf("## FOUND IN (%s):\n",patfile);

        /* If we are searching for a specific pattern in the file,
           then we skip lines till we find it.  This is a pretty tacky way
           to handle multiple pattern definitions in the same file,
           but files with #I format probably won't get big enough for
           anyone to notice.  Really, we should implement a dictionary to
           save the location of a pattern definition in a file the first time 
           we see it. 
        */
        deletelifeext(patname+relpath);
        if (patfield[0]!='\0') { 
	   while (!found && fgets(buf, BUFSIZ, loadfl) != (char *)NULL) { 
              if (buf[0]=='#' && buf[1]=='B') {
		 (void) sscanf(buf+2, " %s", patid); 
                 found=!strcmp(patfield,patid);
              } 
           }
           if (!found) {
	      fclose(loadfl);
              return 0; 
           } else if (!ismain) printf("#B %s.%s\n",patname+relpath,patfield);
        } else if (!ismain) printf("#B %s\n",patname+relpath);
	while (fgets(buf, BUFSIZ, loadfl) != (char *)NULL && !endpattern)
	{
	    if (buf[0] == '#')
	    {
		char	incl[BUFSIZ];
		char	tmpstring[PATNAMESIZ];
                int xoff,yoff,rotate,flip;
		unsigned long loadtime;

		incl[0] = '\0';
		switch(buf[1])
		{
                  case 'B':
                    /* Anything between #B and #E is ignored, since it
                       is a pattern definition to be included later.
                       #B and #E cannot be nested, so we just skip till
                       we pass #E */
	            while (fgets(buf, BUFSIZ, loadfl) != (char *)NULL
                           && !(buf[0]=='#' && buf[1]=='E')) {}
                    break;

                  case 'E': 
                    /* if we hit a "#E" at this point, then it must be
                       the one that closes the "#B <patfield>" that we
                       are reading (assuming a syntactically correct file) */
                    endpattern=1;
                    if (!ismain) printf("%s",buf);
                    break;

		  case 'I':
		    xoff = yoff = rotate = loadtime = 0;
                    flip = 1;
		    (void) sscanf(buf+2, " %s %d %d %d %d %ld", incl, 
                                 &xoff, &yoff, &rotate, &flip, &loadtime); 
                    /* Silently ignore invalid flips */
                    if ((flip!=1) && (flip!= -1)) flip=1;

                    /* if included pattern begins with ':' then assume
                       it's in current file */
                    if (incl[0]==':') {
                       strcpy(tmpstring,patfile);
                       strcat(tmpstring,incl);
                       strcpy(incl,tmpstring);
                    } else {
                       /* if relative path given, assume directory of parent */
                       if (!strchr("/~",incl[0])) {
                          strcpy(pardir,patfile);
                          seppatdir(pardir,tmpstring);
                          relpathoffset=strlen(pardir);
                          strcat(pardir,incl);
                          strcpy(incl,pardir);
                       }
                    } 
                    if (!seen(seenlist,incl)) 
                      add_loadreq4(loadqueue, incl, relpathoffset, 
                                   generations+loadtime); 
                    parse_patname(incl,tmpstring);
                    deletelifeext(incl);
                    if (tmpstring[0]!='\0') 
                       printf("#I :%s.%s %d %d %d %d %ld\n",incl+relpathoffset,
                                                           tmpstring,
                                    xoff, yoff, rotate, flip, loadtime); 
                    else
                       printf("#I :%s %d %d %d %d %ld\n",incl+relpathoffset,
                                    xoff, yoff, rotate, flip, loadtime); 
		    break;
		    
		  default:
                    printf("%s",buf); 
		    break;
		}
	    }
	    else printf("%s",buf);
	}
	fclose(loadfl);
        if (patfield[0]=='\0' && !ismain) printf("#E\n"); 
        printf("\n");
	return 1;
   }
}

int do_loadreq2(LoadReq **seenlist, LoadReq *loadqueue)
{
char thispat[PATNAMESIZ];

   strcpy(thispat,loadqueue->patname);
   if (!do_loadfile3(seenlist,&loadqueue,1)) {
       fprintf(stderr,"Can't load (%s)\n",thispat);
	   return  -1;
   }
   while(loadqueue!=NULL) {
      generations=loadqueue->loadtime;
      strcpy(thispat,loadqueue->patname);
      if (!do_loadfile3(seenlist,&loadqueue,0)) {
          fprintf(stderr,"Can't load (%s)\n",thispat);
	      return  -1;
      }
   }
   return 0;
}


void main(unsigned argc, char *argv[])
{
    char outbuf[PATNAMESIZ];
    LoadReq *loadqueue=NULL;
    LoadReq *seenlist=NULL;
      
    if (argc==2) {
       strcpy(outbuf,checktilda(argv[1]));
       generations=0;
       add_loadreq4(&loadqueue, outbuf, 0, generations); 
       seen(&seenlist,outbuf);
       do_loadreq2(&seenlist,loadqueue);
    } else fprintf(stderr,"Wrong number of arguments\n");

    exit(0);
}
