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
#include <stdlib.h>

typedef enum sct_arg_kind_ {
    SA_FILENAME,            // file must exist
    SA_NEW_FILENAME,        // may or may not exist
    SA_FILE_OR_DIR_NAME,    // must exist
    SA_DIRNAME,             // must exist
    SA_TEXT,
    SA_INETNAME
} sct_arg_kind_t;

typedef struct sct_arg_ {
    sct_arg_kind_t kind;
    bool optional;
    char *value;
} sct_arg_t;

#define SCT_MAX_ARGS 2

typedef int (*sct_exec_cb_t)(sct_arg_t *args, int argc);

bool sct_initialize(void);
void sct_finalize(void);
bool sct_add_command(char *name, sct_arg_t *args, int argc, 
    sct_exec_cb_t exec_fn);
void sct_run(void);
void sct_request_terminate(void);