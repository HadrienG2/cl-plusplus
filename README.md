# cl-plusplus
A more modern C++ wrapper to OpenCL

## What's the point?
There is such a thing as an C++ wrapper within the OpenCL standard, but it fails to leverage the latest advances in the C++ standard, such as lambdas or std::array.

In addition, the standard OpenCL C++ wrapper follows dangerous practices from the OpenCL C library, such as using zero-terminated arrays, which are a frequent source of application crashes and security exploits.

This wrapper aims to provide a more modern C++ wrapper to OpenCL which does not have these shortcomings.

## Sounds great! How do I use it?
First, you should check your OpenCL installation by building and running the example applications within the repository's root directory.

All that should be needed, besides a working OpenCL installation is a relatively recent release of g++, and GNU Make.

A makefile is provided to automate the build process. It is not very pretty on the inside (.o files are scattered everywhere and modifying a single header rebuilds the entire project), but it gets the job done. If someone wishes to contribute a higher-quality makefile, with dependency tracking for example, that would be most welcome.

Once you have confirmed that the examples applications are working, studying their code and the wrapper's headers, should give you a good feel of how the wrapper works. A first draft of higher-quality documentation is available in the Documentation sub-directory of the source tree.

## What are the current and targeted OpenCL support?
At the moment (Nov. 2015) I am targeting OpenCL 1.2, because that is the best which my GPU vendor will accept to support, in an attempt to push forward its own competing proprietary technology.

Platform layer is complete: one can query platforms and devices, create contexts, and query context properties.

Runtime layer is coming next. With it, one will be able create command queues, submit commands, and in fact do everything which OpenCL allows on the host side.

Extension support is, at the moment, a question mark. I am most interested in extensions which add new features to OpenCL itself, rather than improving its interoperability with graphics APIs (which, from what I've read across the web, is pretty broken in most implementations anyway).
