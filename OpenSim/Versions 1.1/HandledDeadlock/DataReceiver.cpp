////#include "stdafx.h"
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include "net.h"
//
//using namespace std;
//using namespace net;
//
///*
//float convertByteToSingle(unsigned char* number);
//void printDataFromPacket(unsigned char* startingIndexInBuffer, Address *address, int *numberOfBytesInPacket, int* dataIndex);
//int calculateDataCount(int packetSize);
//unsigned char *startOfDataInPacket(unsigned char *buffer, int dataIndex);
//*/
//
//class DataReceiver
//{
//	private:
//		Socket socket;
//		char c = ' ';
//		int numberOfBytesInPacket = 0;
//		Address sender;
//		unsigned char buffer[1000];
//		unsigned char* headOfBinary = NULL;
//		int numberOfDataSeries = 0;
//
//		float convertByteToSingle(unsigned char* number);
//		int calculateDataCount(int packetSize);
//		void printDataFromPacket(unsigned char* startingIndexInBuffer, Address *address, int *numberOfBytesInPacket, int* dataIndex);
//		unsigned char *startOfDataInPacket(unsigned char *buffer, int dataIndex);
//
//	public:
//		bool initialize();
//		void getNextData();		
//		void shutDown();
//
//};
//
//
//bool DataReceiver::initialize()
//{
//	if (InitializeSockets())
//	{
//		printf("failed to initialize sockets\n");
//		return 1;
//	}
//
//	// create socket
//	int port = 49001;
//	printf("creating socket on port %d\n", port);
//
//
//
//	if (!socket.Open(port))
//	{
//		printf("failed to create socket!\n");
//		return 1;
//	}
//}
//
//void DataReceiver::getNextData()
//{
//	// send and receive packets to ourself until the user ctrl-breaks...
//	// receive is non-blocking so it might receive no packets, continue command can be used in case of numberOfBytesInPacket == 0 so that the program
//	//gets back to while loop again. If size of buffer was smaller than size of message, numberOfBytesInPacket will remain zero but buffer gets full
//	while (numberOfBytesInPacket == 0)
//	{
//		try
//		{
//			numberOfBytesInPacket = socket.Receive(sender, buffer, sizeof(buffer));
//		}
//		catch (exception& e) {
//			cout << "Exception: " << e.what() << endl;
//		}
//	}
//
//	numberOfDataSeries = calculateDataCount(numberOfBytesInPacket);
//
//	for (int dataIndex = 0; dataIndex < numberOfDataSeries; dataIndex++)
//		printDataFromPacket(startOfDataInPacket(buffer, dataIndex), &sender, &numberOfBytesInPacket, &dataIndex);
//
//
//
//	numberOfBytesInPacket = 0;
//	//wait(0.25f);
//
//}
//
//	// shutdown socket layer
//void DataReceiver::shutDown()
//{
//	net::ShutdownSockets();
//}
//
//
//int DataReceiver::calculateDataCount(int packetSize)
//{
//	return (packetSize - 5) / 36;
//}
//
//unsigned char *DataReceiver::startOfDataInPacket(unsigned char *buffer, int dataIndex)
//{
//	int startIndex = 5 + dataIndex * 32 + (dataIndex + 1) * 4;
//	return (unsigned char*)(&buffer[startIndex]);
//}
//
//void DataReceiver::printDataFromPacket(unsigned char* startingIndexInBuffer, Address *senderAddress, int *numberOfBytesInPacket, int* dataIndex)
//{
//	if (*dataIndex == 0)
//		printf("\n\nReceived packet from %d.%d.%d.%d:%d (%d bytes)\n\n\n",
//		senderAddress->GetA(), senderAddress->GetB(), senderAddress->GetC(), senderAddress->GetD(), senderAddress->GetPort(), numberOfBytesInPacket);
//
//
//	printf("Series %d:	  ", *dataIndex + 1);
//	for (int add_index = 0; add_index < 4; add_index++)
//		printf("%f	", convertByteToSingle(startingIndexInBuffer + add_index * 4));
//	printf("                  ");
//	for (int add_index = 4; add_index < 8; add_index++)
//		printf("%f	", convertByteToSingle(startingIndexInBuffer + add_index * 4));
//
//	printf("-------------------------------------------------------------------------------\n");
//}
//
//float DataReceiver::convertByteToSingle(unsigned char* number)
//{
//	union
//	{
//		unsigned char buf[4];
//		float number = 0.0;
//	}single_number;
//
//	for (int i = 0; i < 4; i++)
//	{
//		//printf("Current ASCII Code: %d\n", *(number + i));
//		single_number.buf[i] = *(number + i);
//	}
//	//printf("Number is: %f", single_number.number);
//
//	return single_number.number;
//}
//
//
