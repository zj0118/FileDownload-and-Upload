# FileDownload-and-Upload

开发一个用C++开发的文件上传和下载客户端
是以本地文件服务器（fileserver）为服务端

### 功能：
上传功能：一个入参，为想要上传文件的绝对路径。上传成功后会在fileserver中的filecache文件夹里以其文件md5码命名的文件保存；
下载功能：两个入参，一个是自定义想要在本地保存文件的绝对路径，如"D:/abc.mp4"；另一个入参为fileserver的filecache文件夹里有的md5文件名。

### 使用：
server端配置可以在/config/fileclient.comf更改，然后使用即可
