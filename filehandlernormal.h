#ifndef FILEHANDLERNORMAL_H
#define FILEHANDLERNORMAL_H


#include "filehandler.h"
#include "inotify.h"
#include "stream.h"

class FileHandlerNormal : public FileHandler{
public:
  FileHandlerNormal(std::string path, int type, int size=0);
  virtual ~FileHandlerNormal();
  virtual void sendChunks(Stream* stream);
  virtual void processFile(Stream* stream);
};

#endif