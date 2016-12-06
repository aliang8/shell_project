compile: shell.c
	gcc shell.c -lreadline
run: ./a.out
	./a.out
clean:
	rm *~
	rm a.out
