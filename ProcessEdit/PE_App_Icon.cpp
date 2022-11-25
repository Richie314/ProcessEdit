#include "pch.h"
#include "Pie_App.h"
#include <commctrl.h>
#include <commoncontrols.h>
using namespace Pie_314;
SuperAnsiString Pie_314::getFileTypeIcon(const SuperAnsiString& sFileType)
{
	if (sFileType.empty())
	{
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_INVALID_STRING);
		return "";
	}
	if (sFileType.first() != '.')
	{
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_INVALID_STRING);
		return "";
	}
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CLASSES_ROOT, sFileType.c_str(), 0,
		KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_HANDLE_ERROR);
		return "";
	}
	Dword dwType = REG_SZ;
	Dword dwSize = 64;
	SuperAnsiString sReceived(64);
	if (RegQueryValueExA(hKey, "", NULL, &dwType,
		(Byte*)(&sReceived[0]), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_HANDLE_ERROR);
		return "";
	}
	RegCloseKey(hKey);
	if (RegOpenKeyExA(HKEY_CLASSES_ROOT,
		(sReceived + "\\DefaultIcon").c_str(), 0,
		KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		setLastErrorNum(PIE_ERROR_HANDLE_ERROR);
		return "";
	}
	dwType = REG_SZ;
	dwSize = 128;
	sReceived.resize(128);
	if (RegQueryValueExA(hKey, "", NULL, &dwType,
		(Byte*)(&sReceived[0]), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		setLastErrorNum(PIE_ERROR_HANDLE_ERROR);
		return "";
	}
	RegCloseKey(hKey);
	if (sReceived.empty() || dwSize == 0)
	{
		setLastErrorNum(PIE_ERROR_INVALID_STRING | PIE_ERROR_MEMORY_ERROR);
		return "";
	}
	if (sReceived.endsWith(",0"))
	{
		sReceived.resize(dwSize - 2);
	}
	else {
		sReceived.resize(dwSize);
	}
	return std::move(sReceived);
}
SuperUnicodeString Pie_314::getFileTypeIcon(const SuperUnicodeString& sFileType)
{
	if (sFileType.empty())
	{
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_INVALID_STRING);
		return L"";
	}
	if (sFileType.first() != '.')
	{
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_INVALID_STRING);
		return L"";
	}
	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CLASSES_ROOT, sFileType.c_str(), 0,
		KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_HANDLE_ERROR);
		return L"";
	}
	Dword dwType = REG_SZ;
	Dword dwSize = 64;
	SuperUnicodeString sReceived(64);
	if (RegQueryValueExW(hKey, L"", NULL, &dwType,
		(Byte*)(&sReceived[0]), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		setLastErrorNum(PIE_ERROR_INVALID_PARAM | PIE_ERROR_HANDLE_ERROR);
		return L"";
	}
	RegCloseKey(hKey);
	if (RegOpenKeyExW(HKEY_CLASSES_ROOT,
		(sReceived + L"\\DefaultIcon").c_str(), 0,
		KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		setLastErrorNum(PIE_ERROR_HANDLE_ERROR);
		return L"";
	}
	dwType = REG_SZ;
	dwSize = 128;
	sReceived.resize(128);
	if (RegQueryValueExW(hKey, L"", NULL, &dwType,
		(Byte*)(&sReceived[0]), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		setLastErrorNum(PIE_ERROR_HANDLE_ERROR);
		return L"";
	}
	RegCloseKey(hKey);
	if (sReceived.empty() || dwSize == 0)
	{
		setLastErrorNum(PIE_ERROR_INVALID_STRING | PIE_ERROR_MEMORY_ERROR);
		return L"";
	}
	if (sReceived.endsWith(L",0"))
	{
		sReceived.resize(dwSize - 2);
	}
	else {
		sReceived.resize(dwSize);
	}
	return std::move(sReceived);
}

HIcon P_ENTRY App::getIcon(enumICON iconType)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return NULL;
	}
	SHFILEINFOW sfi;
	memset(&sfi, 0, sizeof(SHFILEINFOW));
	if (SUCCEEDED(SHGetFileInfoW((this->sLocation + this->sName).c_str(),
		0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX)))
	{
		int iImageIcon = SHIL_EXTRALARGE;
		switch (iconType)
		{
		case enumICON::LARGE_SIZE:
			iImageIcon = SHIL_LARGE;
			break;
		case enumICON::SMALL_SIZE:
			iImageIcon = SHIL_SMALL;
			break;
		case enumICON::MAX_SIZE:
			iImageIcon = SHIL_JUMBO;
		}
		IImageList* piml = PIE_NULL(IImageList);
		if (SHGetImageList(iImageIcon,
			IID_IImageList, (Memory*)&piml) == S_OK)
		{
			HICON hIcon;
			if (piml->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon) == S_OK)
			{
				return hIcon;
			}
			this->SetLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_MEMORY_ERROR | PIE_ERROR_FILE_ERROR,
				"Impossible to retreive icon correctly, error code: " + PrintHexA(GetLastError()));
			return NULL;
		}
		this->SetLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_MEMORY_ERROR | PIE_ERROR_FILE_ERROR,
			"Error in SHGetImageList: " + PrintHexA(GetLastError()));
		return NULL;
	}
	this->SetLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_MEMORY_ERROR | PIE_ERROR_FILE_ERROR,
		"Error in SHGetFileInfoW: " + PrintHexA(GetLastError()));
	return NULL;
}