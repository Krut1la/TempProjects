// MapImpl.h: interface for the CMapImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPIMPL_H__F94B708B_D1D8_4737_A606_E5EB5B722500__INCLUDED_)
#define AFX_MAPIMPL_H__F94B708B_D1D8_4737_A606_E5EB5B722500__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MapDecl.h"
#include <map>

class CShellIconsMap : public _MapUintToPtr
{
public:
	CShellIconsMap();
	virtual ~CShellIconsMap();

	virtual int count();

	virtual bool add(UINT nID, LPVOID pValue);
	virtual LPVOID take(UINT nID);
	virtual LPVOID drop();
	virtual LPVOID drop(UINT nID);

	virtual bool find(LPVOID pValue, UINT& nID);

	virtual _Enum* start() { return new CEnum(m_mymap); }

protected:
	typedef std::map<UINT, LPVOID> mymap;
	mymap m_mymap;

	class CEnum : public _MapUintToPtr::_Enum
	{
	public:
		CEnum(mymap& rmap): m_map(rmap), m_it(rmap.begin()) { }
		virtual ~CEnum() {}

		virtual bool end() { return m_it == m_map.end(); }

		virtual UINT key() { return m_it->first; }
		virtual LPVOID value() { return m_it->second; }

		virtual bool next()
		{
			if(end())	return false;
			++m_it;
			return true;
		}

		virtual bool remove()
		{
			if(end())	return false;
			mymap::iterator it = m_it; ++it;
			m_map.erase(it);
			m_it = it;
		}
		
	protected:
		mymap& m_map;
		mymap::iterator m_it;
	};
};

#endif // !defined(AFX_MAPIMPL_H__F94B708B_D1D8_4737_A606_E5EB5B722500__INCLUDED_)
