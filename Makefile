.PHONY = all clean

CC = gcc
LINKERFLAG = -lncursesw -D_XOPEN_SOURCE_EXTENDED

all:
	@echo "Compiling..."
	${CC} fastfinger.c ${LINKERFLAG} -o fastfingers

clean:
	@echo "Cleaning..."

install:
	@echo "Installing"
