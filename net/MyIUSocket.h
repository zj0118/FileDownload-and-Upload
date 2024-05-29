#pragma once

#include <thread>
#include <mutex>
#include <string>
#include <memory>
#include <condition_variable>
#include <stdint.h>

#include "Platform.h"


//����ͨ�Ų�ֻ�������ݴ���ͽ���
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
	*@param timeout ��ʱʱ�䣬��λΪs
	**/

	bool ConnectToFileServer(int timeout = 3);
	bool IsFileServerClosed();
	void CloseFileServerConnection();

	//ͬ���ӿ�
	//nTimeout��λ����
	bool SendOnFilePort(const char* pBuffer, int64_t nSize, int nTimeout = 30);
	bool RecvOnFilePort(char* pBuffer, int64_t nSize, int nTimeout = 30);

private:
	SOCKET							m_hFileSocket;			//���ļ���Socket������socket��
	short							m_nFilePort;
	std::string						m_strFileServer;		//�ļ���������ַ

	bool							m_bConnectedOnFileSocket;

};

