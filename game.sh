if [ -f frog ]; then
	rm frog
fi

# Build new
gcc game.c -o frog -Wno-unused -lncurses

# Run
./frog