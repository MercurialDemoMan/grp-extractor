CC  = gcc
SRC = main.c
OUT = grp-extractor

default:
	$(CC) $(SRC) -o $(OUT)