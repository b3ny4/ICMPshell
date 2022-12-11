
.PHONY: all

all: icmpshell

icmpshell: icmpshell.c
	gcc -o icmpshell icmpshell.c
