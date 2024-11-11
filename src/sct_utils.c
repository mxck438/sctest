// The MIT License (MIT)
//
//  Copyright (c) 2024 Maxim Kryukov
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the “Software”), 
//  to deal in the Software without restriction, including without limitation 
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//  and/or sell copies of the Software, and to permit persons to whom the 
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included 
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//  DEALINGS IN THE SOFTWARE.

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include "sct_utils.h"

#define EMPTY_STRING(x)  ((x == NULL) || (*x == 0)) 

inline static bool is_regular_file(struct stat *finfo) {
    return  S_ISREG(finfo->st_mode);
}

inline static bool is_directory(struct stat *finfo) {
    return  S_ISDIR(finfo->st_mode);
}

bool scu_file_exists(char *fn, bool *err_printed) {
    char *rfn = scu_dequote(fn);
    if (!rfn) {
        *err_printed = true;
        printf("Empty name.\n");
        return false;
    } 

    struct stat finfo;
    bool result = stat(rfn, &finfo) == 0;
    if (!result) {
        *err_printed = true;
        perror("");
    }
    else result = is_regular_file(&finfo);

    free(rfn);
    return result;
}

bool scu_file_or_dir_exists(char *fn, bool *err_printed) {
    char *rfn = scu_dequote(fn);
    if (!rfn) {
        *err_printed = true;
        printf("Empty name.\n");
        return false;
    } 

    struct stat finfo;
    bool result = stat(rfn, &finfo) == 0;
    if (!result) {
        *err_printed = true;
        perror("");
    }
    else result = (is_regular_file(&finfo) || is_directory(&finfo));

    free(rfn);
    return result;
}

bool scu_directory_exists(char *fn, bool *err_printed) {
    char *rfn = scu_dequote(fn);
    if (!rfn) {
        *err_printed = true;
        printf("Empty name.\n");
        return false;
    } 

    struct stat finfo;
    bool result = stat(rfn, &finfo) == 0;
    if (!result) {
        *err_printed = true;
        perror("");
    }
    else result = is_directory(&finfo);

    free(rfn);
    return result;
}

static char *build_match_mask(bool allow_all, bool allow_alpha, 
    bool allow_numeric, char *allow_chars, char *disallow_chars)
{
    char *result = malloc(256);
    if (!result) return NULL;
    if (allow_all) memset(result, 1, 256);
    else {
        memset(result, 0, sizeof(*result));

        if (allow_numeric) {
            result['0'] = 1;
            result['1'] = 1;
            result['2'] = 1;
            result['3'] = 1;
            result['4'] = 1;
            result['5'] = 1;
            result['6'] = 1;
            result['7'] = 1;
            result['8'] = 1;
            result['9'] = 1;        
        }
        if (allow_alpha) {
            for (int i = 0; i < 256; i++)
                if (isalpha((char)i)) result[i] = 1;
        }
        if (allow_chars) {
            size_t len = strlen(allow_chars);
            for (int i = 0; i < len; i++) result[allow_chars[i]] = 1;
        }
    }
    if (disallow_chars) {
        size_t len = strlen(disallow_chars);
            for (int i = 0; i < len; i++) result[disallow_chars[i]] = 0;
    }

    return result;
}    

// returns true if every char in s is checked in mask
static bool match_mask(char *s, char *mask) {
    size_t len = strlen(s);
    for (int i = 0; i < len; i++)
        if (!mask[s[i]]) return false; 
    return true;
}

static char *g_ip4_mask = NULL;
static char *g_hostname_mask = NULL;
static char *g_ip6_mask = NULL;
static char *g_filename_mask = NULL;

bool scu_initialize_utils(void) {
    g_ip4_mask = build_match_mask(false, false, true, ".", "");
    g_hostname_mask = build_match_mask(false, true, true, "-_.", "");
    g_ip6_mask = build_match_mask(false, false, true, ":abcdefABCDEF", "");
    g_filename_mask = build_match_mask(true, true, true, "", "<>|&");

    if (!g_ip4_mask || !g_hostname_mask || !g_ip6_mask || !g_filename_mask) 
        return false;

    return true;
}

void scu_finalize_utils(void) {
    free(g_ip4_mask);
    g_ip4_mask = NULL;
    free(g_hostname_mask);
    g_hostname_mask = NULL;    
    free(g_ip6_mask);
    g_ip6_mask = NULL;
    free(g_filename_mask);
    g_filename_mask = NULL;    
}

bool scu_validate_filename(char *s) {
    return !scu_is_empty_str(s) && match_mask(s, g_filename_mask);
}

static bool scu_is_hostname(char *s) {
    return !scu_is_empty_str(s) && match_mask(s, g_hostname_mask);
}

static bool scu_is_ip4_name(char *s) {
    return !scu_is_empty_str(s) && match_mask(s, g_ip4_mask);
}

static bool scu_is_ip6_name(char *s) {
    return !scu_is_empty_str(s) && match_mask(s, g_ip6_mask);
}

// We do not actually check a name semantics, just check if all characters
// in a string are valid for a certain name type.
// So addresses like '.1023.15.2' or '_mysite..com' will pass this test.
bool scu_validate_hostname_or_ip(char *s) {
    return scu_is_hostname(s) || scu_is_ip4_name(s) || scu_is_ip6_name(s);
}

bool scu_is_empty_str(char *s) {
    if (EMPTY_STRING(s)) return true;
    size_t len = strlen(s);
    while (s) {
        if ((*s != ' ') && (*s != '\t')) return false;
        s++;
    }
    return true;
}

char *scu_strdup(char *s) {
	if (EMPTY_STRING(s))
		return NULL;
	size_t len = strlen(s);
	char *result = (char*)malloc(len + 1);
	if (result)
	{
		strcpy(result, s);
	}
	return result;
}

char *scu_strndup(char *s, size_t n)
{
	if (EMPTY_STRING(s) || (n == 0))
		return NULL;
	char *result = malloc(n + 1);
    if (result) {
        strncpy(result, s, n);
        result[n] = 0;
    }
	return result;
}

char *scu_sprintf(char *fmt, ...) {
	va_list arglist;
	va_start(arglist, fmt);
	va_list arglist2;
	va_copy(arglist2, arglist);
	size_t new_sz = vsnprintf(NULL, 0, fmt, arglist);
	char *result = NULL;
	if (new_sz) {
		result = malloc(new_sz + 1);
		if (result) {
			if (0 >= vsnprintf(result, new_sz + 1, fmt, arglist2)) {
				free(result);
				result = NULL;
			}
			else result[new_sz] = 0;
		}
	}
	va_end(arglist);
	va_end(arglist2);
	return result;
}

inline static bool is_quote_char(char c) {
    return (c == '\'') || (c == '"');
}

char *scu_dequote(char *s) {
    if (EMPTY_STRING(s)) return NULL;
    size_t len = strlen(s);
    if ((len > 1) && is_quote_char(*s) && (*s == s[len - 1])) {
        len -= 2;
        s++;
        return len ? scu_strndup(s, len) : NULL;
    }
    else return scu_strdup(s);
}