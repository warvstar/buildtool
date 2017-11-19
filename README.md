# buildtool
This is an experimental build system for C++17 with Modules support.
Keep in mind that while this works for building my projects, the code was hastely put together, it needs cleaning and performance optimizations.

Currently works with Visual Studio 2017 with the command buildtool.exe vs, check out the example project for an idea in how to use.

If using without Visual Studio, make sure the args.txt file has SourceDirs: {} with valid directories to search for source/modules.
