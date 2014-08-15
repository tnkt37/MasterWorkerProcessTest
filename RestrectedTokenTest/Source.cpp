#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>

#include <windows.h>
#include <Sddl.h>

#define UP

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

void waitEnter(const char message[] = "press enter...")
{
	cout << message << endl;
	string dummmmm;
	getline(cin, dummmmm);
}

void readFileTest(string fileName)
{
	std::stringstream ss;
	ss << "C:\\Users\\tnkt\\Documents\\Visual Studio 2013\\Projects\\MasterWorkerProcessTest\\Debug\\Temp\\" << fileName << endl;
	std::ifstream ifs(ss.str(), std::ifstream::binary);
	if (ifs.fail())
	{
		std::cerr << "失敗" << std::endl;
	}
	else
	{
		std::string str((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());
		cout << "file content: " << str << endl;
	}

}

void writeFileTest()
{
	ofstream fout("C:\\Users\\tnkt\\Documents\\Visual Studio 2013\\Projects\\MasterWorkerProcessTest\\Debug\\Temp\\lowlevelfile.txt");
	//ofstream fout("C:\\Users\\tnkt\\Documents\\Visual Studio 2013\\Projects\\MasterWorkerProcessTest\\Debug\\Temp\\middlelevelfile.txt");
	if (fout.fail()){  // if(!fout)でもよい。
		cout << "出力ファイルをオープンできません" << endl;
	}
	else
	{
		fout << "write file" << endl;
	}
}

void createFileTest()
{
	char str[] = "hogehoge";
	stringstream ss;
	//ss << "C://CPP//" << "sample" << 0 << ".txt";
	ss << "C:\\Users\\tnkt\\Documents\\Visual Studio 2013\\Projects\\MasterWorkerProcessTest\\Debug\\Temp\\" << "sample" << ".txt";
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
	Py_Initialize();


	cout << "run python" << endl;
	PyRun_SimpleString(
		//"import json\n"
		//"print 'imported json'\n"
		"f = open('pythontest.txt', 'w')\n"
		//"f = open('pythontest.txt','r')\n"
		//"for row in f:\n"
		//"\tprint row\n"
		//"f = open('C:\\CPP\\')"
		"f.write('I am python')\n"
		"f.close()\n");
	//PyRun_SimpleString("print 'hogehoge'");
	cout << "python executed" << endl;
	Py_Finalize();
	cout << "python finalized" << endl;
	waitEnter();
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



enum IntegrityLevel {
	INTEGRITY_LEVEL_SYSTEM,
	INTEGRITY_LEVEL_HIGH,
	INTEGRITY_LEVEL_MEDIUM,
	INTEGRITY_LEVEL_MEDIUM_LOW,
	INTEGRITY_LEVEL_LOW,
	INTEGRITY_LEVEL_BELOW_LOW,
	INTEGRITY_LEVEL_UNTRUSTED,
	INTEGRITY_LEVEL_LAST
};

const LPCTSTR GetIntegrityLevelString(IntegrityLevel integrity_level) {
	switch (integrity_level) {
	case INTEGRITY_LEVEL_SYSTEM:
		return "S-1-16-16384";
	case INTEGRITY_LEVEL_HIGH:
		return "S-1-16-12288";
	case INTEGRITY_LEVEL_MEDIUM:
		return "S-1-16-8192";
	case INTEGRITY_LEVEL_MEDIUM_LOW:
		return "S-1-16-6144";
	case INTEGRITY_LEVEL_LOW:
		return "S-1-16-4096";
	case INTEGRITY_LEVEL_BELOW_LOW:
		return "S-1-16-2048";
	case INTEGRITY_LEVEL_UNTRUSTED:
		return "S-1-16-0";
	case INTEGRITY_LEVEL_LAST:
		return NULL;
	}
	return NULL;
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

	//
	//chrome restricted_token_utils
	//

	//const wchar*からLPCTSTRに書き換えた（良いのか？）
	LPCTSTR  integrity_level_str = GetIntegrityLevelString(INTEGRITY_LEVEL_LOW);
	waitEnter("convert");
	PSID integrity_sid = nullptr;
	//LPCSTRに変えないとここでコンパイルエラー
	if (!::ConvertStringSidToSid(integrity_level_str, &integrity_sid))
		throw std::exception("convert string sid to sid error");

	TOKEN_MANDATORY_LABEL label;
	label.Label.Attributes = SE_GROUP_INTEGRITY;
	//label.Label.Attributes = SE_GROUP_INTEGRITY_ENABLED;
	label.Label.Sid = integrity_sid;

	//msdnのマネ
	HANDLE duplicatedToken;
	DuplicateTokenEx(hInitialToken, MAXIMUM_ALLOWED, nullptr, SecurityImpersonation, TokenPrimary, &duplicatedToken);

	DWORD size = sizeof(TOKEN_MANDATORY_LABEL) + ::GetLengthSid(integrity_sid);
	//制限をつけてないトークンにやる
	BOOL result = ::SetTokenInformation(duplicatedToken, TokenIntegrityLevel, &label, size);
	//bool result = false;
	::LocalFree(integrity_sid);
	if (!result)
	{
		auto error = GetLastError();
		const std::exception ex("Failed to SetTokenInformation");
		throw &ex;
	}
	//
	// end chrome source code
	//

	cout << "last privileges" << endl;
	OutputPirivileges(pTokenPrivileges);

	//色々剥奪したトークンをプロセスに適用(偽装)
	//ImpersonateLoggedOnUser(hTokenPrivilegesRestricted);
	ImpersonateLoggedOnUser(duplicatedToken);
	//ImpersonateLoggedOnUser(hTokenUserRestricted);

	//ファイル読み込みテスト
	std::ifstream ifs("C:\\Users\\tnkt\\Documents\\Visual Studio 2013\\Projects\\MasterWorkerProcessTest\\Debug\\Temp\\lowlevelfile.txt", std::ifstream::binary);
	if (ifs.fail())
		std::cerr << "失敗" << std::endl;
	else
	{
		std::string str((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());
		cout << "file content: " << str << endl;
	}
	//ファイル読み込みテスト

	writeFileTest();
	//WinExec("notepad", SW_SHOW);

	waitEnter();

	exePythonTest();
	createFileTest();

	//トークンを戻す
	RevertToSelf();

	//解放
	//CloseHandle(hTokenRestricted);
	//CloseHandle(hFile);
	//CloseHandle(hToken);
	//LocalFree(pTokenUser);
	return 0;
}