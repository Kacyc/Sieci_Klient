#ifndef INOTIFY_H
#define INOTIFY_H

#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <algorithm>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

//struktura przechowujaca sciezke wzgledna do podfolderu i jego watch deskryptor
struct fold_wd
{
  std::string path;
  int wd;
};

//struktura przechowujaca sciezke wzgledna pliku i jego nazwe
struct path_name
{
    std::string path;
    std::string name;
};

class Inotify
{
private:
  const char* path;
  int fd;
  int wd;
  std::vector<std::string> ignoreList;
  std::vector<fold_wd> subdirs;
  std::vector<int> cookies;
public:
  Inotify(const char* path);
  ~Inotify();
  
  std::vector<path_name> readNotify();
  std::string addzero(int x);
  std::string returndate(const char* path);
  std::string get_path();
  void add_watch();
  void remove_watch();
  int get_fd();
  std::string get_rel_path(int event_wd,std::string event_name);	//zwraca sciezke wzgledna do pliku o nazwie event_name (bez nazwy)
  bool has_suffix(const std::string &str, const std::string &suffix);
  bool is_cookie_on_vector(int cookie);
  void delete_cookie(int cookie);
  void addIgnore(std::string);
  void addSubdir(std::string rel_path, int wd);
  void removeIgnore(std::string);
  bool shouldIgnore(std::string);
  std::vector<path_name> listrecursive(std::string path);
  std::vector<path_name> listdir(std::string path, std::string folder);
  
};


#endif