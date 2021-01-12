////////////////////////////////////////////////////////////////////////
//  (MFC independent)
//  class CNotifyIcon 
//  Version 1.07 (Sept 21 2001)
//  Igor_Vartanov, FIDO: 2:5025/16.26 AKA /38.70
//               e-mail: ivartanov@rsdn.ru
//
////////////////////////////////////////////////////////////////////////
#ifndef IV_NOTIFYICON_class_H
#define IV_NOTIFYICON_class_H


#ifdef _DLL
    #define NI_API __declspec(dllexport)
#else
    #define NI_API 
#endif

#pragma comment(lib,"shell32")

typedef void (*MOUSE_HANDLER   )(HWND hWnd, UINT uID);
typedef BOOL (*CX_MENU_CALLBACK)(HWND hWND, UINT uID, HMENU& hMenu, UINT& uFlags, int& nSubMenu);


class NI_API CNotifyIcon
{
  public:
    const UINT m_uRegisteredMessage;
	
	CNotifyIcon( HWND hWnd );
    CNotifyIcon(
                         HWND      hWnd, 
                         UINT      nIDRes, 
                         UINT      uRegisteredMessage = 0,
                         HINSTANCE hInst = NULL,
                         LPCTSTR   szTip = NULL);
    CNotifyIcon(
                         HWND      hWnd, 
                         UINT      nID, 
                         HINSTANCE hInst,
                         LPCTSTR   szIconRes,
                         LPCTSTR   szTip     = NULL,
                         LPCTSTR   szMenuRes = NULL,
                         UINT      uRegisteredMessage = 0);
    CNotifyIcon(
                         HWND      hWnd, 
                         UINT      nID, 
                         HINSTANCE hInst,
                         UINT      uIconRes,
                         LPCTSTR   szTip    = NULL,
                         UINT      uMenuRes = 0,
                         UINT      uRegisteredMessage = 0);
    virtual ~CNotifyIcon();
    BOOL Modify(HINSTANCE hInst, LPCTSTR szIconRes, LPCTSTR szTip);
    BOOL Modify(HINSTANCE hInst, UINT uIconRes, LPCTSTR szTip);
    BOOL Remove();
    BOOL Restore();
    BOOL SetTip(LPCTSTR szTip);
    BOOL SetTip(HINSTANCE hInst, UINT nTipRes);
    BOOL IsAdded();
    void SetMenu(HINSTANCE hInst, LPCTSTR lpszMenuResource);
	void SetMenu(HINSTANCE hInst, UINT uMenuResource);
    virtual void Dispatch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static  UINT DefMessageID();

    // Message handlers
    MOUSE_HANDLER    OnRButtonUp;
    CX_MENU_CALLBACK AdjustNIContextMenu;

  protected:
    const HWND m_hWnd;
    const UINT m_uID;
    HICON      m_hIcon;
    LPTSTR     m_lpszTip;
    HINSTANCE  m_hInst;
    LPCTSTR    m_lpszMenu;
    BOOL       m_bAdded;
    void OnNIContextMenu(HWND hWnd, UINT uID);

  private:
    BOOL Add(HICON hIcon, LPCTSTR szTip);
    BOOL Add(HINSTANCE hInst, LPCTSTR szIconRes, LPCTSTR szTip);
    BOOL Modify(HICON hIcon, LPCTSTR szTip);
    void StoreTip(LPCTSTR lpszTip);
};

#endif