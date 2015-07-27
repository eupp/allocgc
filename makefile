all:
	g++ -std=c++11 -O0 malloc2.cpp gctest.cpp -o output

clean:
	rm -f output *.o log.out