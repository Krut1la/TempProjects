// MapImpl.cpp: implementation of the CMapImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellIconsMap::CShellIconsMap()
{
}

CShellIconsMap::~CShellIconsMap()
{
}

int CShellIconsMap::count()
{
	return m_mymap.size();
}

bool CShellIconsMap::add(UINT nID, LPVOID pValue)
{
	if(!pValue)	return false;
	std::pair<mymap::iterator, bool> pb = m_mymap.insert(std::make_pair(nID, pValue));
	return pb.second;
}

LPVOID CShellIconsMap::take(UINT nID)
{
	mymap::const_iterator it = m_mymap.find(nID);
	if(it == m_mymap.end())	return NULL;
	return it->second;
}

LPVOID CShellIconsMap::drop()
{
	if(m_mymap.empty())	return NULL;
	LPVOID pValue = m_mymap.begin()->second;
	m_mymap.erase(m_mymap.begin());
	return pValue;
}

LPVOID CShellIconsMap::drop(UINT nID)
{
	mymap::iterator it = m_mymap.find(nID);
	if(it == m_mymap.end())	return NULL;
	LPVOID pValue = it->second;
	m_mymap.erase(it);
	return pValue;
}

bool CShellIconsMap::find(LPVOID pValue, UINT& nID)
{
	mymap::const_iterator it = m_mymap.begin(), end = m_mymap.end();
	while(it != end)
	{
		if(it->second != pValue)	continue;
		nID = it->first;
		return true;
	}
	return false;
}
