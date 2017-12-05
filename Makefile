
CC    = g++
FLAGS = -std=c++11 -Wall -Wextra -Werror -g -pedantic -O2 -DNDEBUG

.PHONY: clean build statistics

build: chocolate

chocolate: main.o
	$(CC) $(FLAGS) -o $@ $^ -l simlib

main.o: main.cpp main.hpp
	$(CC) $(FLAGS) -c $^

clean:
	rm -f main.o chocolate

statistics:
	./chocolate 50 >statistika/statistika_50.csv
	./chocolate 55 >statistika/statistika_55.csv
	./chocolate 60 >statistika/statistika_60.csv
	./chocolate 65 >statistika/statistika_65.csv
	./chocolate 70 >statistika/statistika_70.csv
	./chocolate 75 >statistika/statistika_75.csv
	./chocolate 80 >statistika/statistika_80.csv
	./chocolate 85 >statistika/statistika_85.csv
	./chocolate 90 >statistika/statistika_90.csv
	./chocolate 95 >statistika/statistika_95.csv
	./chocolate 100 >statistika/statistika_100.csv
	./chocolate 76 >statistika/statistika_76.csv
	./chocolate 77 >statistika/statistika_77.csv
	./chocolate 78 >statistika/statistika_78.csv
	./chocolate 79 >statistika/statistika_79.csv
