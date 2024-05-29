#include <algorithm>
#include <iostream>
#include <string>
#include "ConfigFileReader.h"
#include "FileClient.h"

void myHttpResultHandler(CFileClient* client, bool bIsComplete, const std::string& resultStr)
{
    if (bIsComplete)
    {
        client->SetIsComplete(bIsComplete);
    }
    std::cout << resultStr;
}
int main()
{
#ifdef WIN32
    CConfigFileReader config("./config/fileclient.conf");
#else
    CConfigFileReader config("config/fileclient.conf");
#endif
    const char* fileserverIP = config.getConfigName("fileserverip");
    short fileserverPort = (short)atol(config.getConfigName("fileserverport"));

    std::string sFlag;
    std::cout << "Start my File Client now? (Yes or No)" << std::endl;
    getline(std::cin, sFlag);
    transform(sFlag.begin(), sFlag.end(), sFlag.begin(), ::toupper);    //转成大写
    if (sFlag == "YES")
    {
        CFileClient::GetInstance().SetFileServer(fileserverIP);
        CFileClient::GetInstance().SetFilePort(fileserverPort);
        CFileClient::GetInstance().InitNetThreads();
        CFileClient::GetInstance().SetAsyncResultHandler(myHttpResultHandler);
        while (true)
        {
            std::cout << "Continue or not? (Yes or No)" << std::endl;
            getline(std::cin, sFlag);
            transform(sFlag.begin(), sFlag.end(), sFlag.begin(), ::toupper);    //转成大写
            if (sFlag == "NO")
            {
                if (CFileClient::GetInstance().GetIsInit())
                    CFileClient::GetInstance().Uninit();
                break;
            }
            std::cout << "1->Upload File\n";
            std::cout << "2->Download File\n";
            std::cout << "Please input option:\n";
            int nOption;
            std::cin >> nOption;
            switch (nOption)
            {
            case 1:
            {
                std::cout << "Please input the Absolute path of the uploaded file:\n";
                std::string sFilePath;
                std::cin.ignore();
                getline(std::cin, sFilePath);
                int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, sFilePath.c_str(), -1, NULL, 0);
                wchar_t* wideStr = new wchar_t[sizeNeeded];
                MultiByteToWideChar(CP_UTF8, 0, sFilePath.c_str(), -1, wideStr, sizeNeeded);
                const wchar_t* pctFilePath = wideStr;
                CFileClient::GetInstance().UploadFile(pctFilePath);
                delete[] wideStr;
            }
            break;
            case 2:
            {
                std::cout << "Please input the Absolute path you want to save:\n";
                std::string sFilePath;
                std::cin.ignore();
                getline(std::cin, sFilePath);
                std::cout << "Please input the file name in fileserver you want to save:\n";
                std::string sFileName;
                getline(std::cin, sFileName);

                int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, sFilePath.c_str(), -1, NULL, 0);
                wchar_t* wideStr = new wchar_t[sizeNeeded];
                MultiByteToWideChar(CP_UTF8, 0, sFilePath.c_str(), -1, wideStr, sizeNeeded);
                const wchar_t* pctFilePath = wideStr;
                CFileClient::GetInstance().DownloadFile(sFileName.c_str(), pctFilePath);
                delete[] wideStr;
            }
            break;
            default:
                break;
            }
        }
    }

    return 0;
}
