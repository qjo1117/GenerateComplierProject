#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

class Debugger
{
private:
	Debugger() { }
	~Debugger() { }
public:
	static Debugger& GetInstance()
	{
		static Debugger instance;
		return instance;
	}

public:
	bool Init();
	void Run();

private:
	std::thread m_thread;
	
};

