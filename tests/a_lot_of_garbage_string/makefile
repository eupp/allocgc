SOURCE=a_lot_of_garbage.cpp GCString.cpp
CPPC=g++
LOAD_LIBRARY=-L /usr/local/lib/ -l precisegc -l pthread -l msmalloc

all: output

output: $(SOURCE)
	$(CPPC) -std=c++11 -O0 $(SOURCE) $(LOAD_LIBRARY) -o output

clean:
	rm -rf output
