shell: shell.c utils.c command.c
	gcc -o shell shell.c utils.c command.c

clean:
	-rm -f shell
