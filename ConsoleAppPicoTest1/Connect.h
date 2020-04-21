
class Connect
{
private:
	int port;
	char* host;
	char* dbName;
	char* rootName;
	char* password;
public:
	Connect(char* _host, char* _rootName, char* _password, char* _dbname, int _port);
	int getPort()const;
	const char* getHost();
	const char* getDbName();
	const char* getRootName();
	const char* getPassword();



};