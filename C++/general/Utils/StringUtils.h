#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

namespace General
{

typedef std::string               ANSI_STRING;
typedef std::wstring              STRING;
typedef std::vector<STRING>       STRING_LIST;

ANSI_STRING 
WideToAnsi(const STRING& src);

template<class T>
STRING IntToStr(T Value); 

STRING_LIST Split( const STRING& str, const STRING& delims, unsigned int maxSplits);

}

#endif // STRINGUTILS_H