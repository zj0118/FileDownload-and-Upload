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
	NET_DATA_CHAT_CONFIRM_IMAGE_MSG,	//聊天发送完图片后追加的确认信息
	NET_DATA_FIND_FRIEND,
	NET_DATA_OPERATE_FRIEND,
	NET_DATA_HEARTBEAT,
	NET_DATA_UPDATE_LOGON_USER_INFO,
	NET_DATA_TARGET_INFO_CHANGE,
	NET_DATA_MODIFY_PASSWORD,
	NET_DATA_CREATE_NEW_GROUP,
	NET_DATA_OPERATE_TEAM,              //添加新的好友分组
	NET_DATA_MODIFY_FRIEND_MARKNAME,    //修改好友备注名
	NET_DATA_MOVE_FRIEND,               //移动好友至其他分组

	NET_DATA_FILE
};

//文件上传下载类型
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

//文件下载返回结果码
enum FILE_DOWNLOAD_RETCODE
{
	FILE_DOWNLOAD_FAILED,
	FILE_DOWNLOAD_SUCCESS,
	FILE_DOWNLOAD_USERCANCEL	//用户取消下载
};

//文件上传返回结果码
enum FILE_UPLOAD_RETCODE
{
	FILE_UPLOAD_FAILED,
	FILE_UPLOAD_SUCCESS,
	FILE_UPLOAD_USERCANCEL		//用户取消上传
};

//获取文件md5值结果码
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
	UINT			m_uType;		//数据类型
	long			m_nRetryTimes;	//当作为发送数据项时重试次数
};

class CFileItemRequest : public CNetData
{
public:
	CFileItemRequest();
	virtual ~CFileItemRequest();

public:
	long						m_nID;
	char						m_szUtfFilePath[MAX_PATH];	//文件utf8路径（用于下载，下载时必填）
	TCHAR						m_szFilePath[MAX_PATH];		//文件全饰路径（用于上传，上传时必填, 下载时作为目标文件路径）
	AsyncResultHandler			m_reFlectionCallBack;		//							 (非必须字段)
	//HANDLE					m_hCancelEvent;				// 取消事件					 (非必须字段)
	UINT						m_uSenderID;
	std::set<UINT>				m_setTargetIDs;
	UINT						m_uAccountID;				//账户id，用于下载头像
	long						m_nFileType;				//目前有聊天图片、离线文件和自定义头像三种类型

	BOOL						m_bPending;					//当该项已经在下载或者上传为FALSE，反之为TRUE
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
	BOOL			m_bSuccessful;					//是否上传成功
	int64_t			m_nFileSize;					//文件大小
	TCHAR			m_szLocalName[MAX_PATH];		//本地文件名

	char			m_szMd5[64];					//文件的md5值
	char			m_szRemoteName[MAX_PATH];		//上传成功以后文件在服务器上的路径名

	UINT			m_uSenderID;
	std::set<UINT>	m_setTargetIDs;
	//void*			m_reFlectionCallBack;
};

struct FileProgress
{
	TCHAR szDestPath[MAX_PATH];
	long  nPercent;				//百分比，0～100之间(值为-1时，为获取文件md5值)
	long  nVerificationPercent;	//获取md5值进度（0～100）
};

