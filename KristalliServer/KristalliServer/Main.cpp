#include "KristalliServer.h"
#include "ECServer.h"

#include <string.h>
#include <direct.h> // For _getcwd
#include <iostream>

using namespace std;
using namespace clb;

void PrintUsage()
{
    cout << "Usage: " << endl;
    cout << "       tcp|udp <port>" << endl;
}

void PrintWorkingDirectory()
{
   char* buffer;

   // Get the current working directory: 
   if((buffer = _getcwd( NULL, 0 )) == NULL)
      printf("_getcwd error");
   else
   {
      printf("Working directory: %s\n", buffer);
      free(buffer);
   }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        PrintUsage();
        return 0;
    }

    SocketTransportLayer transport = SocketOverUDP;
    if (!stricmp(argv[1], "tcp"))
        transport = SocketOverTCP;
    else if (!!stricmp(argv[1], "udp"))
    {
        cout << "The second parameter is either 'tcp' or 'udp'!" << endl;
        return 0;
    }
    unsigned short port = atoi(argv[2]);

    PrintWorkingDirectory();

    KristalliServer* server = new KristalliServer();
    ECServer* ecServer = new ECServer(server);

    server->RunServer(port, transport);

    delete ecServer;
    delete server;
}
