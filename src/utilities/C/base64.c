/* ------------------------------------------------------------------------ *
 * file:        base64_stringencode.c v1.0                                  *
 * purpose:     tests encoding/decoding strings with base64                 *
 * author:      02/23/2009 Frank4DD                                         *
 *                                                                          *
 * source:      http://base64.sourceforge.net/b64.c for encoding            *
 *              http://en.literateprograms.org/Base64_(C) for decoding      *
 * ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

/* ---- Base64 Encoding/Decoding Table --- */
char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void decodeblockold(unsigned char in[], char *clrstr) {
  unsigned char out[4];
  out[0] = in[0] << 2 | in[1] >> 4;
  out[1] = in[1] << 4 | in[2] >> 2;
  out[2] = in[2] << 6 | in[3] >> 0;
  out[3] = '\0';
  strncat(clrstr, out, sizeof(out));
}

void decodeblock(unsigned char in[], char *clrstr, int *pos) {
  unsigned char out[4];
  out[0] = in[0] << 2 | in[1] >> 4;
  out[1] = in[1] << 4 | in[2] >> 2;
  out[2] = in[2] << 6 | in[3] >> 0;

  clrstr[*pos] = out[0]; *pos +=1;
  clrstr[*pos] = out[1]; *pos +=1;
  clrstr[*pos] = out[2]; *pos +=1;
}

void b64_decode(char *b64src, char *clrdst) {
  int c, phase, i;
  unsigned char in[4];
  char *p;
  int pos=0;

  clrdst[0] = '\0';
  phase = 0; i=0;
  while(b64src[i]) {
    c = (int) b64src[i];
    if(c == '=') {
      decodeblock(in, clrdst, &pos); 
      clrdst[pos]='\0';
      break;
    }
    p = strchr(b64, c);
    if(p) {
      in[phase] = p - b64;
      phase = (phase + 1) % 4;
      if(phase == 0) {
        decodeblock(in, clrdst, &pos);
        clrdst[pos]='\0';
        in[0]=in[1]=in[2]=in[3]=0;
      }
    }
    i++;
  }
}

/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
void encodeblockold( unsigned char in[], char b64str[], int len, int *pos ) {
    unsigned char out[5];
    out[0] = b64[ in[0] >> 2 ];
    out[1] = b64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? b64[ ((in[1] & 0x0f) << 2) |
             ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? b64[ in[2] & 0x3f ] : '=');
    out[4] = '\0';
    strncat(b64str, out, sizeof(out));
}


/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
void encodeblock( unsigned char in[], char b64str[], int len, int *pos ) {
    unsigned char out[5];
    out[0] = b64[ in[0] >> 2 ];
    out[1] = b64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? b64[ ((in[1] & 0x0f) << 2) |
             ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? b64[ in[2] & 0x3f ] : '=');

    b64str[*pos] = out[0]; *pos +=1;
    b64str[*pos] = out[1]; *pos +=1;
    b64str[*pos] = out[2]; *pos +=1;
    b64str[*pos] = out[3]; *pos +=1;
/*
    out[4] = '\0';
    strncat(b64str, out, sizeof(out));
*/
}

/* encode - base64 encode a stream, adding padding if needed */
void b64_encode(char *clrstr, char *b64dst) {
  unsigned char in[3];
  int i, len = 0;
  int j = 0;
  int pos =0;

  b64dst[0] = '\0';
  while(clrstr[j]) {
    len = 0;
    for(i=0; i<3; i++) {
     in[i] = (unsigned char) clrstr[j];
     if(clrstr[j]) {
        len++; j++;
      }
      else in[i] = 0;
    }
    if( len ) {
      encodeblock( in, b64dst, len, &pos );
    }
  }
  b64dst[pos] = '\0';
}



#ifdef ASMAIN
#define STRSIZE 1500000
//#define STRSIZE 5000
int main() {
  //char mysrc[] = "My bonnie is over the ocean...";
  char fixedstr[] = "My bonnie is over the ocean";
  char *mysrc = (char *)malloc( (STRSIZE)*sizeof(char));

  int len = strlen(fixedstr);
  int i;
  for(i=0; i < STRSIZE-1; i++) {
      mysrc[i] = fixedstr[i%len];    
  }
  mysrc[i]='\0';

  printf("created rand string\n");

  char *myb64 = (char *)malloc( (int)((float)STRSIZE*1.34)*sizeof(char));
  char *mydst = (char *)malloc( (int)((float)STRSIZE*1.34)*sizeof(char));

  b64_encode(mysrc, myb64);
 // printf("The string\n[%s]\nencodes into base64 as:\n[%s]\n", mysrc, myb64);
  printf("encoded string\n");

  b64_decode(myb64, mydst);
//  printf("The string\n[%s]\ndecodes from base64 as:\n[%s]\n", myb64, mydst);
  printf("decoded string\n");


  return 0;
}

#endif
