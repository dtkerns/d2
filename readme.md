# Statement of need
D2 enables computer architects and researchers to easily generate domain-specific accelerators (DSAs) given a set of input workloads based on the super block (SB) granularity. To the best of our knowledge, no such automation tool currently exists. D2 is a shell script “driver” and expects to be run from the top directory of the user’s source code to be analyzed. It takes one argument, the working directory for D2 to store its intermediate data.

# Installation Instructions

D2 is intended run on a Unix/Linux system. To build D2, cd to the src directory and type `make`, then add the D2 bin directory to your PATH

# Dependencies
D2 requires an LLVM compiler (e.g. `clang`) and `opt`. (I have only tested with clang)
Some LLVM installations install `opt` as `opt-VV` where VV is a version number. Please resolve with a symbolic link or modify the `d2` script as appropriate.
It is not required to switch your application to use clang, it is only used for SB intentification.
Other dependencies:
`awk bash cpio cut find grep python3 sed sort` and a c++ compiler, either `g++` or `clang++`

# Example usage

 - modify your project Makefiles to generate .ll and .llg files (see below)
 - run the D2 software
```
   $ PATH=$D2BASE/bin:$PATH
   $ cd $MYPRJBASE
   $ d2 /tmp/myproj
```
 - results are found in the file `sbselected` in the output directory
 - if you modify the constraints file, you can re-run without remaking the .ll/.llg files with the `-m` option

# Community guidelines

To contribute to D2, please submit your request on github or contact the authors/maintainers

# Functionality documentation

see step-by-step.md

# Benchmarks

Quantitative analysis is available as an appendix. see paper/appendix.md 
