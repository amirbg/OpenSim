// Header file for named pipe server
//
#include "windows.h"
#include "iostream"

//Name given to the pipe
//Pipe name format - \\.\pipe\pipename



#define BUFFER_SIZE 4096 //1k
#define ACK_MESG_RECV "Message received successfully"



class NamedPipe
{
	public:		
		NamedPipe(HANDLE *arghPipe, LPCSTR argpipeName);
		bool pipeCreator();
		bool pipeReaderCreator();
		bool waitForClient();
		char *readMessage();
		bool writeMessage(char *szBuffer);
		char *readInput(double inpt1, double inpt2, double inpt3);
		void closeHandle();


	private:
		char classBuffer[1024];
		HANDLE hPipe;
		LPCSTR pipeName;
		DWORD cbBytes;

};

NamedPipe::NamedPipe(HANDLE *arghPipe, LPCSTR argpipeName)
{
	hPipe = *arghPipe;
	pipeName = argpipeName;
}

bool NamedPipe::pipeCreator()
{
	hPipe = CreateNamedPipe(
		pipeName,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUFFER_SIZE,              // output buffer size 
		BUFFER_SIZE,              // input buffer size 
		NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
		NULL);                    // default security attribute 

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		printf("\nError occurred while "
			"creating the pipe: %d", GetLastError());
		return 1;
	}
	else
	{
		printf("\nCreateNamedPipe() was successful.");
		return 0;
	}

}


bool NamedPipe::pipeReaderCreator()
{
	hPipe = CreateFile(
		pipeName,   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		printf("\nError occurred while connecting"
			" to the server: %d", GetLastError());
		//One might want to check whether the server pipe is busy
		//This sample will error out if the server pipe is busy
		//Read on ERROR_PIPE_BUSY and WaitNamedPipe() for that
		return 1;  //Error
	}
	else
	{
		printf("\nCreateFile() was successful.");
		return 0;
	}
}

bool NamedPipe::waitForClient()
{
	BOOL bClientConnected = ConnectNamedPipe(hPipe, NULL);

	if (FALSE == bClientConnected)
	{
		printf("\nError occurred while connecting"
			" to the client: %d", GetLastError());
		CloseHandle(hPipe);
		return 1;  //Error
	}
	else
	{
		printf("\nConnectNamedPipe() was successful.");
		return 0;
	}
}

char *NamedPipe::readMessage()
{
	BOOL bResult = ReadFile(
		hPipe,                // handle to pipe 
		classBuffer,             // buffer to receive data 
		sizeof(classBuffer),     // size of buffer 
		&cbBytes,             // number of bytes read 
		NULL);                // not overlapped I/O 

	if ((!bResult) || (0 == cbBytes))
	{
		printf("\nError occurred while reading "
			"from the client: %d", GetLastError());
		CloseHandle(hPipe);
		return 0x0;  //Error
	}
	else
	{
		//printf("\nReadFile() was successful.");
		return classBuffer;
	}
}

bool NamedPipe::writeMessage(char *szBuffer)
{
	BOOL bResult = WriteFile(
		hPipe,                // handle to pipe 
		classBuffer,             // buffer to write from 
		strlen(classBuffer) + 1,   // number of bytes to write, include the NULL 
		&cbBytes,             // number of bytes written 
		NULL);                // not overlapped I/O 

	if ((!bResult) || (strlen(classBuffer) + 1 != cbBytes))
	{
		printf("\nError occurred while writing"
			" to the client: %d", GetLastError());
		CloseHandle(hPipe);
		return 1;  //Error
	}
	else
	{
		//printf("\nWriteFile() was successful.");
		szBuffer = classBuffer;
		return 0;
	}

}

char *NamedPipe::readInput(double inpt1, double inpt2, double inpt3)
{
	int length1 = 0;
	int length2 = 0;
	int length3 = 0;

	string str = std::to_string(inpt1);
	length1 = str.length();
	
	for (int i = 0; i < length1; i++)
		classBuffer[i] = str[i];

	classBuffer[length1] = 'N';

	string str2 = std::to_string(inpt2);
	length2 = str2.length();

	for (int i = 0; i < length2; i++)
		classBuffer[length1 + i + 1] = str2[i];

	classBuffer[length1 + length2 + 1] = 'N';

	string str3 = std::to_string(inpt3);
	length3 = str3.length();

	for (int i = 0; i < length3; i++)
		classBuffer[length1 + length2 + i + 2] = str3[i];

	

	classBuffer[length1 + length2 + length3 + 2] = NULL;
	return classBuffer;
}



void NamedPipe::closeHandle()
{
	CloseHandle(hPipe);
}