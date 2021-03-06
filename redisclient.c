#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "hiredis/hiredis.h"

typedef void (*arg_func)(redisContext *c, char *key);

typedef struct argument
{
    char *cmd;
    arg_func func;
} arg;

redisContext *connectRedis(char *ip, int port, struct timeval timeout)
{
    redisContext *c;

    c = redisConnectWithTimeout(ip, port, timeout);
    if (c->err)
    {
        printf("Connection error: %s\n", c->errstr);
        exit(1);
    }
    return c;
}

void ping(redisContext *c)
{
    redisReply *reply;

    reply = redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);
}

void get(redisContext *c, char *key)
{
    redisReply *reply;

    reply = redisCommand(c, "get %s", key);

    if ( reply->str != NULL )
        printf("> %s\n", reply->str);
    else printf("> ERR key doesn't exist or wrong number of arguments for 'get' command\n");

    freeReplyObject(reply);
}

void set(redisContext *c, char *arg)
{
    redisReply *reply;
    char *key, *value;
    int keySize, valueSize;

    keySize = strchr(arg,' ')-arg;

    if ( keySize > 0 )
    {
        key = strndup(arg, keySize);
        valueSize = strlen(arg)-(strlen(key)+1);
    }

    if ( valueSize > 0 )
    {
        value = strndup(&arg[keySize+1],valueSize);

        reply = redisCommand(c,"SET %s %s", key, value);
        printf("> SET: %s\n", reply->str);
        freeReplyObject(reply);

        free(value);
    }
    else printf("> ERR wrong number of arguments for 'set' command\n");

    if ( keySize > 0 ) free(key);
}

void step(redisContext *c, int way, char *key)
{
    redisReply *reply;

    if ( way == 0 )
        reply = redisCommand(c, "incr %s", key);
    else reply = redisCommand(c, "decr %s", key);

    if ( (reply->str) == NULL )
    {
        printf("> %lld\n", reply->integer);
    }
    else
    {
        printf("> %s\n", reply->str);
    }
    freeReplyObject(reply);
}

void incr(redisContext *c, char *key)
{
    step(c, 0, key);
}

void decr(redisContext *c, char *key)
{
    step(c, 1, key);
}

void del(redisContext *c, char *key)
{
    redisReply *reply;

    reply = redisCommand(c,"del %s", key);
    printf("> %s\n", reply->str);
    freeReplyObject(reply);
}

static arg commands[] = {
    {
        .cmd = "get",
        .func = get
    },
    {
        .cmd = "set",
        .func = set
    },
    {
        .cmd = "incr",
        .func = incr
    },
    {
        .cmd = "decr",
        .func = decr
    },
    {
        .cmd = "del",
        .func = del
    }
};

int main(int argc, char *argv[]) {
    char *ip;
    int port;
    int counter = 0;
    int i, size, argPos;
    char *cmdLine;
    char *cmd;
    redisContext *c;
    struct timeval timeout = { 1, 500000 };

    if ( argc != 2 )
    {
        port = 6379;
        ip = "127.0.0.1";
        printf("no input host/port, use %s:%d\n", ip, port);
    }
    else
    {
        for ( i = 0; i < strlen(argv[1]); i++ ) //search for : to check the format of host and port
        {
            if ( argv[1][i] == ':' ) counter++;
        }

        if ( counter != 1 )
        {
            printf("error: wrong format (use 127.0.0.1:6379)\n");
            exit(1);
        }

        // split adress and port

        int iplength = strchr(argv[1],':')-argv[1];
        ip = strndup(argv[1], iplength);
        port = atoi(&argv[1][iplength+1]);
    }

    c = connectRedis(ip, port, timeout);

    ping(c); // PING server

    cmdLine = (char*)malloc(50*sizeof(char));

    while( strcmp(cmdLine, "exit") != 0 )
    {
        printf("redis: ");

        fgets(cmdLine, 50, stdin);

        if ( cmdLine[strlen(cmdLine)-1] == '\n' ) cmdLine[strlen(cmdLine)-1] = '\0';

        if ( strchr(cmdLine,' ') > 0 )
        {
            size = strchr(cmdLine,' ')-cmdLine;
            argPos = size+1;

            cmd = strndup(cmdLine, size);

            for ( i = 0; i < size; i++ )
                cmd[i] = tolower(cmd[i]);
        }
        else
        {
            cmd = strndup(cmdLine, strlen(cmdLine));
        }

        for ( i = 0; i < sizeof(commands)/sizeof(arg); i++ )
            if ( !strcmp(commands[i].cmd, cmd) )
                commands[i].func(c, &cmdLine[argPos]);

        free(cmd);
    }

    if ( argc == 2 ) free(ip);
    free(cmdLine);

    return 0;
}
