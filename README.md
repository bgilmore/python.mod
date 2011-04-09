python.mod
==========

Tcl is awful. Seriously, _awful_. This module allows you to write Eggdrop
scripts in Python instead.


Installation
------------

1. Extract the `python.mod` directory into your Eggdrop source tree under `src/mod`
2. Edit `src/mod/python.mod/Makefile` to point to the proper include/lib paths for Python
3. From the root of the source tree, run `make iconfig` and enable the module
4. Build using `make` and/or `make install`
5. In `eggdrop.conf` use `loadmodule python` to enable the `loadpython` directive
6. Use `loadpython` to load and instantiate Python modules

