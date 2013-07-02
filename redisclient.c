#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "hiredis/hiredis.h"

void ping(redisContext *c, redisReply *reply)
{
    reply = redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);
}

void get(redisContext *c, char *cmdLine, redisReply *reply)
{
    reply = redisCommand(c,cmdLine);
    printf("> %s\n", reply->str);
    freeReplyObject(reply);
}

void set(redisContext *c, char *cmdLine, redisReply *reply, char *cmd, int size)
{
    char *key, *value;
    int i, temp;

    size = strchr(cmdLine+strlen(cmd)+1,' ')-cmdLine-strlen(cmd);
    if ( size > 0 )
    {
        key = (char*)malloc(size*sizeof(char));
        for ( i = strlen(cmd)+1; i < strchr(cmdLine+strlen(cmd)+1,' ')-cmdLine; i++ ) key[i-(strlen(cmd)+1)] = cmdLine[i];
        key[i-(strlen(cmd)+1)] = '\0';
        //printf("key:%s|",key);

        temp = i+1;
        size = strlen(cmdLine)-(strlen(cmd)+strlen(key)+1);
    }

    if ( size > 0 )
    {
        value = (char*)malloc(size*sizeof(char));
        //printf("value_size:%d|\n", size);
        for ( i; i < strlen(cmdLine); i++ ) value[i-temp] = cmdLine[i];
        value[i-temp] = '\0';
    }

    //printf("value:%s|\n", value);


    //printf("key:%s|\n", key);
    if ( size > 0 )
    {
        reply = redisCommand(c,"SET %s %s", key, value);
        printf("> SET: %s\n", reply->str);
        freeReplyObject(reply);

        free(key);
        //free(value);
    }
    else printf("> ERR wrong number of arguments for 'set' command\n");
}

void step(redisContext *c, char *cmdLine, redisReply *reply)
{
    int i, counter;

    i = 0;
    counter = 0;
    while ( counter < 2 && i < strlen(cmdLine) )
    {
        if ( cmdLine[i] == ' ' ) counter++;
        i++;
    }
    if ( counter == 2 ) cmdLine[i] = '\0';

    reply = redisCommand(c,cmdLine);
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

void del(redisContext *c, char *cmdLine, redisReply *reply)
{
    reply = redisCommand(c,cmdLine);
    printf("> %s\n", reply->str);
    freeReplyObject(reply);
}

int main(int argc, char *argv[]) {
    char *ip;
    int port;
    int counter = 0;
    int i, size;
    char *cmdLine;
    char *cmd;
    redisContext *c;
    redisReply *reply;
    struct timeval timeout = { 1, 500000 };

    if ( argc != 2 )
    {
        port = 6379;
        ip = "127.0.0.1";
        printf("no input host/port, use %s:%d\n", ip, port);
    }
    else
    {
        for ( i = 0; i < strlen(argv[1]); i++ ) //count the number of . and : to check the format of host and port
        {
            if ( argv[1][i] == ':' || argv[1][i] == '.' )   counter++;
        }

        if ( counter != 4 )
        {
            printf("error: wrong format (use 127.0.0.1:6379)\n");
            exit(1);
        }

        // split adress and port
        port = atoi(strchr(argv[1],':')+1);

        int iplength = strchr(argv[1],':')-argv[1];
        ip = (char*)malloc((iplength+1)*sizeof(char));

        for ( i = 0; i < iplength; i++ )
        {
            ip[i] = argv[1][i];
        }
        ip[i] = '\0';
    }


    c = redisConnectWithTimeout(ip, port, timeout);
    if (c->err)
    {
        printf("Connection error: %s\n", c->errstr);
        exit(1);
    }

    ping(c, reply); // PING server

    cmdLine =  (char*)malloc(50*sizeof(char));

    while( strcmp(cmdLine, "exit") != 0 )
    {
        printf("redis: ");

        fgets(cmdLine, 50, stdin);

        if ( cmdLine[strlen(cmdLine)-1] == '\n' )  cmdLine[strlen(cmdLine)-1] = '\0';

        if ( strchr(cmdLine,' ') > 0 )
        {
            size = strchr(cmdLine,' ')-cmdLine;
            cmd = (char*)malloc((size+1)*sizeof(char));
            for ( i = 0; i < size; i++ ) cmd[i] = cmdLine[i];
            cmd[i] = '\0';
            for ( i = 0; i < size; i++ ) cmd[i] = tolower(cmd[i]);
        }
        else
        {
            cmd = (char*)malloc((strlen(cmdLine)+1)*sizeof(char));
            strcpy(cmd,cmdLine);
        }

        if ( strcmp(cmd, "get") == 0 ) get(c, cmdLine, reply); // get the key

        if ( strcmp(cmd, "set") == 0 )  set(c, cmdLine, reply, cmd, size); // set the key

        if ( strcmp(cmd, "incr") == 0 || strcmp(cmd, "decr") == 0 ) step(c, cmdLine, reply); // increment/decrement the number

        if ( strcmp(cmd,"del") == 0 )  del(c, cmdLine, reply); // delete a key

        free(cmd);
    }

    if ( argc == 2 ) free(ip);
    free(cmdLine);
    return 0;
}
