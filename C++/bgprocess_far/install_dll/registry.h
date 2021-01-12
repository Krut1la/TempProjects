#ifndef REGISTRY_H
#define REGISTRY_H

#include "install.h"

namespace install{

	namespace registry{
		INSTALL_DLL_API DWORD CreateRegistryValue(
			const TCHAR* rootkey,
			const TCHAR* key,
			const TCHAR* valuename,
			const TCHAR* type,
			const TCHAR* data);

		INSTALL_DLL_API  DWORD DeleteRegistryValue(
			const TCHAR* rootkey, 
			const TCHAR* key, 
			const TCHAR* valuename);

		INSTALL_DLL_API  DWORD DeleteRegistryKey(
			const TCHAR* rootkey, 
			const TCHAR* key,
			const TCHAR* subkey);
	}
}

#endif //REGISTRY_H