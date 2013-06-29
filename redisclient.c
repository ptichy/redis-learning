#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>


#include "hiredis/hiredis.h"

int main(int argc, char *argv[]) {
    char *ip;
    int port;
    int counter = 0;
    int i, size, temp;
    char *cmdLine;
    char *cmd;
    char *key, *value;
    //    unsigned int j;
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
            printf("error: wrong format (use 127.0.0.1:6379)");
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
    if (c->err) {
        printf("Connection error: %s\n", c->errstr);
        exit(1);
    }

    // PING server
    reply = redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);


    cmdLine =  (char*)malloc(50*sizeof(char));

    while( strcmp(cmdLine, "exit") != 0 )
    {
        printf("redis: ");

        fgets(cmdLine, 50, stdin);

        if ( cmdLine[strlen(cmdLine)-1] == '\n' )  cmdLine[strlen(cmdLine)-1] = '\0';

        size = strchr(cmdLine,' ')-cmdLine;
        if ( strchr(cmdLine,' ') > 0 ) cmd = (char*)malloc((size+1)*sizeof(char));

        for ( i = 0; i < size; i++ ) cmd[i] = cmdLine[i];
        cmd[i] = '\0';

        for ( i = 0; i < size; i++ ) cmd[i] = tolower(cmd[i]);
        if ( strcmp(cmd,"get") == 0 )   // get the key
        {
            reply = redisCommand(c,cmdLine);
            printf("> %s\n", reply->str);
            freeReplyObject(reply);

            free(cmd);
        }

        if ( strcmp(cmd,"set") == 0 )  // set the key
        {
            size = strchr(cmdLine+strlen(cmd)+1,' ')-cmdLine-strlen(cmd);
            key = (char*)malloc(size*sizeof(char));
            for ( i = strlen(cmd)+1; i < strchr(cmdLine+strlen(cmd)+1,' ')-cmdLine; i++ ) key[i-(strlen(cmd)+1)] = cmdLine[i];
            key[i-(strlen(cmd)+1)] = '\0';
            //printf("key:%s|",key);

            temp = i+1;
            size = strlen(cmdLine)-(strlen(cmd)+strlen(key)+1);

            //printf("sizeofvalue=%d-strlencmdline:%d + strlcmd:%d +strlenkey:%d\n",size, strlen(cmdLine), strlen(cmd), strlen(key));
            value = (char*)malloc(size*sizeof(char));
            //printf("value_size:%d|\n", size);
            for ( i; i < strlen(cmdLine); i++ ) value[i-temp] = cmdLine[i];
            value[i-temp] = '\0';

            //printf("value:%s|\n", value);

            //printf("key:%s|\n", key);
            reply = redisCommand(c,"SET %s %s", key, value);
            printf("SET: %s\n", reply->str);
            freeReplyObject(reply);

            free(key);
            //free(value);
            free(cmd);
        }

        if ( strcmp(cmd,"incr") == 0 )  // increment the number
        {
            i = 0;
            counter = 0;
            while ( counter < 2 && i < strlen(cmdLine) )
            {
                if ( cmdLine[i] == ' ' ) counter++;
                i++;
            }
            if ( counter == 2 ) cmdLine[i] = '\0';

            reply = redisCommand(c,cmdLine);
            printf(">%s: %lld\n", cmdLine, reply->integer);
            freeReplyObject(reply);

            free(cmd);
        }

        if ( strcmp(cmd,"decr") == 0 )  // decrement the number
        {
            i = 0;
            counter = 0;
            // printf("command:%s|\n",cmdLine);
            while ( counter < 2 && i < strlen(cmdLine) )
            {
                if ( cmdLine[i] == ' ' ) counter++;
                i++;
            }
            //printf("counter:%d, cmdind:%d\n",counter,i);
            if ( counter == 2 ) cmdLine[i] = '\0';
            //printf("\ncommand:%s|\n",cmdLine);
            reply = redisCommand(c,cmdLine);
            printf(">%s: %lld\n", cmdLine, reply->integer);
            freeReplyObject(reply);

            free(cmd);
        }

        if ( strcmp(cmd,"del") == 0 )   // delete a key
        {
            reply = redisCommand(c,cmdLine);
            printf("> %s\n", reply->str);
            freeReplyObject(reply);

            free(cmd);
        }

    } // end of while

    free(ip);
    free(cmdLine);
    return 0;
}
