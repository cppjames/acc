.PHONY: build

build: acc
	gcc -O3 -Wall -Wextra -Wpedantic -std=gnu11 -flto cc.c -o acc