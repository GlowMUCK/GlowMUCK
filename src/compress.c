/*

				* NOTICE! *

This source code is Copyright 1995 by Dragon's Eye Productions.

Permission is granted to make unlimited use of this code as part of any
PakRat&Loki Fuzzball based MUCK that complies with all of the terms of
the copyrights on the PakRat&Loki coded parts of the MUCK along with all
copyrights on Revar's Fuzzball MUCK code.  No other permission is granted
or implied for this code or any derivative works based upon it, and no
such permission should be assumed.  If you are interested in licensing
DragonFire compression for use in a commercial product or service, or
requesting permission to use it in some other way, please contact Dr. Cat
at cat@eden.com.

The standard copyright notice and requirements given by Dr. Cat as provided
for the Foxen Fuzzball TinyMuck distribution applies totally to this code,
along with additional permission given by Dr. Cat to PakRat&Loki for use of
this code in their modified Fuzzball TinyMuck server.  No profit may be made
from the use of this code without the direct written consent by ALL authors
of any and all parts of this server.

*/
 
#include "config.h"
#include "local.h"

#define BUFFER_LEN 16384	/* nice big buffer */
 
#define TOKEN_BIT 0x80		/* if on, it's a token */
#define TOKEN_MASK 0x7f		/* for stripping out token value */
#define NUM_TOKENS (128)
#define MAX_CHAR (128)
 
 
/* static char token_table[MAX_CHAR][MAX_CHAR]; */
static int table_initialized = 0;
static char *dict[4096], *dict2[4096], line[80], buffer[40], chksum_buf[80];
static int seconds[43][43],special_case=5;
static int c_index,repeats;
static int lastmatch,completematch,maxmatch;
static char *to;
 
/* static const char copyright[]="Copyright 1995 by Dragon's Eye Productions."; */
 
 
int comp_read_line(FILE *file)
{
	int c;
	int i;
 
	c=0;
	for(i=0;c!='\n'&&c!=EOF;i++)
	{
		c=fgetc(file);
		line[i]=c;
	}
	line[i-1]=0;
	return(i);
}
 
void clear_buffer(void)
{
	int i;
 
	for(c_index=i=0;i<32;i++)
		buffer[i]=0;
}
 
void
save_compress_words_to_file(FILE *f)
{
    int i;
    for (i = 0; i < 4096; i++) {
	fprintf(f, "%s\n", dict2[i]);
    }
}

void 
init_compress_from_file(FILE *dicto)
{
	int i,j,n;
 
	i=0;
	while (!feof(dicto) && i < 4096)
	{
		n = comp_read_line(dicto);
		dict[i]= (char *) malloc(n);
		dict2[i]= (char *) malloc(n);
		strcpy(dict2[i],line);
		for (j = 0; line[j]; j++)
		    if (line[j] >= 'a')
			line[j] -= 32;
		strcpy(dict[i++],line);
	}
	if (i < 4096) {
	    fprintf(stderr, "Too few words in compression wordlist!  Aborting.\n");
	    exit(1);
	}
	clear_buffer();
	for(i=0;i<43;i++)
		for(j=0;j<43;j++)
			seconds[i][j]=-1;
	for(n=0;n<4095;n++)
	{
		if(seconds[dict[n][0]-48][dict[n][1]-48]==-1)
			seconds[dict[n][0]-48][dict[n][1]-48]=n;
		if(!special_case)
			if(dict[n][0]=='i')
				if(dict[n][1]==39)
					special_case=n;
	}
	table_initialized = 1;
}
 
static void 
init_compress(void)
{
	FILE *dicto;
 
	if ((dicto = fopen(WORDLIST_FILE, "r")) == NULL)
	{
		fprintf(stderr, "Cannot open %s dictionary file.\n", WORDLIST_FILE);
		exit(5);
	}
	init_compress_from_file(dicto);
	fclose(dicto);
}
 
static int 
compressed(const char *s)
{
    if (!s)
	return 0;
    while (*s) {
	if (*s++ & TOKEN_BIT)
	    return 1;
    }
    return 0;
}
 
void
compression_filter(char c)
{
 	int a,b,d,e;
	char buffer2[40];
 
	if(!c_index)
	{
		buffer[c_index++]=c;
		completematch=-1;
		return;
	}
	if((d=c)>='a') d-=32;
	if(c_index==1)
	{
		if((e=buffer[0])>='a') e-=32;
		if(c==39)
			if(e=='I')
			{
				maxmatch=lastmatch=special_case;
				goto here2;
			}
		if(e<48||e>90||d<48||d>90 || ((c>='A')&&(c<='Z')) )
			lastmatch=-1;
		else if( (maxmatch=lastmatch=seconds[e-48][d-48]) != -1)
		{
here2:
			if(dict[lastmatch][2]==0)
				completematch=lastmatch;
			a=128;
			do
			{
				b=maxmatch+a;
				if(b<4096)
					if(dict[b][1]==d)
						if(dict[b][0]==e)
							maxmatch=b;
				a>>=1;
			} while(a);
		}
		if(c==buffer[0])
			repeats=2;
		else
			repeats=0;
		if(repeats || (lastmatch>=0) )
		{
			buffer[c_index++]=c;
			return;
		}
		*to++ = buffer[0];
		buffer[0]=c;
		completematch=-1;
		return;
	}
 
/* we fall through to here if we now have 3 or more characters (including c) */
 
	if(repeats)
	{
		if(c==buffer[0])
		{
			buffer[c_index++]=c;
			if(++repeats<33)
				return;
			else
			{
				*to++ = 223;
				*to++ = c;
				clear_buffer();
				return;
			}
		}
		if( (repeats>2) || (lastmatch<0) )
		{
			*to++ = 192+repeats-2;
			*to++ = buffer[0];
			clear_buffer();
			buffer[c_index++]=c;
			completematch=-1;
			return;
		}
		repeats=0;
	}
	if( (c<'0' || (c>'9'&&c<'a') || c>'z') && c!=39) /* char not in dict. */
	{
sendout2:
		if(completematch>=0)
		{
			b=0;
			if((e=buffer[c_index-1])>='a') e-=32;
			if(dict[completematch][c_index-1]==e)
				++b;
			a=128|(completematch>>8);
			if( (buffer[0]>='A') && (buffer[0]<='Z') )
				a|=32;
			if(c==' ')
			{
				if(b)
					a|=16;
			}
			else
			{
				if(dict[completematch][2]==0)
					goto nomatch2;
			}
			if(completematch&224) /* the last bug fix! */
			{
				*to++ = a;
				*to++ = completematch&255;
			}
			else
			{
				*to++ = a|96;
				*to++ = (completematch&255)|64|(a&32);
			}
			if(b)
			{
				clear_buffer();
				if(c!=' ')
				{
					buffer[c_index++]=c;
					completematch=-1;
				}
				return;
			}
			strcpy(buffer2,buffer);
			b=c_index;
			clear_buffer();
			a=1;
			do
			{
				a++;
				if((e=buffer2[a])>='a') e-=32;
			} while(e==dict[completematch][a]);
			for(;a<b;a++)
				compression_filter(buffer2[a]);
			compression_filter(c);
			return;
		}
nomatch2:
		strcpy(buffer2,buffer);
		*to++ = buffer2[0];
		b=c_index;
		clear_buffer();
		for(a=1;a<b;a++)
			compression_filter(buffer2[a]);
		compression_filter(c);
		return;
	}
 
/* binary search here might save some amount of time.
   would probably pay to invoke it only if we're on the third character,
   where with the current dictionary we may be scanning through as many as
   100 words.  By the fourth & fifth characters, the benefits drop off
   and the extra overhead of a fancier search probably isn't worthwhile.
*/
	if(lastmatch<maxmatch)
		if(dict[lastmatch][c_index]!=d)
			do
			{
				++lastmatch;
			} while ( (dict[lastmatch][c_index]!=d) && (lastmatch<maxmatch) );
	if(maxmatch>lastmatch)
		if(dict[maxmatch][c_index]!=d)
			do
			{
				--maxmatch;
			} while ( (dict[maxmatch][c_index]!=d) && (lastmatch<maxmatch) );
	if(dict[lastmatch][c_index+1]==0)
		if(dict[lastmatch][c_index]==d)
			completematch=lastmatch;
	if(lastmatch==maxmatch)
		if(dict[lastmatch][c_index]!=d)
			goto sendout2;
	buffer[c_index++]=c;
	return;
}
 
void pawprint(void)
{
	special_case=0;
	strcpy(chksum_buf,"Iuvexomnz Totkzkkt-totkze-lobk Jxgmut'y Kek Vxujaizouty");
}
 
const char *
uncompress(const char *s)
{
    static char buf[BUFFER_LEN];
    unsigned int i, j, mode, c;
    int limit = 4095;
 
    if (!table_initialized) {
	pawprint();
	init_compress();
    }
 
    if (!s || !compressed(s))
	return s;		/* already uncompressed */
 
    for (to = buf; *s && limit; ) {
		mode = (*s++)&255;
		if( (mode&128) == 0) {
 			*to++ = mode;
 			limit--;
 		}
 		else
		{
 			c=(*s++)&255;
			if(mode&64)
			{
				if(mode&32)
				{
					if(c&128)
					{
						mode=0; /* This is an error if this code is ever reached */
					}
					else
					{
						i=(c&31)+((mode&15)<<8);
						*to++ = dict2[i][0]-(c&32);
						limit--;
						for(j=1;dict2[i][j] && limit;j++)
						{
							*to++ = dict2[i][j];
							limit--;
						}
						if(mode&16 && limit)
						{
							*to++ = ' ';
							limit--;
						}
					}
				}
				else
					for(i=0;i<(mode&31)+2 && limit;i++)
					{
						*to++ = c;
						limit--;
					}
			}
			else
			{
				i=c+((mode&15)<<8);
				*to++ = dict2[i][0]-(mode&32);
				limit--;
				for(j=1;dict2[i][j] && limit;j++)
				{
					*to++ = dict2[i][j];
					limit--;
				}
				if(mode&16 && limit)
				{
					*to++ = ' ';
					limit--;
				}
			}
		}
    }
    *to++ = '\0';
    limit--;
    return buf;
}
 
extern short db_decompression_flag;

const char *
compress(const char *s)
{
    static char buf[BUFFER_LEN];
    int a,b,c,d,e;
    char buffer2[40];
    int done = 0;
 
    if (!table_initialized) {
	pawprint();
	init_compress();
    }

    if (db_decompression_flag) {
	return s;
    }
 
    if (!s || compressed(s))
	return s;		/* already compressed */
 
    for (to = buf; !done; ) {
 
		if( (c=*s++) == 0)
			done=1;
		if(!c_index)
		{
			buffer[c_index++]=c;
			completematch=-1;
			goto nextchar1;
		}
		if((d=c)>='a') d-=32;
		if(c_index==1)
		{
			if((e=buffer[0])>='a') e-=32;
			if(c==39)
				if(e=='I')
				{
					maxmatch=lastmatch=special_case;
					goto here;
				}
			if(e<48||e>90||d<48||d>90 || ((c>='A')&&(c<='Z')) )
				lastmatch=-1;
			else if( (maxmatch=lastmatch=seconds[e-48][d-48]) != -1)
			{
here:
				if(dict[lastmatch][2]==0)
					completematch=lastmatch;
				a=128;
				do
				{
					b=maxmatch+a;
					if(b<4096)
						if(dict[b][1]==d)
							if(dict[b][0]==e)
								maxmatch=b;
					a>>=1;
				} while(a);
			}
			if(c==buffer[0])
				repeats=2;
			else
				repeats=0;
			if(repeats || (lastmatch>=0) )
			{
				buffer[c_index++]=c;
				goto nextchar1;
			}
			*to++ = buffer[0];
			buffer[0]=c;
			completematch=-1;
			goto nextchar1;
		}
	
/* we fall through to here if we now have 3 or more characters (including c) */
	
		if(repeats)
		{
			if(c==buffer[0])
			{
				buffer[c_index++]=c;
				if(++repeats<33)
					goto nextchar1;
				else
				{
					*to++ = 223;
					*to++ = c;
					clear_buffer();
					goto nextchar1;
				}
			}
			if( (repeats>2) || (lastmatch<0) )
			{
				*to++ = 192+repeats-2;
				*to++ = buffer[0];
				clear_buffer();
				buffer[c_index++]=c;
				completematch=-1;
				goto nextchar1;
			}
			repeats=0;
		}
		if( (c<'0' || (c>'9'&&c<'a') || c>'z') && c!=39) /* char not in dict. */
		{
sendout:
			if(completematch>=0)
			{
				b=0;
				if((e=buffer[c_index-1])>='a') e-=32;
				if(dict[completematch][c_index-1]==e)
					++b;
				a=128|completematch>>8;
				if( (buffer[0]>='A') && (buffer[0]<='Z') )
					a|=32;
				if(c==' ')
				{
					if(b)
						a|=16;
				}
				else
				{
					if(dict[completematch][2]==0)
						goto nomatch;
				}
				if(completematch&224) /* the last bug fix! */
				{
					*to++ = a;
					*to++ = completematch&255;
				}
				else
				{
					*to++ = a|96;
					*to++ = (completematch&255)|64|(a&32);
				}
				if(b)
				{
					clear_buffer();
					if(c!=' ')
					{
						buffer[c_index++]=c;
						completematch=-1;
					}
					goto nextchar1;
				}
				strcpy(buffer2,buffer);
				b=c_index;
				clear_buffer();
				a=1;
				do
				{
					a++;
					if((e=buffer2[a])>='a') e-=32;
				} while(e==dict[completematch][a]);
				for(;a<b;a++)
					compression_filter(buffer2[a]);
				compression_filter(c);
				goto nextchar1;
			}
nomatch:
			strcpy(buffer2,buffer);
			*to++ = buffer2[0];
			b=c_index;
			clear_buffer();
			for(a=1;a<b;a++)
				compression_filter(buffer2[a]);
			compression_filter(c);
			goto nextchar1;
		}
	
/* binary search here might save some amount of time.
   would probably pay to invoke it only if we're on the third character,
   where with the current dictionary we may be scanning through as many as
   100 words.  By the fourth & fifth characters, the benefits drop off
   and the extra overhead of a fancier search probably isn't worthwhile.
*/
		if(lastmatch<maxmatch)
			if(dict[lastmatch][c_index]!=d)
				do
				{
					++lastmatch;
				} while ( (dict[lastmatch][c_index]!=d) && (lastmatch<maxmatch) );
		if(maxmatch>lastmatch)
			if(dict[maxmatch][c_index]!=d)
				do
				{
					--maxmatch;
				} while ( (dict[maxmatch][c_index]!=d) && (lastmatch<maxmatch) );
		if(dict[lastmatch][c_index+1]==0)
			if(dict[lastmatch][c_index]==d)
				completematch=lastmatch;
		if(lastmatch==maxmatch)
			if(dict[lastmatch][c_index]!=d)
				goto sendout;
		buffer[c_index++]=c;
nextchar1: ;
   }
 
/* copy the last characters (if any) and null */
 
   c_index = 0;
   while ((*to++ = buffer[c_index++]));
   clear_buffer();
 
   return buf;
}
