# SOURCE=test1.cpp
# SOURCE=test2.cpp
# SOURCE=test3.cpp
SOURCE=test4.cpp
CPPC=g++
LOAD_LIBRARY=-L /usr/local/lib/ -l precisegc -l pthread -l msmalloc

all: output

output: $(SOURCE)
		# $(CPPC) -Wl,--no-as-needed -std=c++11 -O0 $(SOURCE) -pthread -lpthread $(LOAD_LIBRARY) -o test1
		# $(CPPC) -Wl,--no-as-needed -std=c++11 -O0 $(SOURCE) -pthread -lpthread $(LOAD_LIBRARY) -o test2
		$(CPPC) -Wl,--no-as-needed -std=c++11 -O0 $(SOURCE) -pthread -lpthread $(LOAD_LIBRARY) -g -o test3

clean:
		# rm -rf test1
		# rm -rf test2
		rm -rf test3
