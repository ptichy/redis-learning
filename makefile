redisclient:
	gcc -std=gnu99 -pedantic -g -o redisclient redisclient.c `pkg-config --cflags --libs hiredis` `pkg-config --cflags --libs glib-2.0`
