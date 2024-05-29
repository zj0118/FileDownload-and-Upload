#pragma once

#include <thread>
#include <mutex>
#include <string>
#include <memory>
#include <condition_variable>
#include <stdint.h>

#include "Platform.h"


//网络通信层只负责数据传输和接收
class CMyIUSocket
{
private:
	CMyIUSocket();
	~CMyIUSocket(void);

	CMyIUSocket(const CMyIUSocket& rhs) = delete;
	CMyIUSocket& operator = (const CMyIUSocket& rhs) = delete;

public:
	static CMyIUSocket& GetInstance();
	void SetFileServer(const std::string& sFileServer);
	void SetFilePort(short nFilePort);

	/**
	*@param timeout 超时时间，单位为s
	**/

	bool ConnectToFileServer(int timeout = 3);
	bool IsFileServerClosed();
	void CloseFileServerConnection();

	//同步接口
	//nTimeout单位是秒
	bool SendOnFilePort(const char* pBuffer, int64_t nSize, int nTimeout = 30);
	bool RecvOnFilePort(char* pBuffer, int64_t nSize, int nTimeout = 30);

private:
	SOCKET							m_hFileSocket;			//传文件的Socket（阻塞socket）
	short							m_nFilePort;
	std::string						m_strFileServer;		//文件服务器地址

	bool							m_bConnectedOnFileSocket;

};

