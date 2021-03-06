#include <iostream>
#include <poll.h>
#include <string.h>
#include <signal.h>

#include "connector.h"
#include "stream.h"
#include "inotify.h"
#include "filehandlernormal.h"
#include "filehandlerremove.h"
#include "filehandlerfolder.h"
#include "filehandlermove.h"
int FD;

void terminationProtocol(int signal)
{
  Stream stream(FD);
  FileHandler* fh = new FileHandlerNormal("path",4,0);
  stream.send_file( fh);
  exit(0);
}



int main(int argc, char **argv) {

    if(argc < 4){
      std::cout << "Podaj wszystkie argumenty: 1-IP 2-port 3-folder" << std::endl;
      return 1;
    }
        
    const char* addr = argv[1];
    int port = atoi( argv[2]);
    if(port < 1024 || port > 65536){
      std::cout << "Niewlasciwy port" << std::endl;
      return 1;
    }
    const char* dir = argv[3];
    

    
    
    Inotify inotify(dir);

    Connector connector;
    int fd = connector.conn(addr, port);
    
    FD=fd;
    int nfds = 2;
    struct pollfd fds[nfds];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    fds[1].fd = inotify.get_fd();
    fds[1].events = POLLIN;

    Stream stream(fd, dir);
    
    signal(SIGINT,terminationProtocol);

    while(1)
    {
        int rv = poll(fds, nfds, -1);
        if(rv == -1) {
            perror("poll error");
        }
        else if (rv == 0)
        {
            std::cout << "Timeout poll"  << std::endl;
        }



        if (fds[0].revents == 1)
        {
	    //nastopilo zdarzenie od serwera i klient odbiera plik
            FileHandler* fh = stream.recv_file(&inotify);	
            delete fh;
            
        }
        else if(fds[1].revents == 1)
        {
            //zdarzenia od inotify, odbieramy nazwy plikow i wysylamy je do serwera

            std::vector<FileHandler*> x = inotify.readNotify();
            for(std::vector<FileHandler*>::iterator it = x.begin(); it != x.end(); ++it)
            {
                stream.send_file((*it), &inotify);
                delete (*it);
            }

        }

    }


    return 0;
}
