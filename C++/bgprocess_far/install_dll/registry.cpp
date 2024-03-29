#include "../include/registry.h"

DWORD install::registry::CreateRegistryValue(
        const TCHAR* rootkey, 
        const TCHAR* key, 
        const TCHAR* valuename, 
        const TCHAR* type, 
        const TCHAR* data)
{
	HRESULT hr = S_OK;

	static krutila::HKROOTs roots;
    static krutila::HKTYPEs types;

	//kru_string rootkey = _rootkey;
	//kru_string key = _key;
	//kru_string valuename = _valuename;
	//kru_string type = _type;
	//kru_string data = _data;

	HKEY hk;

	hr = RegCreateKeyEx(roots[rootkey],
		key,
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&hk,
		NULL);

	DWORD dwtype = types[type];

	if(hr == S_OK)
	{
		if(dwtype == REG_DWORD)
		{
			DWORD dwdata;
			_stscanf(data,_TEXT("%Ld"),&dwdata);
			hr = RegSetValueEx(hk,valuename,NULL,dwtype, (LPBYTE)&dwdata,sizeof(DWORD));
		}
		else
		{
			hr = RegSetValueEx(hk,valuename,NULL,dwtype, (LPBYTE)data,static_cast<DWORD>(sizeof(TCHAR)*_tcslen(data)));
		}
		RegCloseKey(hk);
	}

	return hr;
}

DWORD install::registry::DeleteRegistryKey(
		const TCHAR* _rootkey, 
		const TCHAR* _key,
		const TCHAR* _subkey)
{
	HRESULT hr = S_OK;

	static krutila::HKROOTs roots;

	kru_string rootkey = _rootkey;
	kru_string key = _key;
	kru_string subkey = _subkey;

	HKEY hk;

	hr = RegOpenKeyEx(roots[rootkey],
		key.c_str(),
		0,
		KEY_SET_VALUE,
		&hk);

	if(hr == S_OK)
	{
		hr = SHDeleteKey(hk, subkey.c_str());
		RegCloseKey(hk);
	}

	return hr;
}


DWORD install::registry::DeleteRegistryValue(
		const TCHAR* _rootkey, 
		const TCHAR* _key, 
		const TCHAR* _valuename)
{
	HRESULT hr = S_OK;

	static krutila::HKROOTs roots;

	kru_string rootkey = _rootkey;
	kru_string key = _key;
	kru_string valuename = _valuename;

	HKEY hk;

	hr = RegOpenKeyEx(roots[rootkey],
		key.c_str(),
		0,
		KEY_SET_VALUE,
		&hk);

	if(hr == S_OK)
	{
		hr = RegDeleteValue(hk, valuename.c_str());
		RegCloseKey(hk);
	}

	return hr;
}