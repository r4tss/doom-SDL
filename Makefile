##
# Doom
#
# @main.c
# @version 0.1

main: main.c
	cc main.c -Wall -lSDL2 -lSDL2_ttf -lm -o doom

map maker: maker.c
	cc maker.c -Wall -lSDL2 -lSDL2_ttf -o maker
# end
