#include "MyIUSocket.h"

#include <functional>
#include "Msg.h"

//包最大字节数限制为10M
#define MAX_PACKAGE_SIZE    10 * 1024 * 1024

CMyIUSocket::CMyIUSocket()
{
    m_hFileSocket = INVALID_SOCKET;
    m_nFilePort = 20001;
    m_strFileServer = "47.94.88.231";
    m_bConnectedOnFileSocket = false;
}
CMyIUSocket::~CMyIUSocket(void)
{
}

CMyIUSocket& CMyIUSocket::GetInstance()
{
    static CMyIUSocket socketInstance;
    return socketInstance;
}

void CMyIUSocket::SetFileServer(const std::string& sFileServer)
{
    m_strFileServer = sFileServer;
    CloseFileServerConnection();
}

void CMyIUSocket::SetFilePort(short nFilePort)
{
    m_nFilePort = nFilePort;
    CloseFileServerConnection();
}

bool CMyIUSocket::ConnectToFileServer(int timeout/* = 3*/)
{
    //TODO: 暂时只支持Windows，考虑兼容Linux
    if (!IsFileServerClosed())
        return true;

    m_hFileSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_hFileSocket == INVALID_SOCKET)
        return false;

    long tmSend = 3 * 1000L;
    long tmRecv = 3 * 1000L;
    long noDelay = 1;
    setsockopt(m_hFileSocket, IPPROTO_TCP, TCP_NODELAY, (LPSTR)&noDelay, sizeof(long));
    setsockopt(m_hFileSocket, SOL_SOCKET, SO_SNDTIMEO, (LPSTR)&tmSend, sizeof(long));
    setsockopt(m_hFileSocket, SOL_SOCKET, SO_RCVTIMEO, (LPSTR)&tmRecv, sizeof(long));

    //将socket设置成非阻塞的
    unsigned long on = 1;
    if (::ioctlsocket(m_hFileSocket, FIONBIO, &on) == SOCKET_ERROR)
        return false;

    struct sockaddr_in addrSrv = { 0 };
    struct hostent* pHostent = NULL;
    unsigned int addr = 0;

    //m_strFileServer如果是域名形式，要先解析成xxxx.xxxx.xxxx.xxxx形式
    if ((addrSrv.sin_addr.s_addr = inet_addr(m_strFileServer.c_str())) == INADDR_NONE)
    {
        pHostent = ::gethostbyname(m_strFileServer.c_str());
        if (!pHostent)
        {
            //写日志
            //LOG_ERROR("Could not connect file server:%s, port:%d.", m_strFileServer.c_str(), m_nFilePort);
            return false;
        }
        else
            addrSrv.sin_addr.s_addr = *((unsigned long*)pHostent->h_addr);
    }

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons((u_short)m_nFilePort);
    int ret = ::connect(m_hFileSocket, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
    if (ret == 0)
    {
        //TODO: 写日志
        //LOG_INFO("Connect to file server:%s, port:%d successfully.", m_strFileServer.c_str(), m_nFilePort);
        m_bConnectedOnFileSocket = true;
        return true;
    }
    else if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
    {
        //TODO: 写日志
        //LOG_ERROR("Could not connect to file server:%s, port:%d.", m_strFileServer.c_str(), m_nFilePort);
        return false;
    }

    //利用select检测FileSocket的写事件是否就绪，就绪说明连接成功
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(m_hFileSocket, &writeset);
    struct timeval tv = { timeout, 0 };
    //超时返回0;失败返回-1；成功返回大于0的整数
    int res = ::select(m_hFileSocket + 1, NULL, &writeset, NULL, &tv);
    if (res != 1)
    {
        //TODO: 写日志
        //LOG_ERROR("Could not connect to file server:%s, port:%d.", m_strFileServer.c_str(), m_nFilePort);
        return false;
    }

    //TODO: 写日志
    //LOG_INFO("Connect to file server:%s, port:%d successfully.", m_strFileServer.c_str(), m_nFilePort);

    m_bConnectedOnFileSocket = true;

    return true;
}

bool CMyIUSocket::IsFileServerClosed()
{
    return !m_bConnectedOnFileSocket;
}

void CMyIUSocket::CloseFileServerConnection()
{
    if (m_hFileSocket == INVALID_SOCKET)
        return;

    ::shutdown(m_hFileSocket, SD_BOTH);
    ::closesocket(m_hFileSocket);
    m_hFileSocket = INVALID_SOCKET;

    //TODO: 写日志
    //LOG_ERROR("Disconnect file server:%s, port:%d.", m_strFileServer.c_str(), m_nFilePort);

    m_bConnectedOnFileSocket = false;
}

bool CMyIUSocket::SendOnFilePort(const char* pBuffer, int64_t nSize, int nTimeout)
{
    //如果未连接则重连，重连也失败则返回FALSE
    if (IsFileServerClosed())
        return false;

    int64_t nStartTime = time(NULL);

    int64_t nSentBytes = 0;
    int nRet = 0;
    int64_t now;
    do
    {
        //FIXME: 将int64_t强制转换成int32可能会有问题
        nRet = ::send(m_hFileSocket, pBuffer + nSentBytes, (int)(nSize - nSentBytes), 0);
        if (nRet == SOCKET_ERROR && ::WSAGetLastError() == WSAEWOULDBLOCK)
        {
            ::Sleep(1);
            now = (int64_t)time(NULL);
            if (now - nStartTime < (int64_t)nTimeout)
                continue;
            else
            {
                //超时了,关闭socket,并返回false
                CloseFileServerConnection();
                //TODO: 写日志
                //LOG_ERROR("Send data timeout, now: %lld, nStartTime: %lld, nTimeout: %d, disconnect file server:%s, port:%d.", now, nStartTime, nTimeout, m_strFileServer.c_str(), m_nFilePort);
                return false;
            }
        }
        else if (nRet < 1)
        {
            //一旦出现错误就立刻关闭Socket
            //TODO: 写日志
            //LOG_ERROR("Send data error, nRet: %d, disconnect file server: %s, port: %d, socket errorCode: %d", nRet, m_strFileServer.c_str(), m_nFilePort, ::WSAGetLastError());
            CloseFileServerConnection();
            return false;
        }

        nSentBytes += (int64_t)nRet;

        if (nSentBytes >= nSize)
            break;

        ::Sleep(1);

    } while (true);

    return true;
}

bool CMyIUSocket::RecvOnFilePort(char* pBuffer, int64_t nSize, int nTimeout)
{
    if (IsFileServerClosed())
        return false;

    int64_t nStartTime = time(NULL);

    int nRet = 0;
    int64_t nRecvBytes = 0;
    int64_t now;

    do
    {
        nRet = ::recv(m_hFileSocket, pBuffer + nRecvBytes, (int)(nSize - nRecvBytes), 0);
        if (nRet == SOCKET_ERROR && ::WSAGetLastError() == WSAEWOULDBLOCK)
        {
            ::Sleep(1);
            now = time(NULL);
            if (now - nStartTime < (int64_t)nTimeout)
                continue;
            else
            {
                //超时了,关闭socket,并返回false
                CloseFileServerConnection();
                //TODO: 写日志
                //LOG_ERROR("Recv data timeout, now: %lld, nStartTime: %lld, nTimeout: %d, disconnect file server:%s, port:%d.", now, nStartTime, nTimeout, m_strFileServer.c_str(), m_nFilePort);
                return false;
            }
        }
        //一旦出现错误就立刻关闭Socket
        else if (nRet < 1)
        {
            //TODO: 写日志
            //LOG_ERROR("Recv data error, nRet: %d, disconnect file server: %s, port: %d, socket errorCode: %d.", nRet, m_strFileServer.c_str(), m_nFilePort, ::WSAGetLastError());
            CloseFileServerConnection();
            return false;
        }

        nRecvBytes += (int64_t)nRet;
        if (nRecvBytes >= nSize)
            break;

        ::Sleep(1);

    } while (true);

    return true;
}
