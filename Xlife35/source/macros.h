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


#define COMPUTELINE(x1,x2,x3,x4,v1)\
    		v1 = 0;\
		x1=(0x1<<((x3)&0xf));\
		x2=x4&0x1;\
		if ((x2&&(x1&live))||((!x2)&&(x1&born)))  v1 |= 0x1;\
		x1=(0x1<<((x3>>(4))&0xf));\
		x2=x4&0x2;\
		if ((x2&&(x1&live))||((!x2)&&(x1&born)))  v1 |= 0x2;\
		x1=(0x1<<((x3>>(8))&0xf));\
		x2=x4&0x4;\
		if ((x2&&(x1&live))||((!x2)&&(x1&born)))  v1 |= 0x4;\
		x1=(0x1<<((x3>>(12))&0xf));\
		x2=x4&0x8;\
		if ((x2&&(x1&live))||((!x2)&&(x1&born)))  v1 |= 0x8;


    
	

