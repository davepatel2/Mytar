
all: mytar

mytar: mytar.c
	gcc -Wall -o mytar mytar.c
	
clean:
	rm *.o all