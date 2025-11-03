/* copy from fts5_tokenize.c */
#include <iostream>

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;

static const unsigned char sqlite3Utf8Trans1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    while( zIn<zTerm && (*zIn & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    if( c<0x80                                             \
        || (c&0xFFFFF800)==0xD800                          \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }        \
}

#define WRITE_UTF8(zOut, c) {                          \
  if( c<0x00080 ){                                     \
    *zOut++ = (unsigned char)(c&0xFF);                 \
  }                                                    \
  else if( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (unsigned char)((c>>6)&0x1F);     \
    *zOut++ = 0x80 + (unsigned char)(c & 0x3F);        \
  }                                                    \
  else if( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (unsigned char)((c>>12)&0x0F);    \
    *zOut++ = 0x80 + (unsigned char)((c>>6) & 0x3F);   \
    *zOut++ = 0x80 + (unsigned char)(c & 0x3F);        \
  }else{                                               \
    *zOut++ = 0xF0 + (unsigned char)((c>>18) & 0x07);  \
    *zOut++ = 0x80 + (unsigned char)((c>>12) & 0x3F);  \
    *zOut++ = 0x80 + (unsigned char)((c>>6) & 0x3F);   \
    *zOut++ = 0x80 + (unsigned char)(c & 0x3F);        \
  }                                                    \
}

int sqlite3Fts5UnicodeFold(int c, int eRemoveDiacritic);

extern "C" std::string unicode61(std::string in)  {
    const char *pText = in.c_str();
    int nText = in.length();
    unsigned char *zTerm = (unsigned char *) &pText[nText];
    unsigned char *zCsr = (unsigned char *) pText;
    u32 iCode;
    std::string result;
    result.reserve(nText+1);
    while (zCsr < zTerm) {
        if( *zCsr & 0x80 ){
            /* An non-ascii-range character. Fold it into the output buffer if
            ** it is a token character, or break out of the loop if it is not. */
            READ_UTF8(zCsr, zTerm, iCode);
            iCode = sqlite3Fts5UnicodeFold(iCode,2);
            if( iCode ){
                char aFold[5] = {0,0,0,0,0};
                char* zOut = aFold;
                WRITE_UTF8(zOut, iCode);
                result+=aFold;
            }
        } else {
            if (*zCsr >= 'A' && *zCsr <= 'Z') {
                result += *zCsr + 32;
            } else {
                result += *zCsr;
            }
            zCsr++;
        }
    }
    return result;
}
