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
#include "test_sct_utils.h"
#include "sct_utils.h"

bool perform_test_sct_utils(void) {
    printf("testing scu_validate_hostname_or_ip()...\n");
    bool succeeded = true;

    if (!scu_validate_hostname_or_ip("8.8.8.8")) {
        printf("\t scu_validate_hostname_or_ip(\"8.8.8.8\") FAILED.\n");
        succeeded = false;
    }
    if (scu_validate_hostname_or_ip("8.$8.8.8")) {
        printf("\t scu_validate_hostname_or_ip(\"8.a8.8.8\") FAILED -- ." \
            "false positive\n");
        succeeded = false;
    }
    if (!scu_validate_hostname_or_ip("ya.ru")) {
        printf("\t scu_validate_hostname_or_ip(\"ya.ru\") FAILED.\n");
        succeeded = false;
    }
    if (scu_validate_hostname_or_ip("ya?.ru")) {
        printf("\t scu_validate_hostname_or_ip(\"ya?.ru\") FAILED -- ." \
            "false positive\n");
        succeeded = false;
    }

    if (succeeded)
        printf("All sct_utils succeeded.\n");
    return succeeded;
}