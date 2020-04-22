#pragma comment(lib, "Pathcch.lib")

#include <iostream>
#include <Windows.h>
#include <shlobj_core.h>
#include <pathcch.h>
#include <accctrl.h>
#include <aclapi.h>

int main()
{
	TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP | CSIDL_FLAG_CREATE, NULL, 0, path)))
	{
		HANDLE cf = NULL;

		//////////////////////////////////////////////////////////////////////////////////////////////////////

		PSID pEverythingSID = NULL, pAdministratorSID = NULL;
		SID_IDENTIFIER_AUTHORITY authorityEverything = SECURITY_WORLD_SID_AUTHORITY;
		SID_IDENTIFIER_AUTHORITY authorityAdministrator = SECURITY_NT_AUTHORITY;

		EXPLICIT_ACCESS ea[2];
		ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

		DWORD res;
		PACL pACL = NULL;
		PSECURITY_DESCRIPTOR pSD = NULL;
		SECURITY_ATTRIBUTES sa;

		if (!AllocateAndInitializeSid(&authorityEverything, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEverythingSID))
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
			goto Cleanup;
		}

		/* Все */
		ea[0].grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_EXECUTE;
		ea[0].grfAccessMode = DENY_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[0].Trustee.ptstrName = (LPTSTR)pEverythingSID;

		if (!AllocateAndInitializeSid(&authorityAdministrator, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorSID))
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
			goto Cleanup;
		}

		/* Администратор */
		ea[1].grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_EXECUTE | FILE_GENERIC_WRITE | DELETE;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance = NO_INHERITANCE;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[1].Trustee.ptstrName = (LPTSTR)pAdministratorSID;

		res = SetEntriesInAcl(2, ea, NULL, &pACL);

		if (ERROR_SUCCESS != res)
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
			goto Cleanup;
		}

		pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

		if (NULL == pSD)
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
			goto Cleanup;
		}

		if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
			goto Cleanup;
		}

		if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
			goto Cleanup;
		}

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = pSD;
		sa.bInheritHandle = FALSE;

		//////////////////////////////////////////////////////////////////////////////////////////////////////

		PathCchAppend(path, MAX_PATH, TEXT("test.txt"));
		cf = CreateFile(path, GENERIC_READ, 0, &sa, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == cf)
		{
			MessageBox(NULL, TEXT("Возникла ошибка при созданиии файла :\nвозможно файл с таким именем уже был создан."), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
		}

	Cleanup:
		if (pEverythingSID)
			FreeSid(pEverythingSID);
		if (pAdministratorSID)
			FreeSid(pAdministratorSID);
		if (pACL)
			LocalFree(pACL);
		if (pSD)
			LocalFree(pSD);
		if (cf)
			CloseHandle(cf);
	}

	return 0;
}