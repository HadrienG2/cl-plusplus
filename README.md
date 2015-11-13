# cl-plusplus
A more modern C++ wrapper to OpenCL

## What's the point?
There is such a thing as an C++ wrapper within the OpenCL standard, but it fails to leverage latest advances in the C++ standard, such as lambdas or std::bind.

In addition, the standard OpenCL C++ wrapper keeps following dangerous practices from the OpenCL C library, such as the use of zero-terminated arrays, which are a frequent source of application crashes and security exploits.

This wrapper aims to provide a more modern C++ wrapper to OpenCL which does not have these shortcomings.

## Sounds great! How do I use it?
First, you should check your OpenCL installation by building and running the example applications within the repository's root directory.

All that should be needed, besides a working OpenCL installation is a relatively recent release of g++, and GNU Make.

A makefile is provided to automate the build process. It is not very pretty on the inside (.o files are scattered everywhere and modifying a single header rebuilds the entire project), but it gets the job done. If someone wishes to contribute a makefile with better dependency tracking, that would be most welcome.
