quidditch: main.c
	gcc -o quidditch main.c -pthread

clean: 
	rm quidditch
