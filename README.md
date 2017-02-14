# libmwg
C++03 Utilities
- `<mwg/except.h>` ... exception class and assertions
- `<mwg/str.h>` ... a range-based string
- `<mwg/xprintf.h>` ... generalized printf with variadic templates
- `<mwg/std/*>` ... C++03/11/14 standard library abstraction
- `<mwg/functor.h>` ... functional objects [obsoleted; it will be replaced by exp/fun/fun.h]
- `<mwg/bio/*>` ... binary I/O support
- `<mwg/stat/*>` ... utils for statistics including binning, histograms, and errored values


## Usage
### Compile&Install

```bash
$ git clone --recursive git@github.com:akinomyoga/libmwg.git
$ cd libmwg
$ make
$ make INSDIR=/path/to/install/prefix install
```

### Include&Link
```
$ g++ \
  -I /path/to/install/prefix/include \
  -I /path/to/install/prefix/include/i686-pc-linux-gnu-gcc-4.9.2+cxx11-release \
  -L /path/to/install/prefix/lib \
  your-files... \
  -lmwg
```

## Support
Currently it's only tested with the following C++ implementations:

- GNU/Linux (Fedora 25) 
  - icpc (ICC) 14.0.3 20140422 (`-std=c++98`)
  - g++ (GCC) 3.4.6 (`-std=c++98`)
  - g++ (GCC) 4.5.4 (`-std=c++98`)
  - g++ (GCC) 4.6.3 (`-std=c++98`)
  - g++ (GCC) 6.3.1 20161221 (Red Hat 6.3.1-1) (`-std=c++98, c++11, c++14`)
  - clang version 3.8.1 (tags/RELEASE_381/final) (Target: i686-pc-linux-gnu) (`-std=c++98, c++11, c++14`)
- Cygwin (Windows 10)
  - g++ (GCC) 5.4.0 (`-std=c++98, c++11, c++14`)
  - clang version 3.8.1 (tags/RELEASE_381/final) (Target: i686-pc-windows-cygnus) (`-std=c++98, c++11, c++14`)
