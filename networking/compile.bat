@echo off
gcc client.c -lws2_32 -L./include -o client
gcc server.c -lws2_32 -L./include -o server