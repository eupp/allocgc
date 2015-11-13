#include <iostream>
#include <libprecisegc/libprecisegc.h>

class GCString {
private:
	gc_ptr<char> pData;
	int length;	 
	GCString (gc_ptr<char> p, int l) : pData (p), length (l) {};     
public:
	GCString () : length (0) {};
	GCString (const char *cString);
	GCString (gc_ptr<char> cString);
	virtual ~GCString () {};
	GCString (const GCString &s);
	GCString operator= (const GCString &s);
	GCString operator= (const char *cString);
	char operator[] (int i) {return pData [i];};
	GCString operator+  (const GCString &s);
	GCString operator+  (const char *cString);
	GCString operator+= (GCString& s);
	
	int size () {	return length;	};
	void print () {
		std::cerr<<length<<" ";
		for (int i = 0; i < length; i++) {
			std::cerr << pData[i];
		}
		std::cerr << "\n";
	};
};