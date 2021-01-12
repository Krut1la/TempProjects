#include "StringUtils.h"

namespace General
{
ANSI_STRING
WideToAnsi(const STRING& src)
{
    std::string ret;
    ret.resize(src.size());
    for (unsigned int i=0; i<src.size(); i++) {
        ret[i] = (src[i] < 256 ? static_cast<char>(src[i]) : ' ');
    }

    return ret;
}

template<class T>
STRING IntToStr(T Value) 
{
    if(Value == 0) {
        return L"0";
    }
    
    wchar_t Buffer[21];
    wchar_t Result[21];
    unsigned int BufferPos = 0;
    unsigned int RetPos = 0;

    if(Value < 0) {
        Result[RetPos++] = '-';
    }
    
    // convert to string in reverse order
    while(Value != 0) {
        T Mod = Value % 10;
        Value = Value / 10;
        Buffer[BufferPos++] = '0' + static_cast<wchar_t>(Mod);
    }
    
    // now reverse the string to get it in proper order
    while(BufferPos > 0) {
        Ret[RetPos++] = Buffer[--BufferPos];
    }
    Ret[RetPos] = 0;
    
    return Result;
}


STRING_LIST Split( const STRING& str, const STRING& delims, unsigned int maxSplits)
{
   STRING_LIST Result;
   // Pre-allocate some space for performance
   Result.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

   unsigned int numSplits = 0;

   // Use STL methods 
   size_t start, pos;
   start = 0;
   do 
   {
      pos = str.find_first_of(delims, start);
      if (pos == start)
      {
            // Do nothing
            start = pos + 1;
      }
      else if (pos == STRING::npos || (maxSplits && numSplits == maxSplits))
      {
            // Copy the rest of the string
            Result.push_back( str.substr(start) );
            break;
      }
      else
      {
            // Copy up to delimiter
            Result.push_back( str.substr(start, pos - start) );
            start = pos + 1;
      }
      // parse up to next real data
      start = str.find_first_not_of(delims, start);
      ++numSplits;

   } while (pos != STRING::npos);
          
   return Result;
}

}