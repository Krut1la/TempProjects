#ifndef REGISTRY_H
#define REGISTRY_H

    class HKTYPEs{
		std::map<kru_string, DWORD> types;
	public:
		HKTYPEs(){
			types[_TEXT("REG_BINARY")] = REG_BINARY;
			types[_TEXT("REG_DWORD")] = REG_DWORD;
			types[_TEXT("REG_SZ")] = REG_SZ;
			types[_TEXT("REG_DWORD_LITTLE_ENDIAN")] = REG_DWORD_LITTLE_ENDIAN;
			types[_TEXT("REG_DWORD_BIG_ENDIAN")] = REG_DWORD_BIG_ENDIAN;
			types[_TEXT("REG_EXPAND_SZ")] = REG_EXPAND_SZ;
			types[_TEXT("REG_LINK")] = REG_LINK;
			types[_TEXT("REG_MULTI_SZ")] = REG_MULTI_SZ;
			types[_TEXT("REG_NONE")] = REG_NONE;
			types[_TEXT("REG_QWORD")] = REG_QWORD;
			types[_TEXT("REG_QWORD_LITTLE_ENDIAN")] = REG_QWORD_LITTLE_ENDIAN;
			types[_TEXT("REG_RESOURCE_LIST")] = REG_RESOURCE_LIST;
		}

		DWORD& operator[](const kru_string& rhs)
		{
			return types[rhs];
		}
	};

	class HKROOTs{
		std::map<kru_string, HKEY> roots;
	public:
		HKROOTs(){
			roots[_TEXT("HKEY_CLASSES_ROOT")] = HKEY_CLASSES_ROOT;
			roots[_TEXT("HKEY_CURRENT_USER")] = HKEY_CURRENT_USER;
			roots[_TEXT("HKEY_LOCAL_MACHINE")] = HKEY_LOCAL_MACHINE;
			roots[_TEXT("HKEY_USERS")] = HKEY_USERS;
			roots[_TEXT("HKEY_PERFORMANCE_DATA")] = HKEY_PERFORMANCE_DATA;
			roots[_TEXT("HKEY_DYN_DATA")] = HKEY_DYN_DATA;
		}

		HKEY& operator[](const kru_string& rhs)
		{
			return roots[rhs];
		}
	};

#endif //REGISTRY_H