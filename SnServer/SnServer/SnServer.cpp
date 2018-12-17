// SnServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Time.h"
#include "LogThread.h"
#include"TcpServer.h"

int main()
{
	std::cout << NsTime::GetStrTimeStamp() << std::endl;

	LogThread& thread = LogThread::getInstance();
	thread.addLogFile("info", "nslog.log");
	thread.run();

	EchoServer server;
	server.run();

    return 0;
}

