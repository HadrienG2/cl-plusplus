# cl-plusplus
A more modern C++ wrapper to OpenCL

## What's the point?
There is such a thing as an C++ wrapper within the OpenCL standard, but it fails to leverage the latest advances in the C++ standard, such as lambdas or std::array.

In addition, the standard OpenCL C++ wrapper follows dangerous practices from the OpenCL C library, such as using zero-terminated arrays, which are a frequent source of application crashes and security exploits.

This wrapper aims to provide a more modern C++ wrapper to OpenCL which does not have these shortcomings.

## Sounds great! How do I use it?
Besides OpenCL, we now use CMake as our build system, and require CMake >= 3.1 along with a CMake-supported C++11 compiler. Once these prerequisites are met, in order to create a CLplusplus build, you can do the following

    mkdir build && cd build
    cmake ..
    make

If the build succeeds, you can quickly check that the example programs work by running them within the build directory

    make test

Once you have confirmed that the examples are working, studying their code and the wrapper's headers should give you a feel of how the wrapper works. Beyond that, a first draft of higher-quality documentation is available in the doc/ sub-directory of the source tree.

Said documentation is written using the Asciidoc format and automatically rendered into HTML for your viewing pleasure. If you have asciidoctor installed, CMake will render the documentation as part of a CLplusplus build.

## What are the current and targeted OpenCL support?
At the moment (March 2016) I am targeting OpenCL 1.2, because that is the best which my GPU vendor will accept to support, in an attempt to push forward its own competing proprietary technology.

Platform layer is complete: one can query platforms and devices, create contexts, and query context properties.

Runtime layer is coming next. With it, one will be able create command queues, submit commands, and in fact do everything which OpenCL allows on the host side.

Extension support is, at the moment, a question mark. I am most interested in extensions which add new features to OpenCL itself, rather than improving its interoperability with graphics APIs (which, from what I've read across the web, is pretty broken in most implementations anyway).
