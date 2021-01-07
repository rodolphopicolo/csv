.PHONY: all release clean

all: 
	gcc ./src/csv.c -o ./debug/csv -ggdb
	
release:
	gcc ./src/csv.c -o ./debug/csv

clean:
	rm -rf ./debug/*
	