@echo off
gcc client.c -lws2_32 -L./um -o client
gcc server.c -lws2_32 -L./um -o server