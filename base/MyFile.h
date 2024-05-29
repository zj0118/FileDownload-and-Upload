#pragma once
#include <iostream>
#include <codecvt>
#include <fstream>
#include <filesystem>
#include <locale>
#include "md5.h"
#include "Platform.h"

int GetFileMd5Value(PCTSTR pszFileName, char* pszMd5, long nMd5Length, int64_t& nFileSize, void* hwndReflection = NULL);

class CAutoFileOperator
{
public:
	CAutoFileOperator(PCTSTR pszFileName, bool isRead = true);
	~CAutoFileOperator();

private:
	DWORD DoGetFileTotalSize();

public:
	void Release();
	bool RemoveFile();
	DWORD GetFileTotalSize();
	DWORD GetReadedSize();
	DWORD GetWrittenSize();
	std::fstream* GetFileHandle() { return &m_fFile; };
	bool IsOpen() { return m_fFile.is_open(); };

	bool ReadContent(char* buffer, int bufferLen);
	bool WriteContent(char* buffer, int bufferLen);


private:
	PCTSTR			m_fileName;
	std::fstream	m_fFile;
	bool			m_bIsRead;
	int64_t			m_readedLength;	//已读取内容的长度
	int64_t			m_FileTotalSize;
	int64_t			m_writtenLength;	//已写入内容的长度
	size_t			m_LastPos;	//文件起始位置
};