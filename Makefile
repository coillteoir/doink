build:
	gcc -Wall -Wextra -pedantic -march=native -o bin/doink src/main.c -lncurses

run: build
	./bin/doink

lint:
	astyle --suffix=none src/main.c
	cppcheck src/main.c

release: lint
	gcc -Wall -Wextra -march=native -O4 -o bin/doink src/main.c -lncurses

install: release
	cp bin/doink /usr/bin/doink
