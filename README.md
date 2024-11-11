For build instructions, please see BUILD.md

# Abstract
This is the implementation of a test task for job application for SmartCom company. The test task description is in 'Test Task Description.pdf'. 
# Authorship statement
I, the author, hereby state that the Software in this project had been created by me solely. Any portions that were adapted from third-party sources had been clearly marked as such in source files. I release all files of this project to public under MIT license, and this basically means you may do anything with these files. 
# Requirements meets
The test task had been implemented entirely, including optional wishes.
Bonus: shell-like commands history, example plugin file, and three bonus commands.
# Testing
For unit tests, please see BUILD.md

While in a build directory, run the executable. SCTest enters interactive mode:

    $./sctest
    SCTest v.1.0.6-g039dc7a-CLEAN; build_config: default; built: Nov 10 2024  23:03:22
    SCTest:

You may enter a command with it's arguments or use TAB to see available completions. Completions depend on current context: e.g. if you are selecting a command, a list of available commands will be shown, or a list of files if you are to complete a filename argument.
A 'grep_test_file' is provided in build directory for convinient grep command tests.

## Important note
If the current completion is unambiguous, the completion is performed immediately.
Otherwise, the first TAB reveals nothing. You have to press TAB second time to show all possible completions. This is consistent with default shell behavior in Debian / Ubuntu.

Three additional commands had been added to demonstrate a mechanism of pluggable commands: 'q', 'quit', and 'exit'. All three quit the application upon use.
You may also quit SCTest by entering an empty line or pressing ctrl+C.
# Design
There is a SCT Processing Core. 
User commands are registered with the Core by means of sct_add_command(...), providing a description of command arguments, and an execution callback.
The Core handles all user interaction, the TAB completion of arguments and arguments validation depending on argument type.
This way adding a user function is a simple task - an execution function receives just as many arguments as it requested.
Also a decent security context is established, because the Core tries to eliminate any unwanted behavior by prevalidating command arguments.
E.g. if an argument's type is a filename, we don't want it to come with any redirections or piping like in:
    
    'ls ./myfile | <arbitrary_code> > <unwanted location>'

The Core is build around GNU readline lib. 
# Source map
### src/sctest_main.c
The driver. The main entry point. It initializes SCTest components and enters the main loop.
### src/sct_core.c
The SCT Core. Built over GNU Readline, it handles user interaction, including context-sensitive completions and invoking registered commands. Prevalidates declaired commands' arguments.
### src/sct_commands.c
Provides the implementaation of built-in commands: ls, pwd, cd, ping, grep, cp.
### src/sct_utils.c
Helper functions mainly concerning string manipulations and arguments validation.
### src/sct_example_plugin.c
Demonstrates the custom plugin implementation.
# Adding custom commands
To add a command one has to develop a command implementation file exporting a single function like init_my_command(). Place the call to this routine into main(). While in an init routine, call sct_add_command() to add your custom command. One might also want to adjust SCT_MAX_ARGS in sct_core.h if the number of arguments is greater than current limit (2). See src/sct_example_plugin.c for reference.
# Known limitations

### Quoted arguments completion is not implemented. 
This feature is not mentioned in Task, and is left as a TODO item.
You may still enclose your filename arguments in quotes and it will work.
### No CMake Installation
CMake installation section had not been developed.
### Schematic unit tests
The unit tests are somewhat limited.

