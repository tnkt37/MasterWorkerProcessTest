#include <iostream>
#include <sstream>
#include <windows.h>
#include <Sddl.h>

//#define UP

#ifdef UP
#include "Python.h"
#endif

using namespace std;
using std::getline;

extern "C"{

}


BOOL ConvertSidToName(PSID pSid, LPTSTR lpszName, DWORD dwSizeName)
{
	TCHAR        szDomainName[256];
	DWORD        dwSizeDomain = sizeof(szDomainName) / sizeof(TCHAR);
	SID_NAME_USE sidName;

	return LookupAccountSid(nullptr, pSid, lpszName, &dwSizeName, szDomainName, &dwSizeDomain, &sidName);
}

void OutputPirivileges(PTOKEN_PRIVILEGES pTokenPrivilegest)
{
	TCHAR szProgramName[256];
	TCHAR szDisplayName[256];
	TCHAR szBuf[1024];
	for (int i = 0; i < pTokenPrivilegest->PrivilegeCount; i++) {
		auto bEnabled = pTokenPrivilegest->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED;
		DWORD dwLength = sizeof(szProgramName) / sizeof(TCHAR);
		DWORD dwLanguageId;
		LookupPrivilegeName(NULL, &pTokenPrivilegest->Privileges[i].Luid, szProgramName, &dwLength);

		dwLength = sizeof(szDisplayName) / sizeof(TCHAR);
		LookupPrivilegeDisplayName(NULL, szProgramName, szDisplayName, &dwLength, &dwLanguageId);

		wsprintf(szBuf, TEXT("%s %s %s "), bEnabled ? TEXT("○") : TEXT("×"), szProgramName, szDisplayName);
		cout << szBuf << endl;
	}
}

void waitEnter()
{
	string dummmmm;
	getline(cin, dummmmm);
}

void createFileTest()
{
	char str[] = "hogehoge";
	stringstream ss;
	ss << "C://CPP//" << "sample" << 0 << ".txt";
	string path = ss.str();
	HANDLE hFile = CreateFile(TEXT(path.c_str()), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	DWORD dwWriteSize;
	auto x = WriteFile(hFile, str, lstrlen(str), &dwWriteSize, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_ACCESS_DENIED)
			MessageBox(nullptr, TEXT("アクセスが拒否されました。"), nullptr, MB_ICONWARNING);
		else
			MessageBox(nullptr, TEXT("ファイルの作成に失敗しました。"), nullptr, MB_ICONWARNING);
	}
	CloseHandle(hFile);
}

void exePythonTest()
{
#ifdef UP
	cout << "init python" << endl;
	string dums;
	getline(cin, dums);
	Py_Initialize();


	cout << "run python" << endl;
	PyRun_SimpleString("import json\n"
		"print 'imported json'\n");
	//PyRun_SimpleString("print 'hogehoge'");
	getline(cin, dums);
	cout << "python executed" << endl;
	Py_Finalize();
	cout << "python finalized" << endl;
	getline(cin, dums);
#endif
}

HANDLE openCurrentProcessToken()
{
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
		auto error = GetLastError();
		const std::exception ex("Failed to OprnProcessToken");
		throw &ex;
	}
	return hToken;
}

template<typename T>
T getTokenInformation(HANDLE hPreToken, TOKEN_INFORMATION_CLASS tokenInfo)
{
	DWORD tokenBytes;
	GetTokenInformation(hPreToken, tokenInfo, nullptr, 0, &tokenBytes);
	auto ptokenT = (T)LocalAlloc(LPTR, tokenBytes);
	GetTokenInformation(hPreToken, tokenInfo, ptokenT, tokenBytes, &tokenBytes);

	return ptokenT;

}

HANDLE createRestrictedToken(HANDLE hPreToken, DWORD flags, DWORD disableSidCount, PSID_AND_ATTRIBUTES sidsToDisable, DWORD deletePrivilegeCount, PLUID_AND_ATTRIBUTES privilegesToDelete)
{
	HANDLE hNextToken;
	if (!CreateRestrictedToken(hPreToken, flags, disableSidCount, sidsToDisable, deletePrivilegeCount, privilegesToDelete, 0, nullptr, &hNextToken))
	{
		auto error = GetLastError();
		const std::exception ex("Failed to create restricted token");
		throw &ex;
	}

	return hNextToken;
}

HANDLE createRestrictedToken(HANDLE hPreToken, DWORD flags, DWORD disableSidCount, PSID_AND_ATTRIBUTES sidsToDisable)
{
	return createRestrictedToken(hPreToken, flags, disableSidCount, sidsToDisable, 0, nullptr);
}

HANDLE createRestrictedToken(HANDLE hPreToken, DWORD flags, DWORD deletePrivilegeCount, PLUID_AND_ATTRIBUTES privilegeToDelete)
{
	return createRestrictedToken(hPreToken, flags, 0, nullptr, deletePrivilegeCount, privilegeToDelete);
}

int main(void)
{
	//始めのトークンを取得
	auto hInitialToken = openCurrentProcessToken();

	//特権の制限test
	auto pTokenPrivilegesTest = getTokenInformation<PTOKEN_PRIVILEGES>(hInitialToken, TokenPrivileges);
	OutputPirivileges(pTokenPrivilegesTest);

	//ユーザーの制限
	auto pTokenUser = getTokenInformation<PTOKEN_USER>(hInitialToken, TokenUser);
	auto hTokenUserRestricted = createRestrictedToken(hInitialToken, DISABLE_MAX_PRIVILEGE, 1, &(pTokenUser->User));
	
	//TokenUserのSIDを取得
	LPTSTR StringSid;
	ConvertSidToStringSid(pTokenUser->User.Sid, &StringSid);
	cout << StringSid << endl;


	////特権の制限test
	//auto pTokenPrivilegesTest2 = getTokenInformation<PTOKEN_PRIVILEGES>(hInitialToken, TokenPrivileges);
	//OutputPirivileges(pTokenPrivilegesTest2);


	//グループの制限
	auto pTokenGroups = getTokenInformation<PTOKEN_GROUPS>(hTokenUserRestricted, TokenGroups);
	auto hTokenGroupsRestricted = createRestrictedToken(hTokenUserRestricted, DISABLE_MAX_PRIVILEGE, pTokenGroups->GroupCount, pTokenGroups->Groups);

	//hTokenRestricted2 = hTokenRestricted;

	//特権の制限
	//全部やってしまってるのでホワイトリストで制限を回避するように
	auto pTokenPrivileges = getTokenInformation<PTOKEN_PRIVILEGES>(hTokenGroupsRestricted, TokenPrivileges);
	auto hTokenPrivilegesRestricted = createRestrictedToken(hTokenGroupsRestricted, DISABLE_MAX_PRIVILEGE, pTokenPrivileges->PrivilegeCount, pTokenPrivileges->Privileges);

	cout << "last privileges" << endl;
	OutputPirivileges(pTokenPrivileges);

	ImpersonateLoggedOnUser(hTokenPrivilegesRestricted);

	WinExec("notepad", SW_SHOW);

	waitEnter();

	exePythonTest();
	createFileTest();

	RevertToSelf();

	//解放
	//CloseHandle(hTokenRestricted);
	//CloseHandle(hFile);
	//CloseHandle(hToken);
	//LocalFree(pTokenUser);
	return 0;
}