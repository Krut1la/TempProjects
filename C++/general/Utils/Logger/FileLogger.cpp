#include "FileLogger.h"

#include "StringUtils.h"

namespace RTS
{
CFileLogger::CFileLogger(const std::wstring& FileName, const std::wstring& Location)
{
   m_Out.open((Location + FileName).c_str(), std::ios::trunc);

}

CFileLogger::~CFileLogger(void)
{

   m_Out.flush();
   m_Out.close();

}

void CFileLogger::Log(const std::wstring& name, const CMessageType& MessageType, UINT32 Level)
{
   if (Level < f_Threshold   ) return;

   m_Out << MessageType.GetName() << L"[" << IntToStr(Level) << "]: " << name << L"\n";
}
}