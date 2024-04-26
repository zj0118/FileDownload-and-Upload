# FileDownload-and-Upload
### 文件上传与下载
开发一个用C++开发的文件上传和下载客户端
是以本地文件服务器（fileserver）为服务端

### 功能：
上传功能：一个入参，为想要上传文件的绝对路径。上传成功后会在fileserver中的filecache文件夹里以其文件md5码命名的文件保存；
下载功能：两个入参，一个是自定义想要在本地保存文件的绝对路径，如"D:/abc.mp4"；另一个入参为fileserver的filecache文件夹里有的md5文件名。
