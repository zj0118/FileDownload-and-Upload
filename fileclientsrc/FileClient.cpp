#include "FileClient.h"

void CFileClient::Start()
{

}

void CFileClient::SetAsyncResultHandler(AsyncResultHandler&& asyncResultHandler)
{
	m_asyncResultHandler = asyncResultHandler;
}

void CFileClient::SetIsComplete(bool bIscomplete)
{
	m_IsComplete = bIscomplete;
}

CFileClient& CFileClient::GetInstance()
{
	static CFileClient client;
	return client;
}

CFileClient::CFileClient()
{
	m_IsInit = false;
}

CFileClient::~CFileClient()
{
}

bool CFileClient::InitNetThreads()
{
#ifdef WIN32
	if (m_IsInit)
		return true;

	//³õÊ¼»¯socket¿â
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int err = ::WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		printf("WSAStartup failed with error: %d\n", err);
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		::WSACleanup();
	}
#endif
	m_FileTask.m_lpFileEvent = this;
	m_FileTask.Start();
	m_IsInit = true;
	return true;
}

void CFileClient::Uninit()
{
#ifdef WIN32
	if (m_IsInit)
		::WSACleanup();
#endif
	m_FileTask.Stop();
	m_FileTask.Join();
	m_IsInit = false;
}

bool CFileClient::GetIsInit()
{
	return m_IsInit;
}

void CFileClient::SetFileServer(const std::string& pszServer)
{
	CMyIUSocket::GetInstance().SetFileServer(pszServer);
}

void CFileClient::SetFilePort(short port)
{
	CMyIUSocket::GetInstance().SetFilePort(port);
}

void CFileClient::UploadFile(PCTSTR pszFileName)
{
	m_IsComplete = false;
	CFileItemRequest* pFileItemRequest = new CFileItemRequest();
	_tcscpy_s(pFileItemRequest->m_szFilePath, ARRAYSIZE(pFileItemRequest->m_szFilePath), pszFileName);
	pFileItemRequest->m_reFlectionCallBack = m_asyncResultHandler;
	pFileItemRequest->m_nFileType = FILE_ITEM_UPLOAD_CHAT_OFFLINE_FILE;
	m_FileTask.AddItem(pFileItemRequest);
	while (!m_IsComplete)
	{
		Sleep(10);
	}
}

void CFileClient::DownloadFile(LPCSTR lpszFileName, LPCTSTR lpszDestPath)
{
	m_IsComplete = false;
	CFileItemRequest* pFileItemRequest = new CFileItemRequest();
	_tcscpy_s(pFileItemRequest->m_szFilePath, ARRAYSIZE(pFileItemRequest->m_szFilePath), lpszDestPath);
	strcpy_s(pFileItemRequest->m_szUtfFilePath, ARRAYSIZE(pFileItemRequest->m_szUtfFilePath), lpszFileName);
	pFileItemRequest->m_reFlectionCallBack = m_asyncResultHandler;
	pFileItemRequest->m_nFileType = FILE_ITEM_DOWNLOAD_CHAT_OFFLINE_FILE;
	m_FileTask.AddItem(pFileItemRequest);
	while (!m_IsComplete)
	{
		Sleep(10);
	}
}
