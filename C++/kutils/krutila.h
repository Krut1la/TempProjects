#ifndef KRUTILA_H
#define KRUTILA_H

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <tchar.h>

#ifdef _UNICODE
   typedef std::wstring kru_string; 
#else
   typedef std::string kru_string;
#endif //UNICODE

typedef std::vector<kru_string> STRINGLIST;

STRINGLIST split(const kru_string& source, TCHAR delim = '\n' ); 

void cut_back_slash(std::string& str);
void cut_back_slash(std::wstring& str);
void add_back_slash(std::string& str);
void add_back_slash(std::wstring& str);

template<class T,class A>
std::wstring to_unicode(
 std::basic_string<char,T,A> const& in,
 std::locale const& loc = std::locale())
{
 typedef std::codecvt<wchar_t,char,std::mbstate_t> facet;
 const facet& cvt = std::use_facet<facet>(loc);

 std::wstring out;
 out.reserve(in.length());

 facet::state_type state = facet::state_type();
 const char *ibuf = in.data(), *iend = in.data() + in.size();
 while(ibuf != iend)
 {
  wchar_t obuf[256], *oend;
  facet::result res = cvt.in(state,
   ibuf, iend, ibuf, obuf, obuf+256, oend=obuf);
  if( res == facet::error )
   { state = facet::state_type(); ibuf += 1; }
  out.append(obuf, oend - obuf);
  if( res == facet::error )
   out += L'?';
 }
 return out;
}

template<class T,class A>
std::string to_multibyte(
 std::basic_string<wchar_t,T,A> const& in,
 std::locale const& loc = std::locale())
{
 typedef std::codecvt<wchar_t,char,std::mbstate_t> facet;
 facet const& cvt = std::use_facet<facet>(loc);

 std::string out;
 out.reserve(in.length());

 facet::state_type state = facet::state_type();
 const wchar_t *ibuf = in.data(), *iend = in.data() + in.size();
 char obuf[256], *oend;
 while(ibuf != iend)
 {
  facet::result res = cvt.out(state,
   ibuf, iend, ibuf, obuf, obuf+256, oend=obuf);
  if( res == facet::error )
   { state = facet::state_type(); ibuf += 1; }
  out.append(obuf, oend - obuf);
  if( res == facet::error )
   out += L'?';
 }
 if( state == facet::partial ) {
  cvt.unshift(state, obuf, obuf+256, oend=obuf);
  out.append(obuf, oend - obuf);
 }
 return out;
}

#ifdef WIN32

#include <windows.h>
#include <tchar.h>

#endif //WIN32
#endif //KRUTILA_H