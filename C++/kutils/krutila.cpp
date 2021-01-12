#include "../include/krutila.h"

STRINGLIST split(const kru_string& source, TCHAR delim) 
{
    STRINGLIST result;

    std::basic_istringstream<TCHAR> stream(source);
    kru_string str;

    while (std::getline(stream, str, delim))
    {
        result.push_back(str);
    }

    return result;
}

void cut_back_slash(std::string& str)
{
	if(str[str.length() - 1] == '\\') str.erase(str.length() - 1, 1);
}

void cut_back_slash(std::wstring& str)
{
	if(str[str.length() - 1] == L'\\') str.erase(str.length() - 1, 1);
}

void add_back_slash(std::string& str)
{
	if(str[str.length() - 1] != '\\') str.append("\\");
}

void add_back_slash(std::wstring& str)
{
	if(str[str.length() - 1] != L'\\') str.append(L"\\");
}