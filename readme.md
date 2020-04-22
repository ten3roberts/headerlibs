# Headerlibs
A collection of single file libaries for C

All libraries are entirely self sustaining and do not require any other files than the C standard library

All library files contain their own copy of the MIT license

Mix and match as you please

Library | Description
------------------------------- | -----
**[hashtable](hashtable.h)** 	| A dynamic hashtable supporting custom key and value types. Common types have predefined hash functions and constructors
**[magpie](magpie.h)** 			| A small and low overhead library for keeping track of allocations and detecting memory leaks from my repo [https://github.com/ten3roberts/magpie](https://github.com/ten3roberts/magpie)

## General documentation

Documentation for the specific libraries can be found withing their header file

To integrate any library, drop the single header file into your project. Define $(LIBRARY_NAME)_IMPLEMENTATION in ONE C file. See specific documentation in the header for further information

Configuring is done at build time by adding defines before including the header, refer to the specific documention for available options

```
#define HASHTABLE_IMPLEMENTATION
#define HASHTABLE_DEFAULT_SIZE 16 // Optional configuration
#include "hashtable.h"
...
```

If you use several header only libraries in a project, you can create a C file that defines all implementations of the libraries

See the specific documentation for further details and usage

##
You may recognize this pattern from the amazing STB libraries by Sean Barret [https://github.com/nothings/stb](https://github.com/nothings/stb)