.PHONY = all clean

CC = gcc
LINKERFLAG = -lncursesw -D_XOPEN_SOURCE_EXTENDED

all:
	@echo "Compiling..."
	${CC} src/fastfingers.c src/util.c src/gui.c src/typing_round.c ${LINKERFLAG} -o fastfingers

clean:
	@echo "Cleaning..."

install:
	@echo "Installing"
