// USBViewer.cpp : Defines the entry point for the application.
//
 
#include "malloc.h"
#include "time.h"
#include <ctype.h>
#include "tchar.h"
#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif 

#define MAX_KEY_LENGTH	255
#define USB_REGPATH	"SYSTEM\\ControlSet001\\Enum\\USBSTOR\\"
#define MAX_MSG_LENGTH 1024
#define SafeDeletePointer(block,size) {if((size)>=0){if((block)&&!(IsBadReadPtr((block),(size)))) {GlobalFree((block));(block)=0;}}}

typedef struct _USBLog
{
	char    USBType[MAX_KEY_LENGTH];
	char	USBID[MAX_KEY_LENGTH];
	char	FriendlyName[128];
	char	ClassGUID[128];
	char	Driver[128];
	char	EnumeratorName[128];
	char	HardwareID[MAX_KEY_LENGTH];
	char	Service[128];
	char	Mfg[128];
	struct _USBLog *pNext;
}SUSBLog;

BOOL RegQuery(char*	pRegPath, char* pRegValueName, char*	pRegContent);
//BOOL GetNameUse(SID_NAME_USE _SidNameUse, TCHAR *szSIDType, DWORD *pdwIOLen);
BOOL ConvertSid(PSID pSid, LPTSTR pszSidText, LPDWORD dwBufferLen);
BOOL GetEventLogType(TCHAR *sz, unsigned short uEventType, DWORD *pdwSize);
BOOL GetEventLogImage(UINT *puImage, unsigned short uEventType);

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	HKEY  hKey = HKEY_LOCAL_MACHINE, phkResult = NULL;
	SUSBLog	*pUSBLog = NULL;		//链表的单个结构
	SUSBLog	*pUSBLogHead = NULL;	//链表头指针
	SUSBLog *pUSBLogLink = NULL;	//巡游指针

	char	USBSTOR_RegPath[MAX_PATH] = { 0 };	memset(USBSTOR_RegPath, 0, MAX_PATH);	strcpy(USBSTOR_RegPath, USB_REGPATH);
	DWORD	dwUSBSTOR_RegPath = strlen(USBSTOR_RegPath);
	char*	pUSBDevice_RegPath = USBSTOR_RegPath + dwUSBSTOR_RegPath;

	/////////////////////////////扫描所有的USB设备/////////////////////////////////////////////

	/*打开hKey中的rootKey, hKey乃继承递归调用者之hKey, rootKey乃递归调用者之subKey*/
	if (RegOpenKeyEx(hKey,		//欲开之键柄
		USBSTOR_RegPath,		//欲开之根键名
		0,				//保留值，强制0
		KEY_READ,		//权限
		&phkResult))	//打开子键后，返回键柄
	{
		MessageBox(NULL, "无法读取注册表", USBSTOR_RegPath, MB_OK);
		return 1;
	}

	//枚举完数据前，枚举子项	
	DWORD	dwUSBType = 0;					//存放子键的项数
	DWORD	i = 0;							//计数
	DWORD   sizeUSBType = 0;					//子键大小
	TCHAR	USBType[MAX_KEY_LENGTH];			//子键名

												/*首先查得当前键下的子键项数*/
	if (RegQueryInfoKey(phkResult,
		NULL,
		NULL,
		NULL,
		&dwUSBType,		//查得子键的项数
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL))
	{
		//MessageBox(NULL,"RegQueryInfoKey","List subKey",MB_OK);
		return 1;
	}
	if (dwUSBType)
	{
		for (i = 0; i < dwUSBType; i++)
		{
			memset(pUSBDevice_RegPath, 0, MAX_PATH - dwUSBSTOR_RegPath);		//清空注册表路径
			memset(USBType, 0, MAX_KEY_LENGTH);
			sizeUSBType = MAX_KEY_LENGTH;	//第一行清空子键，第二行赋缓冲区大小
			if (ERROR_SUCCESS ==
				RegEnumKeyEx(phkResult,		//键柄
					i,				//子键序号
					USBType,			//存入子键名
					&sizeUSBType,	//指明缓冲区大小
					NULL,
					NULL,
					NULL,
					NULL))
			{
				strcat(pUSBDevice_RegPath, USBType);	//USB 设备的注册表路径
				strcat(pUSBDevice_RegPath, "\\");
				////////////////////////////////////////////////
				////////////////////////////////////////////////
				////////////////枚举该类型的USB设备/////////////
				////////////////////////////////////////////////
				////////////////////////////////////////////////
				HKEY	phkUSBIDResult = NULL;
				DWORD	dwNumUSBID = 0;		// 该类型USB设备个数
				TCHAR	T_USBID[MAX_KEY_LENGTH] = { 0 };
				DWORD	sizeUSBID = 0;
				char*	pUSBID_RegPath = pUSBDevice_RegPath + sizeUSBType + 1;

				if (RegOpenKeyEx(hKey,		//欲开之键柄
					USBSTOR_RegPath,		//欲开之根键名
					0,				//保留值，强制0
					KEY_READ,		//权限
					&phkUSBIDResult))	//打开子键后，返回键柄
				{
					MessageBox(NULL, "无法读取注册表", USBSTOR_RegPath, MB_OK);
					return 1;
				}
				/*首先查得当前键下的子键项数*/
				if (RegQueryInfoKey(phkUSBIDResult,
					NULL,
					NULL,
					NULL,
					&dwNumUSBID,		//查得子键的项数
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL))
				{
					//MessageBox(NULL,"RegQueryInfoKey","List subKey",MB_OK);
					return 1;
				}
				for (int j = 0; j < dwNumUSBID; j++)
				{
					memset(T_USBID, 0, MAX_KEY_LENGTH);
					sizeUSBID = MAX_KEY_LENGTH;	//第一行清空子键，第二行赋缓冲区大小
					if (ERROR_SUCCESS ==
						RegEnumKeyEx(phkUSBIDResult,	//键柄
							j,					//子键序号
							T_USBID,			//存入子键名
							&sizeUSBID,			//指明缓冲区大小
							NULL,
							NULL,
							NULL,
							NULL))
					{
						pUSBLog = (SUSBLog *)malloc(sizeof(SUSBLog));
						memset((void*)pUSBLog, 0, sizeof(SUSBLog));
						strcpy(pUSBLog->USBType, USBType);
						pUSBLog->pNext = NULL;
					}

					//////////////////////////////////////////////////////////////////////////

					memset(pUSBID_RegPath, 0, MAX_PATH - dwUSBSTOR_RegPath - sizeUSBType - 1);
					strcpy(pUSBID_RegPath, T_USBID);	//strcat(pUSBID_RegPath,"\\");					
														///////////////////////////查询以下值//////////////////////////////////
					RegQuery(USBSTOR_RegPath, "FriendlyName", pUSBLog->FriendlyName);
					RegQuery(USBSTOR_RegPath, "ClassGUID", pUSBLog->ClassGUID);
					RegQuery(USBSTOR_RegPath, "Driver", pUSBLog->Driver);
					RegQuery(USBSTOR_RegPath, "HardwareID", pUSBLog->HardwareID);
					RegQuery(USBSTOR_RegPath, "Service", pUSBLog->Service);
					RegQuery(USBSTOR_RegPath, "Mfg", pUSBLog->Mfg);
					if (i == 0 && j == 0) { pUSBLogHead = pUSBLog; pUSBLogLink = pUSBLog; }
					else {
						pUSBLogLink->pNext = pUSBLog;	pUSBLogLink = pUSBLog;
					}
				}
				RegCloseKey(phkUSBIDResult); phkUSBIDResult = NULL;
			}
		}
	}
	RegCloseKey(phkResult);

	/////////////////////////////记录到日志中/////////////////////////////////////////

	pUSBLogLink = pUSBLogHead;

	char	szUSBLogFile[MAX_PATH] = { 0 };	memset(szUSBLogFile, 0, MAX_PATH);
	GetSystemDirectory(szUSBLogFile, MAX_PATH);
	strcat(szUSBLogFile, "\\USB_LogFile.txt");
	HANDLE	hFile = CreateFile(szUSBLogFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) { return 1; }
	DWORD	dwWritten = 0;
	char	szMess[MAX_PATH] = { 0 };
	memset(szMess, 0, MAX_PATH);	strcpy(szMess, "USB_LogFile:\r\n");
	WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

	while (pUSBLogLink)
	{
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\n***********************************************************************\r\n");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		//		char	FriendlyName[128];
		//		char	ClassGUID[128];
		//		char	Driver[128];
		//		char	EnumeratorName[128];
		//		char	HardwareID[MAX_KEY_LENGTH];
		//		char	Service[128];
		//		char	Mfg[128];

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nFriendlyName             :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->FriendlyName);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nClassGUID                :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->ClassGUID);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nDriver                   :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->ClassGUID);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nEnumeratorName           :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->EnumeratorName);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nHardwareID               :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->HardwareID);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nService                  :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->Service);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);

		memset(szMess, 0, MAX_PATH);	strcpy(szMess, "\r\nMfg                      :	");
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		memset(szMess, 0, MAX_PATH);	strcpy(szMess, pUSBLogLink->Mfg);
		WriteFile(hFile, szMess, strlen(szMess), &dwWritten, NULL);
		pUSBLogLink = pUSBLogLink->pNext;
	}

	/////////////////////////////////读取USB日志/////////////////////////////////////////
	DWORD	g_dwLastError = 0L;
	int		nRetVal = 0;
	HWND hParentWnd = 0, hwndDlg = 0, hwndLV = 0, hwndProgr = 0;
	HANDLE hEventLog = 0;	//事件句柄
	DWORD dwEventLogRecords = 0,		//事件  记录数
		dwOldestEventLogRecord = 0,		//时间  最老的记录数
		dwEvLogCounter = 0,				//计数器
		dwNumberOfBytesToRead = 0,
		dwBytesRead = 0,
		dwMinNumberOfBytesNeeded = 0,
		dwCancel = 0,
		dwClose = 0;
	LPVOID lpEventLogRecordBuffer = 0;	//日志 缓存指针
	TCHAR chFakeBuffer;					//日志 缓存
	BOOL bRetVal = FALSE;
	BOOL fExit = FALSE;
	UINT uStep = 0, uStepAt = 0, uPos = 0, uOffset = 0;
	TCHAR 	lpszErrMsg[1024];//lpUNCServerName[MAX_PATH + 1], 	//lpszEventLogSourceName[MAX_PATH + 1],

	while (!fExit)
	{
		//hEventLog = OpenEventLog((LPCTSTR)lpUNCServerName, (LPCTSTR)lpszEventLogSourceName);
		hEventLog = OpenEventLog("", "System");
		if (hEventLog)
		{
			if (GetNumberOfEventLogRecords(hEventLog, &dwEventLogRecords) &&
				GetOldestEventLogRecord(hEventLog, &dwOldestEventLogRecord))
				//The GetOldestEventLogRecord function retrieves the absolute record number of the oldest record in the specified event log.
			{
				for (dwEvLogCounter = dwOldestEventLogRecord;
					dwEvLogCounter < (dwOldestEventLogRecord + dwEventLogRecords);
					dwEvLogCounter++)
				{

					lpEventLogRecordBuffer = (LPVOID)&chFakeBuffer;
					dwNumberOfBytesToRead = 1;
					dwMinNumberOfBytesNeeded = 1;
				_retry_:
					bRetVal = ReadEventLog(hEventLog, EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ, dwEvLogCounter,
						lpEventLogRecordBuffer, dwNumberOfBytesToRead, &dwBytesRead, &dwMinNumberOfBytesNeeded);
					if (!bRetVal)
					{
						g_dwLastError = GetLastError();
						if (g_dwLastError == ERROR_INSUFFICIENT_BUFFER)
						{
							lpEventLogRecordBuffer = (LPVOID)GlobalAlloc(GPTR, dwMinNumberOfBytesNeeded);
							//							if(lpEventLogRecordBuffer == (void *)0)
							//								goto _allocationfailure_;

							dwNumberOfBytesToRead = dwMinNumberOfBytesNeeded;
							goto _retry_;
						}
						//						else
						//							goto _unknownerror_;
						return 1;
					}
					else {
						PEVENTLOGRECORD pELR = 0;
						TCHAR *lpszSourceName = 0,
							lpszUserName[MAX_PATH + 1],
							*lpszComputerName = 0,
							lpszRefDomainName[MAX_PATH + 1],
							*szSIDType = 0,
							*szSIDName = 0,
							sz2[32],
							*szExpandedString = 0,
							szSubmitTime[32] = { 0 },
							szWriteTime[32] ={0};
						DWORD dwSourceNameLen = 0,
							dwComputerNameLen = 0,
							cbName = MAX_PATH + 1,
							cbRefDomainName = MAX_PATH + 1,
							dwSIDTypeLen = 0,
							dwSidSize = 0,
							dwEventTypeLen = 0;
						PSID pUserSID = 0;
						SID_NAME_USE _SidNameUse = (SID_NAME_USE)(SidTypeUser - 1);
						BOOL bRetVal = FALSE;
						LPBYTE pStrings = 0, pData = 0;
						UINT x = 0, uSize, uStringOffset, uStepOfString = 0, uImage = 0;

						pELR = (PEVENTLOGRECORD)lpEventLogRecordBuffer;
						uOffset = sizeof(EVENTLOGRECORD);
						lpszSourceName = (TCHAR *)GlobalAlloc(GPTR, (MAX_PATH + 1) * sizeof(TCHAR));
						strcpy(lpszSourceName, (LPTSTR)((LPBYTE)pELR + uOffset));
						dwSourceNameLen = strlen(lpszSourceName);

						uOffset += strlen(lpszSourceName) + sizeof(TCHAR);
						lpszComputerName = (TCHAR *)GlobalAlloc(GPTR, (MAX_PATH + 1) * sizeof(TCHAR));
						strcpy(lpszComputerName, (LPTSTR)((LPBYTE)pELR + uOffset));
						dwComputerNameLen = strlen(lpszComputerName);

						uOffset += strlen(lpszComputerName) + sizeof(TCHAR);

						dwSIDTypeLen = 32;
						szSIDType = (TCHAR *)GlobalAlloc(GPTR, (dwSIDTypeLen + 1) * sizeof(TCHAR));

						if (pELR->UserSidLength > 0)
						{
							pUserSID = (SID *)GlobalAlloc(GPTR, pELR->UserSidLength);
							memcpy(pUserSID, (PSID)((LPBYTE)pELR + pELR->UserSidOffset), pELR->UserSidLength);

							cbName = cbRefDomainName = MAX_PATH + 1;
							*lpszRefDomainName = *lpszUserName = '\0';

							bRetVal = LookupAccountSid(0, pUserSID,
								lpszUserName, &cbName,
								lpszRefDomainName, &cbRefDomainName,
								&_SidNameUse);

							if (bRetVal)
							{
								if (bRetVal)
								{
									dwSIDTypeLen = 32;
									//									GetNameUse(_SidNameUse, szSIDType, &dwSIDTypeLen);

									dwSidSize = (15 + 12 + (12 * (*GetSidSubAuthorityCount(pUserSID))) + 1) * sizeof(TCHAR);
									szSIDName = (TCHAR *)GlobalAlloc(GPTR, (dwSidSize + 1) * sizeof(TCHAR));
									ConvertSid(pUserSID, szSIDName, &dwSidSize);
								}
								else
								{
									strcpy(lpszRefDomainName, "N/A");
									strcpy(lpszUserName, "N/A");
									strcpy(szSIDType, "N/A");
								}
							}
							else {
							}
						}
						else {
							strcpy(lpszRefDomainName, "N/A");
							strcpy(lpszUserName, "N/A");
							strcpy(szSIDType, "N/A");
						}

						uSize = 0, uStringOffset = pELR->StringOffset;
						uSize = pELR->DataOffset - pELR->StringOffset;

						// Strings
						if (uSize > 0)
						{
							pStrings = (LPBYTE)GlobalAlloc(GPTR, uSize * sizeof(BYTE));
							memcpy(pStrings, (LPBYTE)pELR + uStringOffset, uSize);

							//	Strings
							uStepOfString = 0;
							szExpandedString = (TCHAR *)GlobalAlloc(GPTR, (uSize + MAX_MSG_LENGTH) * sizeof(TCHAR));
							for (x = 0; x < pELR->NumStrings; x++)
							{
								if (x == 0)
								{
									strcpy(szExpandedString, (TCHAR *)pStrings + uStepOfString);
									if (x < (UINT)pELR->NumStrings - 1)
										strcat(szExpandedString, ",");
								}
								else
									strcat(szExpandedString, (TCHAR *)pStrings + uStepOfString);

								uStepOfString = strlen((TCHAR *)pStrings + uStepOfString) + 1;
							}
						}

						//	Data
						pData = (LPBYTE)GlobalAlloc(GPTR, pELR->DataLength * sizeof(BYTE));
						memcpy(pData, (LPBYTE)((LPBYTE)pELR + pELR->DataOffset), pELR->DataLength);

						dwEventTypeLen = 32;
						GetEventLogType(sz2, pELR->EventType, &dwEventTypeLen);
						GetEventLogImage(&uImage, pELR->EventType);

						//lstrcpyn(szSubmitTime, asctime(localtime((time_t *)&(pELR->TimeGenerated))), 25);
						//lstrcpyn(szWriteTime, asctime(localtime((time_t *)&(pELR->TimeWritten))), 25);

						//						InsertRowInList(hwndLV, 9, &dwEvLogCounter, 
						//							lpszSourceName, 
						//							lpszUserName, 
						//							szSIDName, 
						//							lpszRefDomainName, 
						//							sz2, uImage, 
						//							szSubmitTime, szWriteTime);

						if (uSize > 0 && NULL != strstr(lpszSourceName, "Removable Storage"))
						{
							memset(lpszErrMsg, 0, 1024);
							strcpy(lpszErrMsg, "\r\n***********************************************************************\r\n");
							strcat(lpszErrMsg, szSubmitTime);	strcat(lpszErrMsg, "|");
							strcat(lpszErrMsg, szWriteTime);		strcat(lpszErrMsg, "\r\n");
							strcat(lpszErrMsg, lpszSourceName);	strcat(lpszErrMsg, "\r\n");
							strcat(lpszErrMsg, szExpandedString); strcat(lpszErrMsg, "\r\n");
							WriteFile(hFile, lpszErrMsg, strlen(lpszErrMsg), &dwWritten, NULL);
						}

						SafeDeletePointer(pData, pELR->DataLength);
						SafeDeletePointer(szExpandedString, uSize);
						SafeDeletePointer(pStrings, pELR->DataOffset - pELR->StringOffset);
						SafeDeletePointer(szSIDName, dwSidSize + 1);
						SafeDeletePointer(szSIDType, dwSIDTypeLen + 1);
						SafeDeletePointer(lpszSourceName, dwSourceNameLen);
						SafeDeletePointer(lpszComputerName, dwComputerNameLen);
						SafeDeletePointer(pUserSID, pELR->UserSidLength);
						SafeDeletePointer(lpEventLogRecordBuffer, dwNumberOfBytesToRead);
					}

				}
			}
			fExit = TRUE;
		}

	}

	CloseHandle(hFile);
	return 1;
}

BOOL GetEventLogImage(UINT *puImage, unsigned short uEventType)
{
	if (!puImage)
		return FALSE;

	switch (uEventType)
	{
	case EVENTLOG_SUCCESS:
		*puImage = 2;
		break;
	case EVENTLOG_ERROR_TYPE:
		*puImage = 1;
		break;
	case EVENTLOG_WARNING_TYPE:
		*puImage = 0;
		break;
	case EVENTLOG_INFORMATION_TYPE:
		*puImage = 2;
		break;
	case EVENTLOG_AUDIT_SUCCESS:
		*puImage = 4;
		break;
	case EVENTLOG_AUDIT_FAILURE:
		*puImage = 3;
		break;
	default:
		*puImage = 0;
		break;
	}

	return TRUE;
}


BOOL GetEventLogType(TCHAR *sz, unsigned short uEventType, DWORD *pdwSize)
{
	if (!sz || !pdwSize)
		return FALSE;

	switch (uEventType)
	{
	case EVENTLOG_SUCCESS:
		strncpy(sz, _T("Success"), *pdwSize);
		break;
	case EVENTLOG_ERROR_TYPE:
		strncpy(sz, _T("Error"), *pdwSize);
		break;
	case EVENTLOG_WARNING_TYPE:
		strncpy(sz, _T("Warning"), *pdwSize);
		break;
	case EVENTLOG_INFORMATION_TYPE:
		strncpy(sz, _T("Information"), *pdwSize);
		break;
	case EVENTLOG_AUDIT_SUCCESS:
		strncpy(sz, _T("Success Audit"), *pdwSize);
		break;
	case EVENTLOG_AUDIT_FAILURE:
		strncpy(sz, _T("Failure Audit"), *pdwSize);
		break;
	default:
		strncpy(sz, _T("Unknown"), *pdwSize);
		break;
	}

	*pdwSize = strlen(sz);
	return TRUE;
}

BOOL ConvertSid(PSID pSid, LPTSTR pszSidText, LPDWORD dwBufferLen)
{
	DWORD                     dwSubAuthorities;
	DWORD                     dwSidRev = SID_REVISION;
	DWORD                     dwCounter;
	DWORD                     dwSidSize;
	PSID_IDENTIFIER_AUTHORITY psia;

	if (!IsValidSid(pSid))
		return FALSE;

	psia = GetSidIdentifierAuthority(pSid);

	dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

	dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

	if (*dwBufferLen < dwSidSize)
	{
		*dwBufferLen = dwSidSize;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return FALSE;
	}

	dwSidSize = wsprintf(pszSidText, TEXT("S-%lu-"), dwSidRev);

	if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
		dwSidSize += wsprintf(pszSidText + lstrlen(pszSidText),
			TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
			(USHORT)psia->Value[0],
			(USHORT)psia->Value[1],
			(USHORT)psia->Value[2],
			(USHORT)psia->Value[3],
			(USHORT)psia->Value[4],
			(USHORT)psia->Value[5]);
	else
		dwSidSize += wsprintf(pszSidText + lstrlen(pszSidText),
			TEXT("%lu"),
			(ULONG)(psia->Value[5]) +
			(ULONG)(psia->Value[4] << 8) +
			(ULONG)(psia->Value[3] << 16) +
			(ULONG)(psia->Value[2] << 24));

	for (dwCounter = 0; dwCounter < dwSubAuthorities; dwCounter++)
		dwSidSize += wsprintf(pszSidText + dwSidSize, TEXT("-%lu"), *GetSidSubAuthority(pSid, dwCounter));

	return TRUE;
}

BOOL RegQuery(char* pRegPath, char* pRegValueName, char* pRegContent)
{
	HKEY   phkResult = NULL;
	DWORD	dwType = REG_SZ;
	DWORD cbData_1 = MAX_KEY_LENGTH;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pRegPath, 0, KEY_READ, &phkResult) != ERROR_SUCCESS)
		return FALSE;
	if (RegQueryValueEx(phkResult, pRegValueName, NULL, &dwType, (LPBYTE)pRegContent, &cbData_1) != ERROR_SUCCESS)
		return FALSE;
	RegCloseKey(phkResult);
	return TRUE;
}