
CC    = g++
FLAGS = -std=c++11 -Wall -Wextra -Werror -g -pedantic -O2

.PHONY: clean build experiments run

build: chocolate

chocolate: main.o
	$(CC) $(FLAGS) -o $@ $^ -l simlib

main.o: main.cpp main.hpp
	$(CC) $(FLAGS) -c $<

example: example.o
	$(CC) $(FLAGS) -o $@ $^ -l simlib

example.o: main.cpp main.hpp
	$(CC) $(FLAGS) -DDEBUG -DEXAMPLE -O2 -c $< -o $@

porucha_cepele: porucha_cepele.o
	$(CC) $(FLAGS) -o $@ $^ -l simlib

porucha_cepele.o: main.cpp main.hpp
	$(CC) $(FLAGS) -DCEPEL -O2 -c $< -o $@

porucha_filtru: porucha_filtru.o
	$(CC) $(FLAGS) -o $@ $^ -l simlib

porucha_filtru.o: main.cpp main.hpp
	$(CC) $(FLAGS) -DFILTR -O2 -c $< -o $@

porucha_stroje: porucha_stroje.o
	$(CC) $(FLAGS) -o $@ $^ -l simlib

porucha_stroje.o: main.cpp main.hpp
	$(CC) $(FLAGS) -DSTROJ -O2 -c $< -o $@

clean:
	rm -f main.o chocolate provoz.txt example example.o porucha_stroje porucha_cepele porucha_filtru porucha_stroje.o porucha_cepele.o porucha_filtru.o

run: example
	echo "\033[0;31mSpoustim simulaci tydeniho provozu. Statistika bude na stdout, popis toho, co se deje v case bude v souboru provoz.txt\033[0m"
	./example 50 2>provoz.txt

experiments: porucha_cepele porucha_filtru porucha_stroje chocolate
	mkdir -p statistika
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
	./chocolate 74 >statistika/statistika_74.csv
	./chocolate 76 >statistika/statistika_76.csv
	./chocolate 77 >statistika/statistika_77.csv
	./chocolate 78 >statistika/statistika_78.csv
	./chocolate 79 >statistika/statistika_79.csv
	./porucha_cepele 50 >statistika/vliv_cepele.csv
	./porucha_filtru 50 >statistika/vliv_filtru.csv
	./porucha_stroje 50 >statistika/vliv_stroje.csv
