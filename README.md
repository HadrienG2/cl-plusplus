# cl-plusplus
A more modern C++ wrapper to OpenCL

## What's the point ?
There is such a thing as an C++ wrapper within the OpenCL standard, but it fails to leverage latest advances in the C++ standard, such as lambdas or std::bind.

In addition, the standard OpenCL C++ wrapper keeps following dangerous practices from the OpenCL C library, such as the use of zero-terminated arrays, which are a frequent source of application crashes and security exploits.

This wrapper aims to provide a more modern C++ wrapper to OpenCL which does not have these shortcomings.
