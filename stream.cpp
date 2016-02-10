#include "stream.h"


Stream::Stream(int filedesc)
{
  this->fd = filedesc;
  this->inotify = NULL;
}

Stream::Stream(int filedesc,Inotify *inotify)
{
  this->fd = filedesc;
  this->inotify = inotify;
}

int Stream::send_message(char* buffer, int len)
{
  int bytes_sent = send(fd,buffer,len,0);
  
  return bytes_sent;
}

int Stream::recv_message(char* buffer, int len)
{ 
  int bytes_recv = recv(fd, buffer, len, 0);
  
  
  //std::cout << buffer << std::endl;
  return bytes_recv;
}

int Stream::send_data(std::string path, std::string filename, int filesize, int type)
{
  strncpy(d.path, path.c_str(), path.length() );
  d.pathlength = path.length();
  
  strncpy(d.name, filename.c_str(), filename.length() );
  d.namelength = filename.length();
  
  d.size = filesize;
  d.type = type;
  
  
  int bytes_sent = send(fd,&d,sizeof(d),0);
  
  return bytes_sent;
};

int Stream::recv_data()
{
  int bytes_recv = recv(fd, &d, sizeof(d), 0);
  
  //std::cout << "File: " << d.name << " has: " << d.size << " bytes.  " << d.namesize << std::endl;
  //name = d.name;
  //filesize = d.size;
  return bytes_recv;
}

void Stream::send_file(std::string path, path_name file)
{
  int type, filesize;
  std::string fullpath = path + "/" + file.path + file.name; 
  
  
  struct stat s;
  if( stat(fullpath.c_str(),&s) == 0 )
  {
    if( s.st_mode & S_IFDIR )
    {  
      type = 1;
      filesize = 0;
    }
    else
    {
      type = 0;
      filesize = get_file_size(fullpath);
    }
  }
  else
  {
    type = 0;
    filesize = -1;
  }
  
  send_data(file.path, file.name, filesize, type);	//wyslij nazwe pliku,rozmiar itd..
  
  
  recv_syn();
  
  if(filesize > 0 && type == 0)
  {
  //fullpath = "/home/mati/test/a.txt";
  //std::cout << fullpath << std::endl;
  FILE *pFile = fopen(fullpath.c_str(), "rb");	//otworz plik, czytaj z niego, i wysylaj
  
  int nread;
  char buff[256] = {0};
  int size_to_send = filesize;
  while(size_to_send > 0){
    
    
    nread = fread(buff,1,256,pFile); 
    int bytes_sent = send_message(buff,nread);
    size_to_send -= bytes_sent;
   
    //std::cout << "Bytes sent: " << bytes_sent << std::endl;
    recv_syn();
    
    
    //if (bytes_sent < 256)
    // break;
    
  }
  fclose (pFile);  
  }
  std::cout << "Wyslalem plik: " << file.path << "/" << file.name << "  o rozm.: " << filesize << std::endl;
 

}

path_name Stream::recv_file(std::string path)
{
  
  
  int bytes = recv_data();	//odbierz nazwe,rozmiar itd...
  d.name[d.namelength]='\0';
  d.path[d.pathlength]='\0';
  std::string relpath = d.path;
  std::string name = d.name;
  struct path_name recvdfile = {relpath, name};
  std::string fullpath = path + "/" + relpath + name;
  
  //std::string fullpath = "/home/mati/sv/a.txt";
  //std::cout << fullpath << std::endl;
  
  if (inotify!=NULL)
    inotify->addIgnore(name);
  
  
  send_syn();
  
  
  
  
  if(d.size == -1)
  {
    
    remove( fullpath.c_str() );
  }
  else if(d.type == 1)
  {
   
    mkdir(fullpath.c_str(),0777);
    
    if (inotify!=NULL)
    {
      int new_wd = inotify_add_watch( inotify->get_fd(), fullpath.c_str(), IN_CLOSE_WRITE | IN_CREATE | IN_DELETE |IN_MOVE); 
      std::cout << "Dodaje nowy folder o sciezce absolutnej: " << fullpath << std::endl;
      std::cout << "Dodaje nowy folder o sciezce wzglednej: " << name << std::endl;
      inotify->addSubdir(relpath+name, new_wd);
       
    }
    
  }
  else
  {
    //Jeśli Inotify był podany w konstruktorze, to dodajemy plik który ma ignorować
 
    
    
    FILE *pFile = fopen(fullpath.c_str(), "wb");	//otworz plik o podanej nazwie i zapisuj do niego
  
    if(d.size > 0)
    {
     
      char recvbuf[256] = {0};
      int bytesReceived=0;
      int filesize = d.size;
  
      while(filesize > 0)
      {
	//std::cout << "bytesReceived: " << bytesReceived << std::endl;
	
	//std::cout << "sizeof: " << strlen(recvbuf) << std::endl;
	bytesReceived = recv_message(recvbuf,256); 
	int bytes_sent = fwrite(recvbuf, 1,bytesReceived,pFile);
	filesize -= bytes_sent;
	
	//if(bytesReceived < 256)
	 // break;
	send_syn();
      }
    }
    
    fclose (pFile);
    
  }
  
  std::cout << "Odebralem plik: " << name << "  o rozm.: " << d.size << std::endl;
  
  //if (inotify!=NULL)
  //  inotify->removeIgnore(name);
  
  return recvdfile;
}

int Stream::get_file_size(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int Stream::get_fd()
{
  return fd;
}

void Stream::send_syn()
{
  char buf[4] = "get";
  send_message(buf,4);
}

void Stream::recv_syn()
{
  char buf[4];
  recv_message(buf,4);
}