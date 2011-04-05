gcc -fPIC -g -O2 -pipe -Wall -I../eggdrop1.6.20 -DHAVE_CONFIG_H -DMAKING_MODS -c python.c
ld -bundle -undefined error -o python.so python.o -L/usr/lib -ltcl8.5 -lm -ldl -lpthread /usr/lib/bundle1.o
cp python.so ../eggdrop1.6.20/dev/modules/
