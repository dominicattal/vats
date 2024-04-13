#!/bin/bash

gcc main.c
if [ $? -eq 0 ]; then
	./a.out
else
	rm status.bin
fi
