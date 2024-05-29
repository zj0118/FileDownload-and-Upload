#pragma once
#include "Thread.h"
#include <list>
#include <mutex>
#include <condition_variable>
#include "Platform.h"
#include "ProtocolData.h"

class CFileItemRequest;
class CUploadFileResult;
class CFileClient;

class CMyFileTaskThread : public CThread
{
public:
	CMyFileTaskThread();
	virtual ~CMyFileTaskThread(void);

public:
	virtual void Stop() override;

	BOOL AddItem(CFileItemRequest* pItem);
	void RemoveItem(CFileItemRequest* pItem);					//�Ƴ����������ػ����ϴ���������
	void ClearAllItems();

	void HandleItem(CFileItemRequest* pFileItem);

protected:
	virtual void Run() override;

private:
	//pReflectionCallBack���ڻص������̣߳�����ϴ����صĽ�����Ϣ
	long UploadFile(PCTSTR pszFileName, AsyncResultHandler pReflectionCallBack, /*HANDLE hCancelEvent,*/ CUploadFileResult& uploadFileResult);
	void FillUploadFileResult(CUploadFileResult& uploadFileResult, PCTSTR pszLocalName, PCSTR pszRemoteName, int64_t nFileSize, char* pszMd5);
	long DownloadFile(LPCSTR lpszFileName, LPCTSTR lpszDestPath, BOOL bOverwriteIfExist, AsyncResultHandler pReflectionCallBack/*, HANDLE hCancelEvent*/);

public:
	CFileClient* m_lpFileEvent;

private:
	CFileItemRequest*				m_pCurrentTransferringItem;		//��ǰ���ڴ������
	std::list< CFileItemRequest*>	m_Filelist;
	std::mutex						m_mtItems;
	std::condition_variable			m_cvItems;
	int32_t							m_seq;
};

