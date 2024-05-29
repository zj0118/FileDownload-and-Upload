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
	bool InitNetThreads();		//初始化网络线程——文件线程
	void Uninit();				//反初始化
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
	bool					m_IsComplete;	//是否上传或者下载已结束
	bool					m_IsInit;
	AsyncResultHandler		m_asyncResultHandler;
};

