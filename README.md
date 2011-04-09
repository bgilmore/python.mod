python.mod
==========

Tcl is awful. Seriously, *awful*. This module allows you to write Eggdrop
scripts in Python instead.

This module is still *very* young, immature, and untested. Still, beats
writing Tcl.


Installation
------------

1. Extract the `python.mod` directory into your Eggdrop source tree under
   `src/mod`
2. Edit `src/mod/python.mod/Makefile` to point to the proper include/lib paths
   for Python
3. From the root of the source tree, run `make iconfig` and enable the module
4. Build using `make` and/or `make install`
5. In `eggdrop.conf` use `loadmodule python` to enable the `loadpython`
   directive
6. Use `loadpython` to load and instantiate Python modules


Example Usage
-------------

#### in `eggdrop.conf`

    loadmodule python      # load python.mod
    set python-isolate 1   # use interpreter isolation? (0/1)
    loadpython splat       # load splat.py

#### in `splat.py`

    import eggdrop

    def ctcp_splat(nick, mask, handle, dest, keyword, text):
        eggdrop.putlog("Got splat'd by %s (%s)!" % (nick, mask))
        eggdrop.putserv("PRIVMSG %s :it's not nice to throw things" % nick)

    eggdrop.bind('ctcp', '-', 'splat', ctcp_splat)
    eggdrop.putlog("*** splat module loaded")

