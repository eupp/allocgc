SOURCE=binary_tree_sort.cpp List.cpp Tree.cpp
CPPC=g++
LOAD_LIBRARY=-L /usr/local/lib/ -l precisegc -l pthread -l msmalloc

all: output

output: $(SOURCE)
	$(CPPC) -std=c++11 -O0 $(SOURCE) $(LOAD_LIBRARY) -g -o output

clean:
	rm -rf output
