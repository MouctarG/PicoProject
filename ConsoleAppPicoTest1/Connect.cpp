#include "Connect.h"
Connect::Connect(char* _host, char* _rootName, char* _password, char* _dbname, int _port) :
   port(_port),host(_host),
dbName(_dbname),rootName(_rootName),password(_password){};

 int Connect::getPort()const { return port; }
 const char* Connect::getDbName()  { return dbName;}
 const char* Connect::getHost() { return host; }
const char* Connect::getPassword()  { return password; };
const char* Connect::getRootName()  { return rootName; }

