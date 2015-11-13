SOURCE=test.cpp
CPPC=g++
LOAD_LIBRARY=-L /usr/local/lib/ -l precisegc -l pthread -l msmalloc

all: output

output: $(SOURCE)
	$(CPPC) -O0 -std=c++11 $(SOURCE) $(LOAD_LIBRARY) -o output

clean:
	rm -rf output
