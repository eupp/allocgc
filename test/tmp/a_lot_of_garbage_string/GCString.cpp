#include <string.h>
#include "GCString.h"

GCString::GCString (const char *cString) {
	length = strlen (cString);
	pData  = gc_new<char> (length + 1);
	strcpy ((char *) pData, cString);
}

GCString::GCString (const GCString &s) : length (s.length), pData (s.pData) {}

GCString::GCString (gc_ptr<char> s) {
	length = strlen((char *)s);
	pData = gc_new<char> (length + 1);
	strcpy((char *) pData, (char *)s);
}

GCString GCString::operator= (const GCString &s) {
	length = s.length;
	pData  = s.pData;
	return *this;
}

GCString GCString::operator= (const char *cString) {
	return *this = GCString (cString);
}

GCString GCString::operator+ (const GCString &s) {
	gc_ptr<char> p = gc_new<char> (length + s.length + 1);
	strcpy (p, (char*) this->pData);
	strcat (p, (char*) s.pData);
	return GCString (p, length + s.length);
}

GCString GCString::operator+ (const char *cString) {
	return *this + GCString (cString);
}

GCString GCString::operator+= (GCString &s) {
	return *this = *this + s;
}
