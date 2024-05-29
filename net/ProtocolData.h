#pragma once
#include <functional>
#include <list>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include "Platform.h"

class CFileClient;
using AsyncResultHandler = std::function<void(CFileClient* client, bool bIsComplete, const std::string& resultStr)>;

enum ResultFlag
{
	enComPlete,
	enProcessing,
	enObtainMd5
};

enum NET_DATA_TYPE
{
	NET_DATA_UNKNOWN,
	NET_DATA_REGISTER,
	NET_DATA_LOGIN,
	NET_DATA_USER_BASIC_INFO,
	NET_DATA_CHANGE_STATUS,
	NET_DATA_GROUP_BASIC_INFO,
	NET_DATA_USER_EXTEND_INFO,
	NET_DATA_FRIENDS_ID,
	NET_DATA_FRIENDS_STATUS,
	NET_DATA_CHAT_MSG,
	NET_DATA_CHAT_CONFIRM_IMAGE_MSG,	//���췢����ͼƬ��׷�ӵ�ȷ����Ϣ
	NET_DATA_FIND_FRIEND,
	NET_DATA_OPERATE_FRIEND,
	NET_DATA_HEARTBEAT,
	NET_DATA_UPDATE_LOGON_USER_INFO,
	NET_DATA_TARGET_INFO_CHANGE,
	NET_DATA_MODIFY_PASSWORD,
	NET_DATA_CREATE_NEW_GROUP,
	NET_DATA_OPERATE_TEAM,              //����µĺ��ѷ���
	NET_DATA_MODIFY_FRIEND_MARKNAME,    //�޸ĺ��ѱ�ע��
	NET_DATA_MOVE_FRIEND,               //�ƶ���������������

	NET_DATA_FILE
};

//�ļ��ϴ���������
enum FILE_ITEM_TYPE
{
	FILE_ITEM_UNKNOWN,
	FILE_ITEM_UPLOAD_CHAT_IMAGE,
	FILE_ITEM_UPLOAD_CHAT_OFFLINE_FILE,
	FILE_ITEM_UPLOAD_USER_THUMB,

	FILE_ITEM_DOWNLOAD_CHAT_IMAGE,
	FILE_ITEM_DOWNLOAD_CHAT_OFFLINE_FILE,
	FILE_ITEM_DOWNLOAD_USER_THUMB,
};

//�ļ����ط��ؽ����
enum FILE_DOWNLOAD_RETCODE
{
	FILE_DOWNLOAD_FAILED,
	FILE_DOWNLOAD_SUCCESS,
	FILE_DOWNLOAD_USERCANCEL	//�û�ȡ������
};

//�ļ��ϴ����ؽ����
enum FILE_UPLOAD_RETCODE
{
	FILE_UPLOAD_FAILED,
	FILE_UPLOAD_SUCCESS,
	FILE_UPLOAD_USERCANCEL		//�û�ȡ���ϴ�
};

//��ȡ�ļ�md5ֵ�����
enum GET_FILE_MD5_RETCODE
{
	GET_FILE_MD5_FAILED,
	GET_FILE_MD5_SUCESS,
	GET_FILE_MD5_USERCANCEL
};

class CNetData
{
public:
	CNetData();
	virtual ~CNetData();
public:
	UINT			m_uType;		//��������
	long			m_nRetryTimes;	//����Ϊ����������ʱ���Դ���
};

class CFileItemRequest : public CNetData
{
public:
	CFileItemRequest();
	virtual ~CFileItemRequest();

public:
	long						m_nID;
	char						m_szUtfFilePath[MAX_PATH];	//�ļ�utf8·�����������أ�����ʱ���
	TCHAR						m_szFilePath[MAX_PATH];		//�ļ�ȫ��·���������ϴ����ϴ�ʱ����, ����ʱ��ΪĿ���ļ�·����
	AsyncResultHandler			m_reFlectionCallBack;		//							 (�Ǳ����ֶ�)
	//HANDLE					m_hCancelEvent;				// ȡ���¼�					 (�Ǳ����ֶ�)
	UINT						m_uSenderID;
	std::set<UINT>				m_setTargetIDs;
	UINT						m_uAccountID;				//�˻�id����������ͷ��
	long						m_nFileType;				//Ŀǰ������ͼƬ�������ļ����Զ���ͷ����������

	BOOL						m_bPending;					//�������Ѿ������ػ����ϴ�ΪFALSE����֮ΪTRUE
};

class CUploadFileResult
{
public:
	CUploadFileResult();
	~CUploadFileResult();

public:
	void Clone(const CUploadFileResult* pSource);

public:
	long			m_nFileType;
	BOOL			m_bSuccessful;					//�Ƿ��ϴ��ɹ�
	int64_t			m_nFileSize;					//�ļ���С
	TCHAR			m_szLocalName[MAX_PATH];		//�����ļ���

	char			m_szMd5[64];					//�ļ���md5ֵ
	char			m_szRemoteName[MAX_PATH];		//�ϴ��ɹ��Ժ��ļ��ڷ������ϵ�·����

	UINT			m_uSenderID;
	std::set<UINT>	m_setTargetIDs;
	//void*			m_reFlectionCallBack;
};

struct FileProgress
{
	TCHAR szDestPath[MAX_PATH];
	long  nPercent;				//�ٷֱȣ�0��100֮��(ֵΪ-1ʱ��Ϊ��ȡ�ļ�md5ֵ)
	long  nVerificationPercent;	//��ȡmd5ֵ���ȣ�0��100��
};

