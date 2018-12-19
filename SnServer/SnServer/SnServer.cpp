// SnServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Time.h"
#include "LogThread.h"
#include "EchoServer.h"
#include "SigProcess.h"

int main()
{
	std::cout << NsTime::GetStrTimeStamp() << std::endl;
	LogThread& thread = LogThread::getInstance();
	registeSig();
	//FILE * file = fopen("/dev/null","w");
	FILE * file = fopen("/dev/stdout","we");
	thread.addLogFile("info", file);
	//thread.addLogFile("info", "ns.log");
	thread.run();
	EchoServer server;
	server.run();
	thread.join();
    return 0;
}

