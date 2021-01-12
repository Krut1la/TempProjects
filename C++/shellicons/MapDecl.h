#ifndef __MAPDECL_H__
#define __MAPDECL_H__

#include "ShellIconsAPI.h"

class SHELLICONS_API _MapUintToPtr
{
public:
	_MapUintToPtr() {}
	virtual ~_MapUintToPtr() {}

	virtual int count() = 0;

	virtual bool add(UINT nID, LPVOID pValue) = 0;
	virtual LPVOID take(UINT nID) = 0;
	virtual LPVOID drop() = 0;
	virtual LPVOID drop(UINT nID) = 0;

	virtual bool find(LPVOID pValue, UINT& nID) = 0;

	class _Enum
	{
	public:
		_Enum() {}
		virtual ~_Enum() {}

		virtual bool end() = 0;

		virtual UINT key() = 0;
		virtual LPVOID value() = 0;

		virtual bool next() = 0;

		virtual bool remove() = 0;
	};

	virtual _Enum* start() = 0;
};

#endif //__MAPDECL_H__