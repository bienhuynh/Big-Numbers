#include "pch.h"

const unsigned char String::upperCaseTranslate[256] = {
   0,     1,     2,     3,    4,     5,     6,     7,    8,     9,    10,    11,   12,    13,    14,    15, 
  16,    17,    18,    19,   20,    21,    22,    23,   24,    25,    26,    27,   28,    29,    30,    31, 
  32,    33,    34,    35,   36,    37,    38,    39,   40,    41,    42,    43,   44,    45,    46,    47, 
  48,    49,    50,    51,   52,    53,    54,    55,   56,    57,    58,    59,   60,    61,    62,    63, 
  64,    65,    66,    67,   68,    69,    70,    71,   72,    73,    74,    75,   76,    77,    78,    79, 
  80,    81,    82,    83,   84,    85,    86,    87,   88,    89,    90,    91,   92,    93,    94,    95, 
  96,    65,    66,    67,   68,    69,    70,    71,   72,    73,    74,    75,   76,    77,    78,    79, 
  80,    81,    82,    83,   84,    85,    86,    87,   88,    89,    90,   123,  124,   125,   126,   127, 
 128,   129,   130,   131,  132,   133,   134,   135,  136,   137,   138,   139,  140,   141,   142,   143, 
 144,   145,   146,   147,  148,   149,   150,   151,  152,   153,   154,   155,  156,   157,   158,   159, 
 160,   161,   162,   163,  164,   165,   166,   167,  168,   169,   170,   171,  172,   173,   174,   175, 
 176,   177,   178,   179,  180,   181,   182,   183,  184,   185,   186,   187,  188,   189,   190,   191, 
 192,   193,   194,   195,  196,   197,   198,   199,  200,   201,   202,   203,  204,   205,   206,   207, 
 208,   209,   210,   211,  212,   213,   214,   215,  216,   217,   218,   219,  220,   221,   254,   223, 
 192,   193,   194,   195,  196,   197,   198,   199,  200,   201,   202,   203,  204,   205,   206,   207, 
 208,   209,   210,   211,  212,   213,   214,   247,  216,   217,   218,   219,  220,   221,   254,   255
};

static unsigned char _lowerCaseTranslate[256];
const unsigned char *String::lowerCaseTranslate = _lowerCaseTranslate;

class InitLowerCase {
public:
  InitLowerCase();
};

InitLowerCase::InitLowerCase() {
  for(int i = 0; i < 256; i++) {
    if(String::upperCaseTranslate[i] == i) {
      _lowerCaseTranslate[i] = i;
    } else {
      _lowerCaseTranslate[i] = _lowerCaseTranslate[String::upperCaseTranslate[i]] = i;
    }
  }
}

#pragma warning(disable : 4073)
#pragma init_seg(lib)

static InitLowerCase initLowerCase;

TCHAR *streToUpperCase(TCHAR *s) {
  for(_TUCHAR *t = (_TUCHAR*)s; *t; t++) {
    if(*t < 256) {
      *t = String::upperCaseTranslate[*t];
    }
  }
  return s;
}

TCHAR *streToLowerCase(TCHAR *s) {
  for(_TUCHAR *t = (_TUCHAR*)s; *t; t++) {
    if(*t < 256) {
      *t = String::lowerCaseTranslate[*t];
    }
  }
  return s;
}

int streicmp(const TCHAR *s1, const TCHAR *s2) {
  return strtabcmp(s1, s2, String::upperCaseTranslate);
}

int strneicmp(const TCHAR *s1, const TCHAR *s2, int n) {
  return strntabcmp(s1, s2, n, String::upperCaseTranslate);
}

