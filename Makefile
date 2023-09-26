build:
	gcc -Wall -Wextra -pedantic -march=native -o bin/doink src/main.c -lncurses

run:
	./bin/doink

lint:
	astyle src/main.c
	cppcheck src/main.c

release: lint
	gcc -Wall -Wextra -march=native -O2 -o bin/doink src/main.c -lncurses

install: release
	cp bin/doink /usr/bin/doink
