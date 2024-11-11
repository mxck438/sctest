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

#pragma once

#include <stdbool.h>

bool scu_initialize_utils(void);
void scu_finalize_utils(void);

bool scu_file_exists(char *fn, bool *err_printed);
bool scu_validate_filename(char *s);
bool scu_file_or_dir_exists(char *fn, bool *err_printed);
bool scu_directory_exists(char *fn, bool *err_printed);
bool scu_validate_hostname_or_ip(char *s);
bool scu_is_empty_str(char *s);
char *scu_strdup(char *s);
char *scu_strndup(char *s, size_t n);
char *scu_sprintf(char *fmt, ...);
char *scu_dequote(char *s);