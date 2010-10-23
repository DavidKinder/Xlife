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
 *
 * CMU SUCKS
 */

#include "defs.h"
#include "cellbox.h"
#include "tab.h"

void generate(){
    
    
    register u_long t1,t2,t3,*tmpptr,y;
    cellbox *cptr,*tptr,*cptrup,*cptrdn,*cptrlf,*cptrrt;

#ifdef PROF    
    link_called = link_search = 0;
    create_called = create_null = 0;
#endif

    cptr = head;
    numcells=0;
    numchang=0;
    generations++;
    while(cptr ){

	if(!(cptr->live1 || cptr->live2)){
	    cptr = cptr->next;
	    continue;
	}

	cptrup=cptr->up;
	cptrdn=cptr->dn;
	cptrlf=cptr->lf;
	cptrrt=cptr->rt;
	
	t1=cptr->live1&0xff;
	if(t1){
	    if(cptrup==NULL){
		cptrup=lifelink(cptr->x,cptr->y - 8);
		cptrup->dn=cptr;
	    }
	    t2=tab1[t1];
	    cptrup->on[7]+=t2;
	    cptr->on[1]+=t2;
	    cptr->on[0]+=tab2[t1];
	}
	
	t1=(cptr->live2 & 0xff000000)>>24;	    
	if(t1){
	    if(cptrdn==NULL){
		cptrdn=lifelink(cptr->x,cptr->y + 8);
		cptrdn->up=cptr;
	    }
	    t2=tab1[t1];
	    cptrdn->on[0]+=t2;
	    cptr->on[6]+=t2;
	    cptr->on[7]+=tab2[t1];
	}

	t1=cptr->live1;
	t2=cptr->live2;

	if(t1 & 0x1010101){
	    if(cptrlf==NULL){
		cptrlf=lifelink(cptr->x - 8,cptr->y);
		cptrlf->rt=cptr;
	    }
	    if(t1 & 0x1){
		cptrlf->on[0]+=0x10000000;
		cptrlf->on[1]+=0x10000000;
		if(cptrlf->up==NULL){
		    cptrlf->up=lifelink(cptr->x - 8,cptr->y - 8);
		}
		cptrlf->up->on[7]+= 0x10000000;
		cptrlf->up->dn=cptrlf;
	    }
	    if(t1 & 0x100){
		cptrlf->on[0]+=0x10000000;
		cptrlf->on[1]+=0x10000000;
		cptrlf->on[2]+=0x10000000;
	    }
	    if(t1 & 0x10000){
		cptrlf->on[1]+=0x10000000;
		cptrlf->on[2]+=0x10000000;
		cptrlf->on[3]+=0x10000000;
	    }
	    if(t1 & 0x1000000){
		cptrlf->on[2]+=0x10000000;
		cptrlf->on[3]+=0x10000000;
		cptrlf->on[4]+=0x10000000;
	    }
	}
	
	if(t2 & 0x1010101){
	    if(cptrlf==NULL){
		cptrlf=lifelink(cptr->x - 8,cptr->y);
		cptrlf->rt=cptr;
	    }
	    if(t2 & 0x1){
		cptrlf->on[3]+=0x10000000;
		cptrlf->on[4]+=0x10000000;
		cptrlf->on[5]+=0x10000000;
	    }
	    if(t2 & 0x100){
		cptrlf->on[4]+=0x10000000;
		cptrlf->on[5]+=0x10000000;
		cptrlf->on[6]+=0x10000000;
	    }
	    if(t2 & 0x10000){
		cptrlf->on[5]+=0x10000000;
		cptrlf->on[6]+=0x10000000;
		cptrlf->on[7]+=0x10000000;
	    }	    
	    if(t2 & 0x1000000){
		cptrlf->on[6]+=0x10000000;
		cptrlf->on[7]+=0x10000000;
		if(cptrlf->dn==NULL){
		    cptrlf->dn=lifelink(cptr->x - 8,cptr->y + 8);
		}
		cptrlf->dn->on[0]+= 0x10000000;
		cptrlf->dn->up=cptrlf;
	    }
	}
	
	if(t1 & 0x80808080){
	    if(cptrrt == NULL){
		cptrrt=lifelink(cptr->x + 8,cptr->y);
		cptrrt->lf=cptr;
	    }
	    if(t1 & 0x80){
		cptrrt->on[0]+=0x1;
		cptrrt->on[1]+=0x1;
		if(cptrrt->up==NULL){
		    cptrrt->up=lifelink(cptr->x + 8,cptr->y - 8);
		}
		cptrrt->up->on[7]+= 0x1;
		cptrrt->up->dn=cptrrt;
	    }
	    if(t1 & 0x8000){
		    cptrrt->on[0]+=0x1;
		    cptrrt->on[1]+=0x1;
		    cptrrt->on[2]+=0x1;
	    }
	    if(t1 & 0x800000){
		    cptrrt->on[1]+=0x1;
		    cptrrt->on[2]+=0x1;
		    cptrrt->on[3]+=0x1;
	    }
	    if(t1 & 0x80000000){
		    cptrrt->on[2]+=0x1;
		    cptrrt->on[3]+=0x1;
		    cptrrt->on[4]+=0x1;
	    }
	}
	
	if(t2 & 0x80808080){
	    if(cptrrt == NULL){
		cptrrt=lifelink(cptr->x + 8,cptr->y);
		cptrrt->lf=cptr;
	    }
	    if(t2 & 0x80){
		    cptrrt->on[3]+=0x1;
		    cptrrt->on[4]+=0x1;
		    cptrrt->on[5]+=0x1;
	    }
	    if(t2 & 0x8000){
		    cptrrt->on[4]+=0x1;
		    cptrrt->on[5]+=0x1;
		    cptrrt->on[6]+=0x1;
	    }
	    if(t2 & 0x800000){
		    cptrrt->on[5]+=0x1;
		    cptrrt->on[6]+=0x1;
		    cptrrt->on[7]+=0x1;
	    }
	    if(t2 & 0x80000000){
		cptrrt->on[6]+=0x1;
		cptrrt->on[7]+=0x1;
		if(cptrrt->dn==NULL){
		    cptrrt->dn=lifelink(cptr->x + 8,cptr->y + 8);
		}
		cptrrt->dn->on[0]+= 0x1;
		cptrrt->dn->up=cptrrt;
	    }
	}
	
	t1=(cptr->live1 & 0xff00) >> 8;
	t2=(cptr->live1 & 0xff0000) >> 16;
	
	if(t1){
	    t3 = tab1[t1];
	    cptr->on[1]+=tab2[t1];
	    cptr->on[0]+=t3;
	    cptr->on[2]+=t3;
	}
	
	t1=(cptr->live1 & 0xff000000) >> 24;
	
	if(t2){
	    t3 = tab1[t2];
	    cptr->on[2]+=tab2[t2];
	    cptr->on[1]+=t3;
	    cptr->on[3]+=t3;
	}
	
	t2=(cptr->live2 & 0xff);
	
	if(t1){
	    t3 = tab1[t1];
	    cptr->on[3]+=tab2[t1];
	    cptr->on[2]+=t3;
	    cptr->on[4]+=t3;
	}
	
	t1=(cptr->live2 & 0xff00) >> 8;
	
	if(t2){
	    t3 = tab1[t2];
	    cptr->on[4]+=tab2[t2];
	    cptr->on[3]+=t3;
	    cptr->on[5]+=t3;
	}
	
	t2=(cptr->live2 & 0xff0000) >> 16;	    
	
	if(t1){
	    t3 = tab1[t1];
	    cptr->on[5]+=tab2[t1];
	    cptr->on[4]+=t3;
	    cptr->on[6]+=t3;
	}
	
	if(t2){
	    t3 = tab1[t2];
	    cptr->on[6]+=tab2[t2];
	    cptr->on[5]+=t3;
	    cptr->on[7]+=t3;
	}
	
	cptr->up=cptrup;
	cptr->dn=cptrdn;
	cptr->lf=cptrlf;
	cptr->rt=cptrrt;
	cptr=cptr->next;
    }
    
    cptr=head;
    while(cptr){
	t1=cptr->live1;
	cptr->olive1=t1;
	tmpptr=cptr->on;
	t2=0;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2=lookup[((t3 & 0xffff)<<4) + (t1&0xf)];
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>4)&0xf)] << 4;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2|=lookup[((t3 & 0xffff)<<4) + ((t1>>8)&0xf)] << 8;
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>12)&0xf)] << 12;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2|=lookup[((t3 & 0xffff)<<4) + ((t1>>16)&0xf)] << 16;
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>20)&0xf)] << 20;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2|=lookup[((t3 & 0xffff)<<4) + ((t1>>24)&0xf)] << 24;
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>28)&0xf)] << 28;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	cptr->live1=t2;
	t1=cptr->olive2=cptr->live2;	
	t2=0;

	if(t3 &0xffff){
	    t2=lookup[((t3 & 0xffff)<<4) + (t1&0xf)];
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>4)&0xf)] << 4;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2|=lookup[((t3 & 0xffff)<<4) + ((t1>>8)&0xf)] << 8;
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>12)&0xf)] << 12;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2|=lookup[((t3 & 0xffff)<<4) + ((t1>>16)&0xf)] << 16;
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>20)&0xf)] << 20;
	}
	*tmpptr=0;
	tmpptr++;
	t3= *tmpptr;
	if(t3 &0xffff){
	    t2|=lookup[((t3 & 0xffff)<<4) + ((t1>>24)&0xf)] << 24;
	}
	if(t3 &0xffff0000){
	    t2|=lookup[((t3 & 0xffff0000)>>12) + ((t1>>28)&0xf)] << 28;
	}
	*tmpptr=0;
	cptr->live2=t2;

	if(dispchang){
	    t1=cptr->live1 ^ cptr->olive1;
	    for(y=0;y<32;y++)
		if(t1 & (1<<y)) numchang++;
	    t1=cptr->live2 ^ cptr->olive2;
	    for(y=0;y<32;y++)
		if(t1 & (1<<y)) numchang++;
    }
	
	if(dispcells || numcellsperiod){
	    t1=cptr->live1;
	    for(y=0;y<32;y++)
		if(t1 & (1<<y)) numcells++;
	    t1=cptr->live2;
	    for(y=0;y<32;y++)
		if(t1 & (1<<y)) numcells++;
	}	
	
 	if(cptr->live1 || cptr->live2){
	    cptr->dead=0;
	    cptr=cptr->next;
	}
	else{
	    cptr->dead++;
	    if(cptr->dead > maxdead){
		tptr=cptr->next;
		killbox(cptr);
		cptr=tptr;
	    }
	    else{
		cptr=cptr->next;
	    }
	}
    }
#ifdef PROF
    printf("num=%d ",numboxes);
    if(link_called){
	printf("l_c=%d ",link_called);
	if(link_search){
	    printf(" l_ave=%f ",link_search/(float)link_called);
	}
    }
    if(create_called){
	printf("c_c=%d ",create_called);
	if(create_null){
	    printf(" c_ave=%f ",create_null/(float)create_called);
	}
    }

    printf("\n");
#endif
}
