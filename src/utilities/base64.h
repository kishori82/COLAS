/* ------------------------------------------------------------------------ *
 * file:        base64_stringencode.c v1.0                                  *
 * purpose:     tests encoding/decoding strings with base64                 *
 * author:      02/23/2009 Frank4DD                                         *
 *                                                                          *
 * source:      http://base64.sourceforge.net/b64.c for encoding            *
 *              http://en.literateprograms.org/Base64_(C) for decoding      *
 * ------------------------------------------------------------------------ */
#ifndef _BASE64_ENCODING_C
#define _BASE64_ENCODING_C

#include <stdio.h>
#include <string.h>

/* ---- Base64 Encoding/Decoding Table --- */
void b64_decode(char *b64src, char *clrdst) ;

/* encode - base64 encode a stream, adding padding if needed */
void b64_encode(char *clrstr, char *b64dst) ;

#endif
