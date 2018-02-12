shell: shell.c utils.c command.c
	gcc -std=gnu99 -o shell shell.c  builtins.c utils.c command.c

clean:
	-rm -f shell
