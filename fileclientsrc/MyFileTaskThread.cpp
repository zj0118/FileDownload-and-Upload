#include "MyFileTaskThread.h"
#ifdef WIN32
#include <Shlwapi.h>
#include "EncodeUtil.h"
#endif
#include "FileClient.h"
#include "FileMsg.h"
#include "MyFile.h"
#include "MyIUSocket.h"
#include "protocolstream.h"

using namespace net;

CMyFileTaskThread::CMyFileTaskThread() : m_seq(0)
{
	m_lpFileEvent = NULL;
}

CMyFileTaskThread::~CMyFileTaskThread(void)
{
	ClearAllItems();
}

void CMyFileTaskThread::Stop()
{
	m_bStop = true;
	m_cvItems.notify_one();
}

BOOL CMyFileTaskThread::AddItem(CFileItemRequest* pItem)
{
	if (NULL == pItem)
		return FALSE;

	std::lock_guard<std::mutex> guard(m_mtItems);
	m_Filelist.push_back(pItem);
	m_cvItems.notify_one();

	return TRUE;
}

void CMyFileTaskThread::RemoveItem(CFileItemRequest* pItem)
{
	if (NULL == pItem)
		return;

	std::lock_guard<std::mutex> guard(m_mtItems);
	//δ��ʼ���ػ��ϴ�����ֱ���Ƴ�
	if (pItem->m_bPending)
	{
		std::list< CFileItemRequest*>::iterator iter = m_Filelist.begin();
		for (; iter != m_Filelist.end(); ++iter)
		{
			if (pItem == *iter)
			{
				m_Filelist.erase(iter);
				delete pItem;
				break;
			}
		}
	}
}

void CMyFileTaskThread::ClearAllItems()
{
	std::lock_guard<std::mutex> guard(m_mtItems);
	for (auto& iter : m_Filelist)
	{
		delete iter;
	}
	m_Filelist.clear();
}

void CMyFileTaskThread::HandleItem(CFileItemRequest* pFileItem)
{
	if (pFileItem == NULL || pFileItem->m_uType != NET_DATA_FILE)
		return;

	//��ʼ���д���
	pFileItem->m_bPending = FALSE;
	m_pCurrentTransferringItem = pFileItem;

	BOOL bRet = FALSE;
	CUploadFileResult* pUploadFileResult = new CUploadFileResult();
	if (pFileItem->m_nFileType == FILE_ITEM_UPLOAD_CHAT_OFFLINE_FILE)
	{
		//�ϴ��ļ����ʧ�ܣ�����������
		pUploadFileResult->m_uSenderID = pFileItem->m_uSenderID;
		pUploadFileResult->m_setTargetIDs = pFileItem->m_setTargetIDs;
		pUploadFileResult->m_nFileType = pFileItem->m_nFileType;
		//pUploadFileResult->m_reFlectionCallBack = pFileItem->m_reFlectionCallBack;
		long nRetCode;
		while (pFileItem->m_nRetryTimes < 3)
		{
			nRetCode = UploadFile(pFileItem->m_szFilePath, pFileItem->m_reFlectionCallBack, *pUploadFileResult);
			if (nRetCode == FILE_UPLOAD_SUCCESS || nRetCode == FILE_UPLOAD_USERCANCEL)
				break;

			::Sleep(3000);

			++pFileItem->m_nRetryTimes;
		}

		//�����û�ȡ���������ϴ��ɹ���ʧ�ܶ�Ҫ�����û�
		//TODO: ����ص���������߳�������ʾ���
		if (nRetCode != FILE_UPLOAD_USERCANCEL)
		{
			std::string sTip;
			if (nRetCode == FILE_UPLOAD_SUCCESS)
				sTip = "Upload File Successfully............\n";
			else
				sTip = "Fail to Upload File............\n";
			
			pFileItem->m_reFlectionCallBack(m_lpFileEvent, true, sTip);
		}

	}
	//���������ļ�
	else if (pFileItem->m_nFileType == FILE_ITEM_DOWNLOAD_CHAT_OFFLINE_FILE)
	{
		long nRetCode;
		while (pFileItem->m_nRetryTimes < 3)
		{
			nRetCode = DownloadFile(pFileItem->m_szUtfFilePath, pFileItem->m_szFilePath, TRUE, pFileItem->m_reFlectionCallBack);
			if (nRetCode == FILE_DOWNLOAD_SUCCESS || nRetCode == FILE_DOWNLOAD_USERCANCEL)
				break;

			++pFileItem->m_nRetryTimes;
			::Sleep(3000);
		}

		//TODO: ����ص���������߳�������ʾ���
		std::string sTip;
		if (nRetCode == FILE_DOWNLOAD_SUCCESS)
		{
			sTip = "Dpload File Successfully............\n";
			pFileItem->m_reFlectionCallBack(m_lpFileEvent, true, sTip);
		}
		else if (nRetCode == FILE_DOWNLOAD_FAILED)
		{
			sTip = "Fail to Dpload File............\n";
			pFileItem->m_reFlectionCallBack(m_lpFileEvent, true, sTip);
		}
	}

	delete pFileItem;

	m_pCurrentTransferringItem = NULL;
}

void CMyFileTaskThread::Run()
{
	while (!m_bStop)
	{
		CFileItemRequest* pFileItem;
		{
			std::unique_lock<std::mutex> guard(m_mtItems);
			while (m_Filelist.empty())
			{
				if (m_bStop)
					return;

				m_cvItems.wait(guard);
			}

			pFileItem = m_Filelist.front();
			m_Filelist.pop_front();
		}

		HandleItem(pFileItem);
	}
}

//TODO: �������������ļ����ⲿ������ԣ������ļ�md5Ҳ������һ�Σ����Ե�ֻ������ͨ�Ų���
//�޸ĵ�,�ĳɴ�һ���ļ�������ֻ��������ͨ�Ų��֡�
long CMyFileTaskThread::UploadFile(PCTSTR pszFileName, AsyncResultHandler pReflectionCallBack, CUploadFileResult& uploadFileResult)
{
	long nBreakType = FILE_UPLOAD_SUCCESS;

	_tcscpy_s(uploadFileResult.m_szLocalName, ARRAYSIZE(uploadFileResult.m_szLocalName), pszFileName);

	//�ļ�md5ֵ
	char szMd5[64] = { 0 };
	//TODO: ��ʾ�û�����У���ļ�
	std::string sTip = "Obtaining the file Md5Code............\n";
	pReflectionCallBack(m_lpFileEvent, false, sTip);

	int64_t nFileSize;
	long nRetCode = GetFileMd5Value(pszFileName, szMd5, ARRAYSIZE(szMd5), nFileSize, NULL);
	if (nRetCode == GET_FILE_MD5_FAILED)
	{
		//TODO: д��־
		//LOG_INFO(_T("Failed to upload file:%s as unable to get file md5."), pszFileName);
		return FILE_UPLOAD_FAILED;
	}
	else if (nRetCode == GET_FILE_MD5_USERCANCEL)
	{
		//TODO: д��־
		//LOG_INFO(_T("User cancel to upload file:%s."), pszFileName);
		return FILE_UPLOAD_USERCANCEL;
	}

	sTip = "Get the file Md5Code Successfully............\n";
	pReflectionCallBack(m_lpFileEvent, false, sTip);

	//0�ֽڵ��ļ������ϴ�
	if (nFileSize == 0)
	{
		//TODO: д��־
		//LOG_ERROR(_T("Failed to upload file:%s as file size is 0."), pszFileName);
		return FILE_UPLOAD_FAILED;
	}

	uploadFileResult.m_nFileSize = nFileSize;
	strcpy_s(uploadFileResult.m_szMd5, ARRAYSIZE(uploadFileResult.m_szMd5), szMd5);

	CAutoFileOperator autoFile(pszFileName);
	if (!autoFile.IsOpen())
	{
		//д��־
		//LOG_ERROR(_T("Failed to upload file:%s as unable to open the file."), pszFileName);
		return FILE_UPLOAD_FAILED;
	}

	CMyIUSocket& iusocket = CMyIUSocket::GetInstance();
	if (!iusocket.ConnectToFileServer())
	{
		//д��־
		//LOG_ERROR(_T("Failed to connect to FileServer when upload file:%s as unable to open the file."), pszFileName);
		return FILE_UPLOAD_FAILED;
	}

	//�ļ�utf8��ʽ����
	char szUtf8Name[MAX_PATH] = { 0 };
#ifdef WIN32
	EncodeUtil::UnicodeToUtf8(::PathFindFileName(pszFileName), szUtf8Name, ARRAYSIZE(szUtf8Name));
#else
#endif

	int64_t offsetX = 0;	//���ϴ���ƫ����
	while (true)
	{
		std::string outbuf;
		BinaryStreamWriter writeStream(&outbuf);
		writeStream.WriteInt32(msg_type_upload_req);
		writeStream.WriteInt32(m_seq);
		writeStream.WriteCString(szMd5, 32);
		writeStream.WriteInt64(offsetX);
		writeStream.WriteInt64(nFileSize);
		int64_t eachFileSize = 512 * 1024;	//ÿ���ϴ����ֽڣ��ȳ�ʼΪ512*1024
		if (nFileSize - offsetX < eachFileSize)
			eachFileSize = nFileSize - offsetX;

		char* buffer = new char[eachFileSize];
		DWORD dwFileRead;
		if (!autoFile.ReadContent(buffer, eachFileSize))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as ReadFile error, errorCode: %d"), pszFileName, (int32_t)::GetLastError());
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		std::string fileData;
		fileData.append(buffer, eachFileSize);
		writeStream.WriteString(fileData);
		writeStream.Flush();
		file_msg headerx = { outbuf.length() };
		outbuf.insert(0, (const char*)&headerx, sizeof(headerx));
		if (!iusocket.SendOnFilePort(outbuf.c_str(), (int64_t)outbuf.length()))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as SendOnFilePort error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}
		delete[] buffer;

		offsetX += eachFileSize;
		//TODO: �ص��ϴ����ȸ����߳�

		int nProgress = offsetX * 100 / nFileSize;
		char szProgress[100] = { 0 };
		sprintf_s(szProgress, ARRAYSIZE(szProgress), "The Progress of uploading files is %d%%\n", nProgress);
		std::string sPro = szProgress;
		pReflectionCallBack(m_lpFileEvent, false, sPro);

		//�հ�У��
		file_msg header;
		if (!iusocket.RecvOnFilePort((char*)&header, (int64_t)sizeof(header)))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as recv header error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		char* recvBuffer = new char[header.packagesize];
		if (!iusocket.RecvOnFilePort(recvBuffer, header.packagesize))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as recv body error, bodysize: %lld"), pszFileName, header.packagesize);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}
		BinaryStreamReader readStream(recvBuffer, header.packagesize);
		int32_t cmd;
		if (!readStream.ReadInt32(cmd) || cmd != msg_type_upload_resp)
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read cmd error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		//int seq;
		if (!readStream.ReadInt32(m_seq))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read seq error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		int32_t nErrorCode = 0;
		if (!readStream.ReadInt32(nErrorCode))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read ErrorCode error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		std::string filemd5;
		size_t md5length;
		if (!readStream.ReadString(&filemd5, 0, md5length) || md5length != 32)
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read filemd5 error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		int64_t offset;
		if (!readStream.ReadInt64(offset))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read offset error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		int64_t filesize;
		if (!readStream.ReadInt64(filesize))
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read filesize error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}

		std::string dummyfiledata;
		size_t filedatalength;
		if (!readStream.ReadString(&dummyfiledata, 0, filedatalength) || filedatalength != 0)
		{
			//TODO: д��־
			//LOG_ERROR(_T("Failed to upload file: %s as read dummyfiledata error."), pszFileName);
			nBreakType = FILE_UPLOAD_FAILED;
			break;
		}


		if (nErrorCode == file_msg_error_complete)
		{
			FillUploadFileResult(uploadFileResult, pszFileName, filemd5.c_str(), nFileSize, szMd5);
			//TODO: д��־
			//LOG_INFO(_T("Succeed to upload file:%s as there already exist file on server."), pszFileName);

			//TODO: ����ⲿ���ͷ�������ڴ�й¶
			//TODO: �ص������߳�֪ͨ�����

			iusocket.CloseFileServerConnection();
			return FILE_UPLOAD_SUCCESS;
		}
		delete[] recvBuffer;
	}

	iusocket.CloseFileServerConnection();
	return nBreakType;
}

void CMyFileTaskThread::FillUploadFileResult(CUploadFileResult& uploadFileResult, PCTSTR pszLocalName, PCSTR pszRemoteName, int64_t nFileSize, char* pszMd5)
{
	uploadFileResult.m_bSuccessful = TRUE;
	uploadFileResult.m_nFileSize = (DWORD)nFileSize;
	_tcscpy_s(uploadFileResult.m_szLocalName, ARRAYSIZE(uploadFileResult.m_szLocalName), pszLocalName);
	strcpy_s(uploadFileResult.m_szRemoteName, ARRAYSIZE(uploadFileResult.m_szRemoteName), pszRemoteName);
	strcpy_s(uploadFileResult.m_szMd5, ARRAYSIZE(uploadFileResult.m_szMd5), pszMd5);
}

long CMyFileTaskThread::DownloadFile(LPCSTR lpszFileName, LPCTSTR lpszDestPath, BOOL bOverwriteIfExist, AsyncResultHandler pReflectionCallBack)
{
	//TODO: ȷ���Ƿ񸲸ǵķ���Ӧ���Ǹ���md5ֵ���жϱ��ص��ļ������ص��ļ��Ƿ���ȫ��ͬ

	//ƫ����
	long nOffset = 0;
	long nBreakType = FILE_DOWNLOAD_SUCCESS;

	char* pBuffer = NULL;
	//nCurrentContentSizeΪ��ǰ�յ��İ����ļ������ֽ�����С
	long nCurrentContentSize = 0;
	DWORD dwSizeToWrite = 0;
	DWORD dwSizeWritten = 0;
	BOOL bRet = TRUE;
	std::string sTip;

	CAutoFileOperator autoFile(lpszDestPath, false);
	if (!autoFile.IsOpen())
	{
		//TODO: д��־
		//LOG_ERROR(_T("Failed to download file %s as unable to create the file."), lpszDestPath);
		return FILE_DOWNLOAD_FAILED;
	}

	CMyIUSocket& iusocket = CMyIUSocket::GetInstance();
	if (!iusocket.ConnectToFileServer())
	{
		//TODO: д��־
		//LOG_ERROR(_T("Failed to connect to FileServer when download file %s as unable to create the file."), lpszDestPath);
		sTip = "Fail to Connect FileServer............\n";
		pReflectionCallBack(m_lpFileEvent, false, sTip);
		return FILE_DOWNLOAD_FAILED;
	}

	int64_t offsetX = 0;
	while (true)
	{
		std::string outbuf;
		BinaryStreamWriter writeStream(&outbuf);
		writeStream.WriteInt32(msg_type_download_req);
		writeStream.WriteInt32(m_seq);
		writeStream.WriteCString(lpszFileName, strlen(lpszFileName));
		int64_t dummyoffset = 0;
		writeStream.WriteInt64(dummyoffset);
		int64_t dummyfilesize = 0;
		writeStream.WriteInt64(dummyfilesize);
		std::string dummyfiledata;
		writeStream.WriteString(dummyfiledata);
		int32_t clientNetType = client_net_type_broadband;
		writeStream.WriteInt32(clientNetType);
		writeStream.Flush();

		file_msg header = { outbuf.length() };
		outbuf.insert(0, (const char*)&header, sizeof(header));

		if (!iusocket.SendOnFilePort(outbuf.c_str(), (int64_t)outbuf.length()))
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when SendOnFilePort error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		file_msg recvheader;
		if (!iusocket.RecvOnFilePort((char*)&recvheader, (int64_t)sizeof(recvheader)))
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when recv header error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		char* recvBuffer = new char[recvheader.packagesize];
		if (!iusocket.RecvOnFilePort(recvBuffer, recvheader.packagesize))
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when recv body error, bodysize: %lld", lpszFileName, recvheader.packagesize);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		BinaryStreamReader readStream(recvBuffer, (size_t)recvheader.packagesize);
		int32_t cmd;
		if (!readStream.ReadInt32(cmd) || cmd != msg_type_download_resp)
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read cmd error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		//int seq;
		if (!readStream.ReadInt32(m_seq))
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read seq error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		int32_t nErrorCode;
		if (!readStream.ReadInt32(nErrorCode))
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read nErrorCode error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		if (nErrorCode == file_msg_error_not_exist)
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error as file not exist on server", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			char szTip[200] = { 0 };
			sprintf_s(szTip, ARRAYSIZE(szTip), "DownloadFile %s error as file not exist on server\n", lpszFileName);
			std::string strTip = szTip;
			pReflectionCallBack(m_lpFileEvent, false, strTip);
			break;
		}

		std::string filemd5;
		size_t md5length;
		if (!readStream.ReadString(&filemd5, 0, md5length) || md5length == 0)
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read filemd5 error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		int64_t offset;
		if (!readStream.ReadInt64(offset))
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read offset error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		int64_t filesize;
		if (!readStream.ReadInt64(filesize) || filesize <= 0)
		{
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read filesize error", lpszFileName);
			nBreakType = FILE_DOWNLOAD_FAILED;
			break;
		}

		std::string filedata;
		size_t filedatalength;
		if (!readStream.ReadString(&filedata, 0, filedatalength) || filedatalength == 0)
		{
			nBreakType = FILE_DOWNLOAD_FAILED;
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when read filedata error", lpszFileName);
			break;
		}

		if (!autoFile.WriteContent((char*)filedata.c_str(), filedata.length()))
		{
			nBreakType = FILE_DOWNLOAD_FAILED;
			//TODO: д��־
			//LOG_ERROR("DownloadFile %s error when WriteFile error, errorCode: %d", lpszFileName, (int32_t)::GetLastError());
			break;
		}

		offsetX += (int64_t)filedata.length();

		//TODO: �ص������߳���ʾ����
		int nProgress = offsetX * 100 / filesize;
		char szProgress[100] = { 0 };
		sprintf_s(szProgress, ARRAYSIZE(szProgress), "The Progress of Downloading file is %d%%\n", nProgress);
		std::string sPro = szProgress;
		pReflectionCallBack(m_lpFileEvent, false, sPro);

		if (nErrorCode == file_msg_error_complete)
		{
			nBreakType = FILE_DOWNLOAD_SUCCESS;
			break;
		}

		delete[] recvBuffer;
	}// end while-loop

	iusocket.CloseFileServerConnection();

	//���سɹ�
	if (nBreakType == FILE_DOWNLOAD_SUCCESS)
	{
		//TODO: �ص����߳���ʾ�ɹ�
	}
	//����ʧ��
	else
	{
		//TODO: �ص����߳���ʾʧ��
		autoFile.Release();
		autoFile.RemoveFile();
	}

	return nBreakType;
}
