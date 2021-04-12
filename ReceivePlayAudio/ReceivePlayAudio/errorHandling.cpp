#include "errorHandling.h"

//call to print an english description of the HRESULT error. 
void ErrorDescription(HRESULT hr)
{
    if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
        hr = HRESULT_CODE(hr);
    TCHAR* szErrMsg;

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&szErrMsg, 0, NULL) != 0)
    {
        _tprintf(TEXT("%s"), szErrMsg);
        LocalFree(szErrMsg);
    }
    else
        _tprintf(TEXT("[Could not find a description for error # %#x.]\n"), hr);
}