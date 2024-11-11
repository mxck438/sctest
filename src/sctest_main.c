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
#include "sctest_build_config.h"
#include "sct_core.h"
#include "sct_utils.h"
#include "sct_commands.h"
#include "sct_example_plugin.h"

#define SCT_PROG_TITLE "SCTest"
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

static void print_welcome(void) {
    char *build_config = STRINGIZE(SCTEST_BUILD_CONFIG);
    if (scu_is_empty_str(build_config)) build_config = "default";
    printf("\n%s v.%s; build_config: %s; built: %s\n", SCT_PROG_TITLE,
        SCTEST_VERSION, build_config, SCTEST_BUILD_DATE);
}

int main(int argc, char** argv) {    
    if (!scu_initialize_utils() || !sct_initialize()) {
        printf("Unexpected error.\n");
        return 1;
    }

    print_welcome();

    sct_init_builtin_commands();
    // put additional plugin commands' initialization here
    init_example_plugin();

    sct_run();
    sct_finalize();
    scu_finalize_utils();
    return 0;
}

