shell: shell.c utils.c command.c
	gcc -std=c99 -o shell shell.c utils.c command.c

clean:
	-rm -f shell
