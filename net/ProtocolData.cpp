#include "ProtocolData.h"

CNetData::CNetData()
{
	m_uType = 0;
	m_nRetryTimes = 0;
}

CNetData::~CNetData()
{
}

//class CFileItemRequest
CFileItemRequest::CFileItemRequest()
{
	m_uType = NET_DATA_FILE;

	m_nID = 0;
	memset(m_szUtfFilePath, 0, sizeof(m_szUtfFilePath));
	memset(m_szFilePath, 0, sizeof(m_szFilePath));

	m_reFlectionCallBack = NULL;
	//m_hCancelEvent = NULL;

	m_uSenderID = 0;

	m_nFileType = FILE_ITEM_UNKNOWN;

	m_bPending = TRUE;

	m_uAccountID = 0;
}

CFileItemRequest::~CFileItemRequest()
{
}

// class CUploadFileResult
CUploadFileResult::CUploadFileResult()
{
	m_nFileType = FILE_ITEM_UNKNOWN;
	m_bSuccessful = FALSE;
	m_nFileSize = 0;
	memset(m_szLocalName, 0, sizeof(m_szLocalName));

	memset(m_szMd5, 0, sizeof(m_szMd5));
	memset(m_szRemoteName, 0, sizeof(m_szRemoteName));

	m_uSenderID = 0;

	//m_reFlectionCallBack = NULL;
}

CUploadFileResult::~CUploadFileResult()
{

}

void CUploadFileResult::Clone(const CUploadFileResult* pSource)
{
	if (pSource == NULL)
		return;

	m_nFileType = pSource->m_nFileType;
	m_bSuccessful = pSource->m_bSuccessful;
	m_nFileSize = pSource->m_nFileSize;

	_tcscpy_s(m_szLocalName, ARRAYSIZE(m_szLocalName), pSource->m_szLocalName);

	strcpy_s(m_szMd5, ARRAYSIZE(m_szMd5), pSource->m_szMd5);
	strcpy_s(m_szRemoteName, ARRAYSIZE(m_szRemoteName), pSource->m_szRemoteName);

	m_uSenderID = pSource->m_uSenderID;
	m_setTargetIDs = pSource->m_setTargetIDs;

	//m_reFlectionCallBack = pSource->m_reFlectionCallBack;
}
