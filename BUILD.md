# Environment
SCTest has been successfully built and run on Debian 11 bullseye and Ubuntu 22.04 TLS jammy.
# Prerequisites
SCTest is built around GNU Readline lib, so the libreadline-dev package is required to build SCTest:

    $sudo apt install libreadline-dev

# Clone 

    $git clone /..TODO

# Build on command line

    $cd sctest
    $mkdir build
    $cd build
    $cmake ..
    $make

This will build SCTest with a default compiler setup in you system for C language.

# Build with VSCode
Open SCTest folder in VSCode. Press Ctrl+Shift+P and select CMake: Configure. 
You may select a build config. Two build configs had been defined: one for GCC and the other for Clang.
Hit F7 to build. The SCTest is built into build/<build_config>.
Two executables are built: 'sctest' and 'test_sctest'.

    $cd <build_config>

# Run unit tests
While in a build directory:

    $./test_sctest

This should deliver:

    Tests SUCCEEDED OK.

# Run
For overall description and running instructions see README.md


