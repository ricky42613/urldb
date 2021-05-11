main: db.c main.c
	gcc db.c main.c -lcrypto -lm -lz -levent -o main
clear:
	rm -rf main slice* *.gz core *.db