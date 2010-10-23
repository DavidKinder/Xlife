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

#include "defs.h"
#include "macros.h"

void gentab()
{
    struct worktab {
	u_long value;
	u_long off_1;
	u_long off_2;
    } work[32], *wend = work+32, *w1, *w2, *w3, *w4;
    
    u_long val1, val2, val3;
    char *ptr1, *ptr2, *ptr3;
    u_long t1;
    int i;

    for (i=0, t1=1; i<16; i++, t1<<=1) {
	work[i].value = (born & t1) ? 1 : 0;
	work[i].off_1 = i;
	work[i].off_2 = 0;
    }
    for (i=0, t1=1; i<16; i++, t1<<=1) {
	work[i+16].value = (live & t1) ? 1 : 0;
	work[i+16].off_1 = i;
	work[i+16].off_2 = 1;
    }

    for (w1=work; w1<wend; w1++) {
	ptr1 = lookup + w1->off_1*0x10000+w1->off_2*0x8;
	val1 = w1->value * 0x8;
	for (w2 = work; w2<wend; w2++) {
	    ptr2 = ptr1 + w2->off_1*0x01000+w2->off_2*0x4;
	    val2 = val1 + w2->value*0x4;
	    for (w3 = work; w3<wend; w3++) {
		ptr3 = ptr2 + w3->off_1*0x00100+w3->off_2*0x2;
		val3 = val2 + w3->value*0x2;
		for (w4 = work; w4 < wend; w4++) {
		    ptr3[w4->off_1*0x00010+w4->off_2*0x1] = val3 + w4->value*0x1;
		}
	    }
	}
    }
/*    for (count=0;count <= 0xffff;count++){
	for (cells=0;cells <= 0xf;cells++){
	    
	    COMPUTELINE(t1,t2,count,cells,val);
	    *ptr= (char)val;
	    ptr++;
	}
    }*/
}
