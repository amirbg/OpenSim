//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "net.h"

using namespace std;
using namespace net;

/*
float convertByteToSingle(unsigned char* number);
void printDataFromPacket(unsigned char* startingIndexInBuffer, Address *address, int *numberOfBytesInPacket, int* dataIndex);
int calculateDataCount(int packetSize);
unsigned char *startOfDataInPacket(unsigned char *buffer, int dataIndex);
*/

#define TIMEOUT_CONST 10

struct DataReceiver
{
private:
	Socket socket;
	char c = ' ';
	int numberOfBytesInPacket = 0;
	Address sender;
	unsigned char buffer[1000];
	unsigned char* headOfBinary = NULL;
	int numberOfDataSeries = 0;
	float outputData[24];
	float nullFloat = NULL;
	float currentFlaotOutputBuffer[8];
	int timeoutCounter = 0; 

	float convertByteToSingle(unsigned char* number)
	{

		struct BinaryFloat
		{
			union
			{
				unsigned char buf[4];
				float number = 0.0;
			};
		};
		
		BinaryFloat objBF;

		for (int i = 0; i < 4; i++)
		{
			//printf("Current ASCII Code: %d\n", *(number + i));
			objBF.buf[i] = *(number + i);
		}
		//printf("Number is: %f", single_number.number);

		return objBF.number;
	}
	
	int calculateDataCount(int packetSize)
	{ 
		return (packetSize - 5) / 36; 
	}
	
	
	void printDataFromPacket(unsigned char* startingIndexInBuffer, Address *senderAddress, int *numberOfBytesInPacket, int* dataIndex, float *startOfOutputPacket)
	{
		char counter = 0;

		if (*dataIndex == 0)
			printf("\n\nReceived packet from %d.%d.%d.%d:%d (%d bytes)\n\n\n",
			senderAddress->GetA(), senderAddress->GetB(), senderAddress->GetC(), senderAddress->GetD(), senderAddress->GetPort(), numberOfBytesInPacket);

		printf("Series %d:	  ", *dataIndex + 1);
		for (int add_index = 0; add_index < 4; add_index++)
			printf("%f	", convertByteToSingle(startingIndexInBuffer + add_index * 4));
		printf("                  ");
		for (int add_index = 4; add_index < 8; add_index++)
			printf("%f	", convertByteToSingle(startingIndexInBuffer + add_index * 4));

		printf("-------------------------------------------------------------------------------\n");

		switch (*dataIndex + 1)
		{
		case 1:
			for (int add_index = 0; add_index < 4; add_index++)
				*(startOfOutputPacket + add_index) = convertByteToSingle(startingIndexInBuffer + add_index * 4);
			break;
		case 2:
			for (int add_index = 4; add_index < 7; add_index++)
			{
				*(startOfOutputPacket + add_index) = convertByteToSingle(startingIndexInBuffer + counter * 4);
				counter++;
			}

			break;
		case 3:
			for (int add_index = 7; add_index < 10; add_index++)
			{
				*(startOfOutputPacket + add_index) = convertByteToSingle(startingIndexInBuffer + counter * 4);
				counter++;
			}
			break;
		default:
			cout << "Error!";
		}
	}
	
	unsigned char *startOfDataInPacket(unsigned char *buffer, int dataIndex)
	{
		int startIndex = 5 + dataIndex * 32 + (dataIndex + 1) * 4;
		return (unsigned char*)(&buffer[startIndex]);
	}

	void receiveNextData()
	{
		while (numberOfBytesInPacket == 0)
		{
			try
			{
				numberOfBytesInPacket = socket.Receive(sender, buffer, sizeof(buffer));
			}
			catch (exception& e) {
				cout << "Exception: " << e.what() << endl;
			}
		}

	}



public:

	bool initialize()
	{
		if (InitializeSockets())
		{
			printf("failed to initialize sockets\n");
			return 1;
		}

		// create socket
		int port = 49001;
		printf("creating socket on port %d\n", port);



		if (!socket.Open(port))
		{
			printf("failed to create socket!\n");
			return 1;
		}
	}

	//This is an old function which is not required anymore. After it becoming too complicated the function fillAndReturnOutputBuffer was
	//replaced
	float *getNextData(bool *newDataFlag)
	{
		*newDataFlag = false;
		if (readSocket())
		{
			numberOfDataSeries = calculateDataCount(numberOfBytesInPacket);

			for (int dataIndex = 0; dataIndex < numberOfDataSeries; dataIndex++)
				printDataFromPacket(startOfDataInPacket(buffer, dataIndex), &sender, &numberOfBytesInPacket, &dataIndex, outputData);

			*newDataFlag = true;
		}
		else
			*newDataFlag = false;

		numberOfBytesInPacket = 0;
		//wait(0.25f);
		return outputData;
	}
	void shutDown()
	{ 
		net::ShutdownSockets(); 
	}

	// This function gets an index of data in UDP packet received from x-plane and retrieves the information from the packet
	// The information is in format of 8 float numbers which will be stored in currentDataOutputBuffer.
	// By iterating on that buffer single elements can be read
	void getDataWithIndex(int* dataIndex)
	{
		numberOfDataSeries = calculateDataCount(numberOfBytesInPacket);
		unsigned char* startingIndexInBuffer = startOfDataInPacket(buffer, *dataIndex);
		for (char add_index = 0; add_index < 8; add_index++)
		{
			currentFlaotOutputBuffer[add_index] = convertByteToSingle(startingIndexInBuffer + add_index * 4);
			printf("%f ", currentFlaotOutputBuffer[add_index]);
		}	

	}


	//This function can be called from main function to update the output Buffer. Regarding the size of the packets, amount
	//of data will change in output buffer
	float *fillAndReturnOutputBuffer()
	{
		
		receiveNextData();
		numberOfDataSeries = calculateDataCount(numberOfBytesInPacket);
		
		for (int dataIndex = 0; dataIndex < numberOfDataSeries; dataIndex++)
		{
			unsigned char *startingIndexInBuffer = startOfDataInPacket(buffer, dataIndex);
			
			int currentDataSetIndex = dataIndex * 8;
			
			for (char add_index = 0; add_index < 8; add_index++)
			{
				outputData[add_index + currentDataSetIndex] = convertByteToSingle(startingIndexInBuffer + add_index * 4);
				printf("%f ", outputData[add_index + currentDataSetIndex]);
			}
			printf("\n");
		}

		//for (int dataIndex = 0; dataIndex < numberOfDataSeries; dataIndex++)
		//	printDataFromPacket(startOfDataInPacket(buffer, dataIndex), &sender, &numberOfBytesInPacket, &dataIndex, outputData);
		numberOfBytesInPacket = 0;

		return outputData;

	}

	bool readSocket()
	{
		timeoutCounter = 0;
		// send and receive packets to ourself until the user ctrl-breaks...
		// receive is non-blocking so it might receive no packets, continue command can be used in case of numberOfBytesInPacket == 0 so that the program
		//gets back to while loop again. If size of buffer was smaller than size of message, numberOfBytesInPacket will remain zero but buffer gets full
		while (numberOfBytesInPacket == 0 && timeoutCounter < TIMEOUT_CONST)
		{
			//printf(" Waiting for packet!\n");
			try
			{
				numberOfBytesInPacket = socket.Receive(sender, buffer, sizeof(buffer));

			}
			catch (exception& e) {
				cout << "Exception: " << e.what() << endl;
			}
			timeoutCounter++;

		}

		if (timeoutCounter == TIMEOUT_CONST)
			return false;
		else
			return true;

		
	}

};


