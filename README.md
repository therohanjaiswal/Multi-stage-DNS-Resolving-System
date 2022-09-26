# Multi-stage-DNS-Resolving-System using Socket Programming
* We have implemented 3 programs Client.cpp, ProxyServer.cpp and DNSServer.cpp.
* Open 3 terminals where one terminal will behave like a live client, second one will behave like Proxy Server and third one will behave like a DNS Server.
* Compile 3 programs with the command (g++ Client.cpp –o client, g++ ProxyServer.cpp and g++ DNSServer.cpp –o dns).
* 3 Executable files will be generated as client, proxy and dns. We have to run all the 3 files with proper argument.
* We have considered all 3 programs run on a local host (127.0.0.1).
* While running Client.cpp we have to provide the IP address and Port number, for running the DNSServer.cpp we have to provide the DNS Server’s port number 
and while running ProxyServer.cpp we have to provide the port number for the proxy server.
* Eg: <br/>
  * ```$ . /dns 4207```
  * ```$ . /proxy 4209```
  * ```$ . /client 127.0.0.1 4209```
