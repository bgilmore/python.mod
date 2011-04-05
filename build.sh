gcc -fPIC -g -O2 -pipe -Wall -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6 -I../eggdrop1.6.20/src/mod/wire.mod -I../eggdrop1.6.20 -DHAVE_CONFIG_H -DMAKING_MODS -c python.c
gcc -fPIC -g -O2 -pipe -Wall -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6 -I../eggdrop1.6.20/src/mod/wire.mod -I../eggdrop1.6.20 -DHAVE_CONFIG_H -DMAKING_MODS -c api.c
ld -bundle -undefined error -o python.so python.o api.o -L/usr/lib -ltcl8.5 -lm -ldl -lpthread -lpython2.6 /usr/lib/bundle1.o
cp python.so ../eggdrop1.6.20/dev/modules/
