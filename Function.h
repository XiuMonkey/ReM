
#include <unzip.h>
#include <zconf.h>
#include <string.h>
#include <zlib.h>
#include <string>
#include <iostream>
#include <strsafe.h>
#include "CJson.h"
#define WM_PLAYMUSIC (WM_USER+100)
using namespace std;

extern WNDCLASS wc;
extern HINSTANCE hInst;
extern HDC mHDC;
extern HWND JPE;
extern WNDPROC oldDEProc;
extern void OnPaintJPE();
cJSON *DataJS;

char* W2C(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	try
	{
		if (lpwszStrIn != NULL)
		{
			int nInputStrLen = wcslen(lpwszStrIn);

			// Double NULL Termination  
			int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
			pszOut = new char[nOutputStrLen];

			if (pszOut)
			{
				memset(pszOut, 0x00, nOutputStrLen);
				WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
			}
		}
	}
	catch (std::exception e)
	{
	}

	return pszOut;
}
LPWSTR C2W(char* asciiString) {

	// 获取需要的缓冲区大小
	int bufferSize = MultiByteToWideChar(CP_UTF8, 0, asciiString, -1, NULL, 0);

	// 分配缓冲区
	wchar_t* wideString = new wchar_t[bufferSize];

	// 转换为Unicode字符串
	MultiByteToWideChar(CP_UTF8, 0, asciiString, -1, wideString, bufferSize);

	return wideString;
}
LPWSTR S2W(std::string str)
{
	size_t size = str.length();
	int wLen = ::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t* buffer = new wchar_t[wLen + 1];
	memset(buffer, 0, (wLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), size, (LPWSTR)buffer, wLen);
	return buffer;
}
string WS2S(wstring wstr)
{
	string result;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	if (len <= 0)return result;
	char* buffer = new char[len + 1];
	if (buffer == NULL)return result;
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}
std::wstring GetClipboardText()
{
	std::wstring clipboardText;

	// 打开剪切板
	if (OpenClipboard(nullptr))
	{
		// 获取剪切板中的数据句柄
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData != nullptr)
		{
			// 锁定数据句柄以获取数据指针
			wchar_t* pszData = static_cast<wchar_t*>(GlobalLock(hData));
			if (pszData != nullptr)
			{
				// 将数据复制到wstring中
				clipboardText = pszData;

				// 解锁数据句柄
				GlobalUnlock(hData);
			}
		}

		// 关闭剪切板
		CloseClipboard();
	}
	return clipboardText;
}
BOOL FreeMyResource(UINT uiResouceName, char* lpszResourceType, char* lpszSaveFileName)
{
	HRSRC hRsrc = ::FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(uiResouceName), C2W(lpszResourceType));
	LPTSTR szBuffer = new TCHAR[1024];
	if (hRsrc == NULL)
	{
		return FALSE;
	}
	DWORD dwSize = ::SizeofResource(NULL, hRsrc);
	if (0 >= dwSize)
	{
		return FALSE;
	}
	HGLOBAL hGlobal = ::LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
	{
		return FALSE;
	}
	LPVOID lpVoid = ::LockResource(hGlobal);
	if (NULL == lpVoid)
	{
		return FALSE;
	}
	FILE* fp = NULL;
	fopen_s(&fp, lpszSaveFileName, "wb+");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(lpVoid, sizeof(char), dwSize, fp);
	fclose(fp);
	return TRUE;
}//释放资源文件

void playclick2() {
	mciSendString(L"play data\\01\\Click.mp3", 0, 0, 0);

}//播放按钮声音

bool unzipCurrentFile(unzFile uf, const char* destFolder)
{
	char szFilePath[512];
	unz_file_info64 FileInfo;

	if (unzGetCurrentFileInfo64(uf, &FileInfo, szFilePath, sizeof(szFilePath), NULL, 0, NULL, 0) != UNZ_OK)
		return false;

	size_t len = strlen(szFilePath);
	if (len <= 0)
	{
		return false;
	}

	std::string fullFileName = destFolder;
	fullFileName = fullFileName + "\\" + szFilePath;
	if (szFilePath[len - 1] == '\\' || szFilePath[len - 1] == '/')
	{
		_mkdir(fullFileName.c_str());
		return true;
	}
	auto file = fopen(fullFileName.c_str(), "wb");

	if (file == nullptr)
	{
		return false;
	}

	const int BUFFER_SIZE = 4096;
	unsigned char byBuffer[BUFFER_SIZE];
	if (unzOpenCurrentFile(uf) != UNZ_OK)
	{
		fclose(file);
		return false;
	}

	while (true)
	{
		int nSize = unzReadCurrentFile(uf, byBuffer, BUFFER_SIZE);

		if (nSize < 0)
		{
			unzCloseCurrentFile(uf);
			fclose(file);
			return false;
		}
		else if (nSize == 0)
		{
			break;
		}
		else
		{
			size_t wSize = fwrite(byBuffer, 1, nSize, file);
			if (wSize != nSize)
			{
				unzCloseCurrentFile(uf);
				fclose(file);
				return false;
			}
		}
	}

	unzCloseCurrentFile(uf);
	fclose(file);
	return true;
}
bool unzipFile(std::string zipFileName, std::string goalPath)
{
	unzFile uf = unzOpen64(zipFileName.c_str());
	if (uf == NULL)
		return false;

	unz_global_info64 gi;
	if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK)
	{
		unzClose(uf);
		return false;
	}

	std::string path = zipFileName;
	auto pos = path.find_last_of("/\\");
	if (pos != std::string::npos)
		path.erase(path.begin() + pos, path.end());

	for (int i = 0; i < gi.number_entry; ++i)
	{
		if (!unzipCurrentFile(uf, goalPath.c_str()))
		{
			unzClose(uf);
			return false;
		}
		if (i < gi.number_entry - 1)
		{
			if (unzGoToNextFile(uf) != UNZ_OK)
			{
				unzClose(uf);
				return false;
			}
		}
	}
	unzClose(uf);
	return true;
}

void WriteInJson(string key,string value) {
	if (cJSON_GetObjectItem(DataJS, key.c_str()) == NULL) {
		cJSON_AddItemToObject(DataJS, key.c_str(), cJSON_CreateString(value.c_str()));
	}
	else {
		cJSON_ReplaceItemInObject(DataJS, key.c_str(), cJSON_CreateString(value.c_str()));
	}
	FILE* file = fopen("data\\data.json", "w");
	fwrite(cJSON_Print(DataJS), strlen(cJSON_Print(DataJS)),1,file);
	fclose(file);
}



