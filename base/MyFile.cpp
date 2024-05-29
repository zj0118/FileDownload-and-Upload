#include <iostream>
#include "MyFile.h"
#include "ProtocolData.h"

int GetFileMd5Value(PCTSTR pszFileName, char* pszMd5, long nMd5Length, int64_t& nFileSize, void* hwndReflection)
{
	CAutoFileOperator autoFile(pszFileName);
	if (! autoFile.IsOpen())
		return GET_FILE_MD5_FAILED;

	nFileSize = autoFile.GetFileTotalSize();
	if (nFileSize == 0)
		return GET_FILE_MD5_FAILED;

	MD5 md5;
	std::fstream* fFile = autoFile.GetFileHandle();
	md5.update(*fFile);
	std::string md5Str = md5.toString();

	//TODO ��ȡmd5У��������ص����̣߳�������ʾ
	if (!md5Str.empty())
		strcpy_s((char*)pszMd5, nMd5Length, md5Str.c_str());

	return !md5Str.empty() ? GET_FILE_MD5_SUCESS : GET_FILE_MD5_FAILED;
}

// class CAutoFileOperator
CAutoFileOperator::CAutoFileOperator(PCTSTR pszFileName, bool isRead /*= true*/) : m_bIsRead(isRead)
{
	if (m_bIsRead)
	{
		//�Զ�������ʽ�����ļ����������ʧ��
		m_fFile.open(pszFileName, std::ios::binary | std::ios::in);
		m_LastPos = m_fFile.tellg();
	}
	else
	{
		//�Զ�����׷����ʽд���ļ������ڻᴴ���ļ�
		m_fFile.open(pszFileName, std::ios::binary | std::ios::out | std::ios::app);
		m_LastPos = m_fFile.tellp();
	}
	m_fileName = pszFileName;
	m_readedLength = 0;
	m_writtenLength = 0;
	m_FileTotalSize = DoGetFileTotalSize();
}

CAutoFileOperator::~CAutoFileOperator()
{
	if (m_fFile.is_open())
		m_fFile.close();
}

DWORD CAutoFileOperator::DoGetFileTotalSize()
{
	int64_t iFileTotalSize = 0;
	if (m_fFile.is_open())
	{
		std::streampos ipos;
		if (m_bIsRead)
		{
			m_fFile.seekg(0, std::ios::end);	//���ļ�ָ���ƶ����ļ�ĩβ
			ipos = m_fFile.tellg();	//���ص�ǰָ���λ�ã�Ҳ�����ļ��Ĵ�С
			m_fFile.seekg(0, std::ios::beg); //  �ļ�ָ��ص��ļ��ײ�
			iFileTotalSize = static_cast<int64_t>(ipos);
		}
		else
		{
			//m_fFile.seekp(0, std::ios::end);	//д�ļ�ָ���ƶ����ļ�ĩβ
			//ipos = m_fFile.tellp();				//���ص�ǰָ���λ�ã�Ҳ�����ļ��Ĵ�С
			//m_fFile.seekp(0, std::ios::beg);	//  �ļ�ָ��ص��ļ��ײ�
			//iFileTotalSize = static_cast<int64_t>(ipos);
			iFileTotalSize = m_writtenLength;
		}
	}
	return iFileTotalSize;
}

void CAutoFileOperator::Release()
{
	if (m_fFile.is_open())
		m_fFile.close();
}

bool CAutoFileOperator::RemoveFile()
{
	::DeleteFile(m_fileName);
	return true;
	//return std::filesystem::remove(fileName);
}

DWORD CAutoFileOperator::GetFileTotalSize()
{
	DWORD dwResult = m_FileTotalSize;
	if (!m_bIsRead)
		dwResult = m_writtenLength;
	return dwResult;
}

DWORD CAutoFileOperator::GetReadedSize()
{
	return m_readedLength;
}

DWORD CAutoFileOperator::GetWrittenSize()
{
	return m_writtenLength;
}

bool CAutoFileOperator::ReadContent(char* buffer, int bufferLen)
{
	bool bret = false;
	if (m_fFile.is_open() && m_bIsRead && (m_readedLength + bufferLen <= m_FileTotalSize))
	{
		m_fFile.read(buffer, bufferLen);
		bret = (bufferLen == m_fFile.gcount());
		if (bret)
			m_readedLength += bufferLen;
	}
	return bret;
}

bool CAutoFileOperator::WriteContent(char* buffer, int bufferLen)
{
	bool bret = false;
	if (m_fFile.is_open() && !m_bIsRead)
	{
		m_fFile.write(buffer, bufferLen);
		m_fFile.flush();
		size_t curPos = m_fFile.tellp();
		int iwrittenLen = curPos - m_LastPos;
		bret = (bufferLen == iwrittenLen);
		if (bret)
		{
			m_LastPos = curPos;
			m_writtenLength += bufferLen;
		}
	}
	return bret;
}
