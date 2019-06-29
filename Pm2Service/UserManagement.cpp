#include "UserManagement.h"

// Debugging
#include <iostream>

#include <sstream>

// Yay! Let's have ANOTHER kind of string.
bool InitLsaString(PLSA_UNICODE_STRING pLsaString, LPCWSTR pwszString)
{
  DWORD dwLen = 0;

  if (NULL == pLsaString)
      return FALSE;

  if (NULL != pwszString) 
  {
      dwLen = (DWORD)wcslen(pwszString);
      if (dwLen > 0x7ffe)   // String is too large
          return FALSE;
  }

  // Store the string.
  pLsaString->Buffer = (WCHAR *)pwszString;
  pLsaString->Length =  (USHORT)dwLen * sizeof(WCHAR);
  pLsaString->MaximumLength= (USHORT)(dwLen+1) * sizeof(WCHAR);

  return TRUE;
}


bool VerifyUserExists(std::wstring& username)
{
    DWORD cbSid = 0;
    DWORD cchRefDomain = 0;
    SID_NAME_USE sidUse;

    BOOL ok = LookupAccountNameW(
        NULL,
        username.c_str(),
        NULL,
        &cbSid,
        NULL,
        &cchRefDomain,
        &sidUse);

    int err = GetLastError();
    if (ok == 0 && err != ERROR_INSUFFICIENT_BUFFER)
    {
        return false;
    }

    PSID sidPtr = calloc(cbSid, 1);
    wchar_t* refDomain = (wchar_t*)calloc(cchRefDomain + 1, sizeof(wchar_t));
    ok = LookupAccountNameW(
        NULL,
        username.c_str(),
        sidPtr,
        &cbSid,
        refDomain,
        &cchRefDomain,
        &sidUse);

    free(sidPtr);
    free(refDomain);

    return ok != 0;
}
bool VerifyUser(std::wstring& username)
{
    bool result = false;

    LSA_HANDLE policy;
    LSA_OBJECT_ATTRIBUTES attribs = { 0 };
    NTSTATUS status = LsaOpenPolicy(NULL, &attribs, STANDARD_RIGHTS_READ|POLICY_LOOKUP_NAMES|POLICY_VIEW_LOCAL_INFORMATION, &policy);
    if (status == STATUS_SUCCESS)
    {
        LSA_UNICODE_STRING uname;
        if (!InitLsaString(&uname, username.c_str()))
        {
            LsaClose(policy);
            return false;
        }

        PLSA_TRANSLATED_SID2 sids;
        PLSA_REFERENCED_DOMAIN_LIST domainList;

        status = LsaLookupNames2(policy, 0, 1, &uname, &domainList, &sids);
        if (status == STATUS_SUCCESS)
        {
            PLSA_UNICODE_STRING perms;
            ULONG nPerms;
            status = LsaEnumerateAccountRights(policy, sids->Sid, &perms, &nPerms);
            if (status == STATUS_SUCCESS)
            {
                for (ULONG i = 0; i < nPerms; i++)
                {
                    if (perms[i].Buffer && _wcsicmp(perms[i].Buffer, SE_SERVICE_LOGON_NAME) == 0)
                    {
                        result = true;
                    }
                }
                LsaFreeMemory(perms);
            }
        }

        LsaFreeMemory(sids);
        LsaFreeMemory(domainList);
        LsaClose(policy);
    }

    return result;
}
HRESULT ConfigureUser(std::wstring& username)
{
    LSA_HANDLE policy;
    LSA_OBJECT_ATTRIBUTES attribs = { 0 };
    NTSTATUS status = LsaOpenPolicy(NULL, &attribs, POLICY_ALL_ACCESS, &policy);
    if (status == STATUS_SUCCESS)
    {
        LSA_UNICODE_STRING uname;
        if (!InitLsaString(&uname, username.c_str()))
        {
            LsaClose(policy);
            return false;
        }

        PLSA_TRANSLATED_SID2 sids;
        PLSA_REFERENCED_DOMAIN_LIST domainList;

        status = LsaLookupNames2(policy, 0, 1, &uname, &domainList, &sids);
        if (status == STATUS_SUCCESS)
        {
            LSA_UNICODE_STRING perm;
            if (InitLsaString(&perm, SE_SERVICE_LOGON_NAME))
            {
                status = LsaAddAccountRights(policy, sids->Sid, &perm, 1);
            }
        }

        LsaFreeMemory(sids);
        LsaFreeMemory(domainList);
        LsaClose(policy);
    }

    return HRESULT_FROM_NT(status);
}
bool NormalizeUsername(std::wstring& username)
{
    DWORD cbSid = 0;
    DWORD cchRefDomain = 0;
    SID_NAME_USE sidUse;

    BOOL ok = LookupAccountNameW(
        NULL,
        username.c_str(),
        NULL,
        &cbSid,
        NULL,
        &cchRefDomain,
        &sidUse);

    int err = GetLastError();
    if (ok == 0 && err != ERROR_INSUFFICIENT_BUFFER)
    {
        return false;
    }

    PSID sidPtr = calloc(cbSid, 1);
    wchar_t* refDomain = (wchar_t*)calloc(cchRefDomain + 1, sizeof(wchar_t));
    ok = LookupAccountNameW(
        NULL,
        username.c_str(),
        sidPtr,
        &cbSid,
        refDomain,
        &cchRefDomain,
        &sidUse);

    free(refDomain);

    DWORD cchName;

    ok = LookupAccountSidW(NULL, sidPtr, NULL, &cchName, NULL, &cchRefDomain, &sidUse);
    
    err = GetLastError();
    if (ok == 0 && err != ERROR_INSUFFICIENT_BUFFER)
    {
        free(sidPtr);
        return false;
    }

    wchar_t* acctName = (wchar_t*)calloc(cchName, sizeof(wchar_t));
    refDomain = (wchar_t*)calloc(cchRefDomain, sizeof(wchar_t));
    ok = LookupAccountSidW(NULL, sidPtr, acctName, &cchName, refDomain, &cchRefDomain, &sidUse);

    std::wstringstream ss;
    ss << refDomain << L"\\" << acctName;
    username = ss.str();

    free(refDomain);
    free(acctName);
    free(sidPtr);

    if (ok == 0)
    {
        return false;
    }
    return true;
}
