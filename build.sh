#!/bin/sh

set -xe

cc -Wall -o bin/main src/main.c -lncurses
./bin/main
