.PHONY: build

build: acc

acc: cc.c
	gcc -O3 -Wall -Wextra -Wpedantic -std=gnu11 -flto cc.c -o acc
