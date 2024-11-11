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

#include <stdio.h>
#include <unistd.h>
#include "sct_commands.h"
#include "sct_core.h"
#include "sct_utils.h"

static int ls_exec(sct_arg_t *args, int argc) {
    char *execcmd;
    if (scu_is_empty_str(args->value))
        execcmd = scu_sprintf("ls -FClg");
    else execcmd = scu_sprintf("ls -FClg %s", args->value);

    int retval = system(execcmd);
    free(execcmd);
    return retval;
}

static int pwd_exec(sct_arg_t *args, int argc) {
    char *s = getcwd(NULL, 0);
    if (!s)
    {
        perror ("Error getting current directory\n");
        return 1;
    }
    printf ("Current directory is %s\n", s);
    free(s);
    return 0; 
}

static int cd_exec(sct_arg_t *args, int argc) {
    if (chdir(args->value) == -1)
    {
      perror(args->value);
      return 1;
    }
    pwd_exec(NULL, 0);
    return 0;
}

static int grep_exec(sct_arg_t *args, int argc) {
    char *execcmd = scu_sprintf("grep %s %s", args->value, args[1].value);

    int retval = system(execcmd);
    free(execcmd);
    return retval;
}

static int ping_exec(sct_arg_t *args, int argc) {
    char *execcmd = scu_sprintf("ping -c 4 -s 64 %s", args->value);

    int retval = system(execcmd);
    free(execcmd);
    return retval;
}

static int cp_exec(sct_arg_t *args, int argc) {
    char *execcmd = scu_sprintf("cp %s %s", args->value, args[1].value);
    int retval = system(execcmd);
    free(execcmd);
    return retval;
}


void sct_init_builtin_commands(void) {
    sct_arg_t args[2] = {   SA_FILE_OR_DIR_NAME, true, NULL };
    sct_add_command("ls", args, 1, ls_exec);

    args[0].kind = SA_DIRNAME;
    args[0].optional = false;
    sct_add_command("cd", args, 1, cd_exec);

    sct_add_command("pwd", NULL, 0, pwd_exec);

    args[0].kind = SA_TEXT;
    args[1].kind = SA_FILENAME;
    args[1].optional = false;
    args[1].value = NULL;
    sct_add_command("grep", args, 2, grep_exec);

    args[0].kind = SA_INETNAME;
    sct_add_command("ping", args, 1, ping_exec);

    args[0].kind = SA_FILENAME;
    args[1].kind = SA_NEW_FILENAME;
    sct_add_command("cp", args, 2, cp_exec);
}