//#ifdef DATARECIEVER_H
//#define DATARECEIVER_H
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
//
//		bool initialize()
//		{
//			if (InitializeSockets())
//			{
//				printf("failed to initialize sockets\n");
//				return 1;
//			}
//
//			// create socket
//			int port = 49001;
//			printf("creating socket on port %d\n", port);
//
//
//
//			if (!socket.Open(port))
//			{
//				printf("failed to create socket!\n");
//				return 1;
//			}
//		}
//
//};
//#endif