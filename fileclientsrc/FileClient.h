#pragma once
#include <functional>

#include "MyFileTaskThread.h"
#include "MyIUSocket.h"
#include "ProtocolData.h"

class CFileClient;

//using AsyncResultHandler = std::function<void(CFileClient* client, bool bIsComplete, const std::string& resultStr)>;

class CFileClient
{
public:
	static CFileClient& GetInstance();

public:
	CFileClient();
	~CFileClient();

public:
	bool InitNetThreads();		//��ʼ�������̡߳����ļ��߳�
	void Uninit();				//����ʼ��
	bool GetIsInit();

	void SetFileServer(const std::string& pszServer);
	void SetFilePort(short port);

	void UploadFile(PCTSTR pszFileName);
	void DownloadFile(LPCSTR lpszFileName, LPCTSTR lpszDestPath);

	void Start();

	void SetAsyncResultHandler(AsyncResultHandler&& asyncResultHandler);
	void SetIsComplete(bool bIscomplete);

private:
	CMyFileTaskThread		m_FileTask;
	bool					m_IsComplete;	//�Ƿ��ϴ����������ѽ���
	bool					m_IsInit;
	AsyncResultHandler		m_asyncResultHandler;
};

