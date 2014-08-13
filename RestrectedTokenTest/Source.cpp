#include <iostream>
#include <sstream>
#include <windows.h>
#include <Sddl.h>

using namespace std;

BOOL ConvertSidToName(PSID pSid, LPTSTR lpszName, DWORD dwSizeName)
{
	TCHAR        szDomainName[256];
	DWORD        dwSizeDomain = sizeof(szDomainName) / sizeof(TCHAR);
	SID_NAME_USE sidName;

	return LookupAccountSid(nullptr, pSid, lpszName, &dwSizeName, szDomainName, &dwSizeDomain, &sidName);
}

int main(void)
{
	HANDLE hToken;
	HANDLE hTokenRestricted = nullptr;
	HANDLE hFile;
	DWORD dwLength;
	PTOKEN_USER pTokenUser;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
		MessageBox(nullptr, TEXT("トークンのハンドルの取得に失敗しました。"), nullptr, MB_ICONWARNING);
		return 0;
	}

	GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLength);
	pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);
	GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength);

	if (!CreateRestrictedToken(hToken, DISABLE_MAX_PRIVILEGE, 1, &pTokenUser->User, 0, nullptr, 0, nullptr, &hTokenRestricted)) {
		auto error = GetLastError();
		MessageBox(nullptr, TEXT("制限付きトークンの作成に失敗しました。"), nullptr, MB_ICONWARNING);
		CloseHandle(hToken);
		return 0;
	}

	LPTSTR StringSid;
	ConvertSidToStringSid(pTokenUser->User.Sid, &StringSid);
	cout << StringSid << endl;


	DWORD dwgLength;
	GetTokenInformation(hToken, TokenGroups, nullptr, 0, &dwgLength);
	auto pTokenGroups = (PTOKEN_GROUPS)LocalAlloc(LPTR, dwgLength);
	GetTokenInformation(hTokenRestricted, TokenGroups, pTokenGroups, dwgLength, &dwgLength);
	HANDLE hTokenRestricted2;

	if (!CreateRestrictedToken(hTokenRestricted, DISABLE_MAX_PRIVILEGE, pTokenGroups->GroupCount, pTokenGroups->Groups, 0, nullptr, 0, nullptr, &hTokenRestricted2)) {
		auto error = GetLastError();
		MessageBox(nullptr, TEXT("制限付きトークンの作成に失敗しました。"), nullptr, MB_ICONWARNING);
		CloseHandle(hToken);
		return 0;
	}

	//特権の制限
	DWORD dwpLength;
	GetTokenInformation(hTokenRestricted2, TokenPrivileges, nullptr, 0, &dwpLength);
	auto pTokenPrivileges = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, dwpLength);
	GetTokenInformation(hTokenRestricted2, TokenPrivileges, pTokenPrivileges, dwpLength, &dwpLength);
	HANDLE hTokenRestricted3;

	if (!CreateRestrictedToken(hTokenRestricted2, DISABLE_MAX_PRIVILEGE, 0, nullptr, pTokenPrivileges->PrivilegeCount, pTokenPrivileges->Privileges, 0, nullptr, &hTokenRestricted3)) {
		auto error = GetLastError();
		MessageBox(nullptr, TEXT("制限付きトークンの作成に失敗しました。"), nullptr, MB_ICONWARNING);
		CloseHandle(hToken);
		return 0;
	}



	ImpersonateLoggedOnUser(hTokenRestricted3);
	char str[] = "hogehoge";

	stringstream ss;
	ss << "C://CPP//" << "sample" << 0 << ".txt";
	string path = ss.str();
	hFile = CreateFile(TEXT(path.c_str()), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	DWORD dwWriteSize;
	auto x = WriteFile(hFile, str, lstrlen(str), &dwWriteSize, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_ACCESS_DENIED)
			MessageBox(nullptr, TEXT("アクセスが拒否されました。"), nullptr, MB_ICONWARNING);
		else
			MessageBox(nullptr, TEXT("ファイルの作成に失敗しました。"), nullptr, MB_ICONWARNING);
		CloseHandle(hTokenRestricted);
		CloseHandle(hToken);
		return 0;
	}

	CloseHandle(hTokenRestricted);
	CloseHandle(hFile);

	RevertToSelf();


	CloseHandle(hToken);
	LocalFree(pTokenUser);
	return 0;
}