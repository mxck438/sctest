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

#include<readline/readline.h>
#include<readline/history.h>
#include "sct_core.h"
#include "sct_utils.h"


/*
    There is a SCT Processing Core. 
    User commands are registered with the Core
        by means of sct_add_command(...),
        providing a description of command arguments, and an execution callback.
    The Core handles all user interaction, the TAB completion of arguments
    and arguments validation depending on argument type.
    This way adding a user function is a simple task - an execution function 
    receives just as many arguments as it requested.
    Also a decent security context is established, because the Core tries to 
    eliminate any unwanted behavior by prevalidating command arguments.
    E.g. if an argument's type is a filename, we don't want it to come with
    any redirections or piping like in:
        'ls ./myfile | <arbitrary_code> > <unwanted location>'

    The Core is build around GNU readline lib. 


*/

#define SCT_USER_PROMPT "SCTest: "
#define SCT_INPUT_ID "SCTest"

typedef struct sct_command_ {
    struct sct_command_ *next;
    char *name;
    sct_exec_cb_t exec_fn;
    sct_arg_t args[SCT_MAX_ARGS];
    int argc;
} sct_command_t;

typedef struct sct_core_ {
    sct_command_t *commands;
    sct_command_t **cmd_idx;
    int cmd_count;

} sct_core_t;

typedef enum complete_kind_ {
    CK_FILENAME,
    CK_COMMAND_NAME,
    CK_NONE           
} complete_kind_t;

static sct_core_t *g_core = NULL;
static bool g_request_terminate = false;
complete_kind_t g_curr_complete_kind = CK_FILENAME;

#pragma region command helper routines
//------------------------------------------------------------------------------
//              command helper routines

static int cmp_cmds(const void *a, const void *b) {
    return strcmp((*(sct_command_t**)a)->name,(*(sct_command_t**)b)->name);
}

static sct_command_t *create_and_install_command(char *name) {
    sct_command_t *command;
    size_t sz = sizeof(*command);
    command = malloc(sz);
    if (command) {
        memset(command, 0, sz);
        command->name = scu_strdup(name);
        command->next = g_core->commands;
        g_core->commands = command;
        g_core->cmd_count++;
        // recreate index
        g_core->cmd_idx = realloc(g_core->cmd_idx, g_core->cmd_count * sz);
        if (!g_core->cmd_idx) {
            perror("Out of memory");
            exit(1);
        }
        sct_command_t *cmd = g_core->commands;
        sct_command_t **p = g_core->cmd_idx;
        while (cmd) {
            *p = cmd;
            p++;
            cmd = cmd->next;
        }
        // this is to respect our seasoned user's desire to contemplate
        // a list of completions in alphabetical order.
        qsort(g_core->cmd_idx, g_core->cmd_count, sizeof(void*), cmp_cmds);
    }
    return command;
}

// called upon finalization only
static void purge_command(sct_command_t *command) {
    free(command->name);
    for (int i = 0; i < command->argc; i++)
        free(command->args[i].value);
    free(command);
}

// Find registered command by its name.
// We could have used a hash table here, but given a small names count,
// plain search is sufficient here and might even be faster.
static sct_command_t *get_command_by_name(char *name) {
    for (int i = 0; i < g_core->cmd_count; i++) {
        if (strcmp(g_core->cmd_idx[i]->name, name) == 0)
            return g_core->cmd_idx[i];
    }
    return NULL;
}

static void reset_command_args(sct_command_t *cmd) {
    for (int i = 0; i < cmd->argc; i++) {
        if (cmd->args[i].value) {
            free(cmd->args[i].value);
            cmd->args[i].value = NULL;
        }
    }
}
#pragma endregion

#pragma region basic line parser
//------------------------------------------------------------------------------
//               basic line parser  

// Overall description. 
// We try to break a source line into a list of words.
// A word here is any string of chars inclosed in quotes or not, breaked by
// a whitespace or EOL.

typedef struct arg_word_ {
    struct arg_word_ *next;
    int index;
    char *text;
    int start;
    int end;
} arg_word_t;

typedef struct parsed_words_ {
    arg_word_t *words;
    int word_count;
} parsed_words_t;
 

static void purge_words(parsed_words_t *words) {
    while (words->words) {
        arg_word_t *w = words->words;
        words->words = words->words->next;
        free(w->text);
        free(w);
    }
    free(words);
} 

inline static bool is_whitespace(char c) {
    return ((c == ' ') || (c == '\t'));
}

// skip_whitespace() returns false if EOL had been reached
inline static bool skip_whitespace(char **p) {
    while (**p) {
        if (!is_whitespace(**p)) break;
        (*p)++;
    }
    return **p != 0;
}

// precondition: *start is a quote char
static bool find_quoted_word_break(char *start, char **end) {
    char qt = *start;
    char *p = start;
    p++;
    while (*p) {
        switch (*p)
        {
            case '\\':  // skip escaped quote
            {
                p++;
                if (*p == qt) p++;
                break;
            }
            default:
            {
                if (*p == qt) {
                    // quoted or not, the arg must terminate with a whitespace
                    p++;
                    if (!*p || is_whitespace(*p)) {
                        *end = p;
                        return true;
                    }
                }
                else p++;
                break;
            }
        }
    }
    return false;
}

static bool find_whitespace_word_break(char *start, char **end) {
    char *p = start;
    while(*p) {
        if (is_whitespace(*p)) break;
        p++;
    }
    *end = p;
    return true;
}

// find_word_break() tries to find where current word starting at 'start' ends.
// A word may start with a single quote, double quotes or any other char.
// If a word started with quotes, we search for a matching closing quote.
// If a word started with any other char, we search for terminating whitespace
// or EOL.
// find_word_break() returns false if a quoted string is not terminated
// properly.
// precondition: start is not an empty string and is not a whitespace.
static bool find_word_break(char *start, char **end) {
    switch (*start)
    {
        case '\'':
        case '"':   return find_quoted_word_break(start, end);

        default: return find_whitespace_word_break(start, end);       
    }
}

// needed because we added words in stack-like manner
static void reverse_word_order(parsed_words_t *words) {
    if (!words->words) return;
	arg_word_t *result = NULL;
	arg_word_t *p = words->words;
	while (p)
	{
		arg_word_t *next = p->next;
		p->next = result;
		result = p;
		p = next;
	}
    words->words = result;

    // renumber words
    p = words->words; 
    int idx = 0;
    while (p) {
        p->index = idx++;
        p = p->next;
    } 
}

static parsed_words_t *parse_words(char *line, bool ignore_parse_errors) {
    if (scu_is_empty_str(line)) return NULL;

    parsed_words_t *words = malloc(sizeof(*words));
    if (!words) return NULL;
    memset(words, 0, sizeof(*words));

    char *p = line;
    char *start;
    char *end;
    bool succeeded = true;
    while (*p) {
        if (!skip_whitespace(&p)) break;
        start = p;
        succeeded = find_word_break(start, &end);
        if (!succeeded) {
            if (ignore_parse_errors) {
                find_whitespace_word_break(start, &end);
            }
            else break;
        }

        arg_word_t *word = malloc(sizeof(*word));
        succeeded = word != NULL;
        if (!succeeded) break;

        word->next = words->words;
        words->words = word;
        words->word_count++;
        word->start = start - line;
        word->end = end - line;
        word->text = scu_strndup(start, word->end - word->start);
        p = end;
    }
    if (!succeeded) {
        purge_words(words);
        words = NULL;
    }
    else reverse_word_order(words);
    return words;
} 
#pragma endregion

#pragma region command parser
//------------------------------------------------------------------------------
//               command parser

static sct_command_t *command_from_words(parsed_words_t *words) {
    sct_command_t *command = NULL;
    if (words->word_count > 0) {
        char *cmd_name = words->words->text;
        command = get_command_by_name(cmd_name);
        if (command) {
            reset_command_args(command);
            arg_word_t *word = words->words->next;
            while (word) {
                int arg_idx = word->index - 1;
                if (arg_idx >= command->argc) break;
                sct_arg_t *arg = &command->args[arg_idx];
                arg->value = scu_strdup(word->text);
                word = word->next;
            }
        }
    }
    return command;
}

static bool validate_arg(sct_arg_t *arg, bool *err_printed) {
    if (!arg->value) return arg->optional;
        
    switch (arg->kind)
    {
        case SA_FILENAME: return scu_file_exists(arg->value, err_printed);
        case SA_NEW_FILENAME: return scu_validate_filename(arg->value);
        case SA_FILE_OR_DIR_NAME: return scu_file_or_dir_exists(arg->value, 
            err_printed);
        case SA_DIRNAME: return scu_directory_exists(arg->value, err_printed);
        case SA_TEXT: return true;
        case SA_INETNAME: return scu_validate_hostname_or_ip(arg->value);
        default: return false;
    }
}

static sct_command_t *parse_final_command(char *line) {
    sct_command_t *command = NULL;
    parsed_words_t *words = parse_words(line, false);
    if (words) {
        command = command_from_words(words);
        if (command) {
            bool err_printed = false;
            for (int i = 0; i < command->argc; i++) {
                if (!validate_arg(&command->args[i], &err_printed)) {                    
                    command = NULL;
                    if (!err_printed)
                        printf("Invalid argument(s).\n");
                    break;
                }
            }
        } 
        else printf("Unrecognized command.\n");
        purge_words(words);
    }
    else printf("Error while parsing command.\n");
    return command;
}
#pragma endregion

#pragma region TAB completions
//------------------------------------------------------------------------------
//             TAB completions

static int word_index_from_str_pos(parsed_words_t *words, int pos) {
    int result = -1;
    arg_word_t *word = words->words;
    while (word) {
        if (pos < word->end) return word->index;
        word = word->next;
    }
    return words->word_count;
}

static complete_kind_t resolve_comletion_kind(int start) {
    complete_kind_t result = CK_COMMAND_NAME;
    parsed_words_t *words = parse_words(rl_line_buffer, true);
    if (words) {
        int word_idx = word_index_from_str_pos(words, start);
        if (word_idx > 0) {
            sct_command_t *command = command_from_words(words);
            if (command) {
                int arg_idx = word_idx - 1;
                if (arg_idx >= command->argc) result = CK_NONE;
                else {
                    sct_arg_t *arg = &command->args[arg_idx];
                    switch (arg->kind)
                    {
                        case SA_FILENAME:
                        case SA_NEW_FILENAME:
                        case SA_FILE_OR_DIR_NAME:
                        case SA_DIRNAME:
                        {
                            result = CK_FILENAME;
                            break;
                        }
                        default:
                        {
                            result = CK_NONE;
                            break;
                        }                            
                    }
                }
            }
            else result = CK_NONE;
        }
        purge_words(words);
    }
    return result;
}

// command_names_provider() is an almost verbatim copy of GNU Readline example.
//
// Generator function for command completion. STATE lets us know whether
// to start from scratch; without any state (i.e. STATE == 0), then we
// start at the top of the list.
static char *command_names_provider(const char *text, int state)
{
    static int list_index, len;
    char *name;

    // If this is a new word to complete, initialize now.  This includes
    // saving the length of TEXT for efficiency, and initializing the index
    // variable to 0.
    if (!state)
    {
      list_index = 0;
      len = strlen(text);
    }

    // Return the next name which partially matches from the command list.  
    while (list_index < g_core->cmd_count)
    {
        name = g_core->cmd_idx[list_index]->name;
        list_index++;

        if (strncmp(name, text, len) == 0)
            return (scu_strdup(name));
    }

    // If no names matched, then return NULL.
    return NULL;
}

static char **sct_completion(char *text, int start, int end)
{
    g_curr_complete_kind = resolve_comletion_kind(start);

    switch (g_curr_complete_kind)
    {
        case CK_NONE:
        default:
        {
            rl_attempted_completion_over = 1;
            break;
        }
        
        case CK_COMMAND_NAME:
        {
            rl_attempted_completion_over = 1;
            return rl_completion_matches (text, command_names_provider);
        }

        case CK_FILENAME: break;
    }
    return NULL;
}

static int filter_completions(char **list) {
    if (g_curr_complete_kind != CK_NONE) return 0;
    char **p = list;
    while(*p) {
        free(*p);
        *p = NULL;
        p++;
    }
    return 0;
}
#pragma endregion

#pragma region public core routines
//------------------------------------------------------------------------------
//             public core routines

bool sct_initialize(void) {
    g_core = malloc(sizeof(*g_core));
    if (!g_core) return false;

    memset(g_core, 0, sizeof(*g_core));

    // allow conditional parsing of the ~/.inputrc file. 
    rl_readline_name = SCT_INPUT_ID;
    // tell the readline's completer we would handle the game
    rl_attempted_completion_function = 
        (rl_completion_func_t *)sct_completion;
    // that's the mechanism to disable default file completion in certain cases
    rl_ignore_some_completions_function = filter_completions;
    return true;
}

void sct_finalize(void) {
    while (g_core->commands) {
        sct_command_t *cmd = g_core->commands;
        g_core->commands = g_core->commands->next;
        purge_command(cmd);
    } 
    free(g_core->cmd_idx);
    free(g_core);
    g_core = NULL;
}

bool sct_add_command(char *name, sct_arg_t *args, int argc, 
    sct_exec_cb_t exec_fn)
{
    // check optional args are at the end of list only
    bool had_opt = false;
    for (int i = 0; i < argc; i++) {
        if (args[i].optional) {
            had_opt = true;
        }
        else {
            if (had_opt) {
                printf("Bad arguments for command: \"%s\"", name);
                return false;
            }
        }
    }

    if (get_command_by_name(name) != NULL) return false;
    sct_command_t *command = create_and_install_command(name);
    if (command) {
        command->exec_fn = exec_fn;
        command->argc = argc;
        for (int i = 0; i < argc; i++) {
            command->args[i] = args[i];
        }
    }
    return command != NULL;
}  

void sct_run(void) {
    while (!g_request_terminate) {
        char *line = readline(SCT_USER_PROMPT);
        if (scu_is_empty_str(line)) break;

        add_history(line);
        sct_command_t *command = parse_final_command(line);
        if (command) {
            // call actual command
            command->exec_fn(command->args, command->argc);
        }
        free(line);
    }
}

void sct_request_terminate(void) {
    g_request_terminate = true;
}

#pragma endregion