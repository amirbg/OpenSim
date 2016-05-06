/****************************************************************************
* Copyright (c) 2014 VT MAK
* All rights reserved.
****************************************************************************/

//! \file talk.cxx
//! \brief Contains the simple talk example.
//! \ingroup examples

#include <vl/exerciseConn.h>
#include <vl/exerciseConnInitializer.h>
#include <vl/topoView.h>
#include <vl/entityPublisher.h>
#include <vl/entityStateRepository.h>
#include <vl/fireInteraction.h>
#include <vl/iffPublisher.h>
#include <vlutil/vlMiniDumper.h>
#include <vl/reflectedEntityListHLA.h>
#include <vl/entityPublisherHLA.h>
#include <vlutil/vlRtiMismatchException.h>

#include <vlpi/entityTypes.h>

#include <vlutil/vlProcessControl.h>

#include <ctime>

#include "ReceiveFromXplane.h"
#include "topoCoord.h"
#include "NamedPipe.h"



using namespace std;
using namespace net;

//Use smaller values for higher frame rates
const int frame_rate_division_variable = 50;
/*
float convertByteToSingle(unsigned char* number);
void printDataFromPacket(unsigned char* startingIndexInBuffer, Address *address, int *numberOfBytesInPacket, int* dataIndex);
int calculateDataCount(int packetSize);
unsigned char *startOfDataInPacket(unsigned char *buffer, int dataIndex);
*/


string doubleToString(double *input);
int calculateFrameRate(clock_t &end_time, clock_t &start_time);

int main(int argc, char* argv[])
{

	clock_t start_time = std::clock();;
	clock_t end_time = std::clock();;
	double duration;

	Socket mySocket;
	Address* myAddress = new Address((unsigned char)127, (unsigned char)0, (unsigned char)0, (unsigned char)1, 49006);

	LPCSTR pipeName = "\\\\.\\Pipe\\MyNamedPipe";
	HANDLE hPipe;
	char *szBuffer = 0;
	char emptyChar = '*';

	float *outputData = NULL;

	bool connection_successful = false;
	bool receivedNewData = false;
	// Used for error handling
	DtINIT_MINIDUMPER("Talk");


	DataReceiver objDataReceiver;
	Socket socket;
	NamedPipe objNP(&hPipe, pipeName);

	try
	{
		//objDataReceiver = new DataReceiver();
		connection_successful = objDataReceiver.initialize();

		//if (!mySocket.Open(49006))
		//{
		//	printf("failed to open socket 49006!");
		//	return 1;
		//}

		if (objNP.pipeReaderCreator())
			return 1;

		// Create a connection to the exercise or federation execution.
		DtVrlApplicationInitializer appInit(argc, argv, "VR-Link Talk");

		DtString fedName = "OpenSim-Send/Receive";
		appInit.setFederateType(fedName);
		appInit.setFederateName(fedName + "1");

		// Process the command line. This will actually set values in the
		// DtVrlApplicationInitializer class and should be called before
		// creating the DtExerciseConn instance.
		appInit.parseCmdLine();

		// The DtExerciseConn instance is perhaps the most important class
		// in any VR-Link exercise. Think of it as the hub which holds
		// everything else together. Technically this instance is what's
		// responsible for connecting this federate to another federate via
		// a network, or an RTI. After exConn is created, and if no error
		// has occurred this simulator will be a live federate.


		DtExerciseConn::InitializationStatus status = DtExerciseConn::DtINIT_SUCCESS;
		DtExerciseConn *exConn = NULL;

		try
		{
			exConn = new DtExerciseConn(appInit, &status);
		}
		catch (DtVlRtiMismatchException)
		{
			std::cerr << "Mismatching RTI compiler Version. Please reconfigure your environment" << std::endl;
			return -1;
		}
		//DtExerciseConn exConn()
		DtReflectedEntityList rel(exConn);

		//Instances of DtEntityStateRepository are used to maintain attributes and 
		//state of entity objects in a protocol independent way.
		DtEntityStateRepository *esr = new DtEntityStateRepository();

		//Instances of DtEntityPublisher are used for local or source simulations of entities
		DtEntityPublisher *ep = new DtEntityPublisher(esr, exConn);



		// Create an object to manage entities that we hear about
		// on the network.
		DtReflectedEntity *first = NULL;



		// Create a topographic view on the state repository, so we
		// can set position information in topographic coordinates.
		DtFloat64 refLatitude = DtDeg2Rad(35.699760);
		DtFloat64 refLongitude = DtDeg2Rad(-121.326577);
		DtFloat64 refAltitude = 0.0;

		DtGeodeticCoord topographicLocation(refLatitude, refLongitude, 1000, 0);
		DtVector geocentricPosition(0, 0, 0);
		DtVector velocity(0, 0, 0);


		DtTopoView topoView(esr, refLatitude, refLongitude);


		//DtVector geocLoc = geodLoc.geocentric();
		// We can use the ESR to set state.
		//      esr->setMarkingText("VR-Link");
		//      topoView.setOrientation(DtTaitBryan(0.0, 0.0, 0.0));

		// Initialize VR-Link time.
		DtClock* clock = exConn->clock();

		DtCoordTransform geocToTopo;
		DtGeocToTopoTransform(DtDeg2Rad(35.699760), DtDeg2Rad(-121.326577), &geocToTopo);

		DtCoordTransform topoToGeoc;
		topoToGeoc.setByInverse(geocToTopo);
		//geocToTopo.coordTrans(esr->location(), geodLoc);

		topoToGeoc.coordTrans(topographicLocation, geocentricPosition);

		for (int i = 0; i < 3; i++)
		{
			printf("%f   \n", geocentricPosition[i]);
		}

		// Main loop
		DtTime dt = 0.05;
		DtTime simTime = 0;
		clock->init();


		while (true)
		{
			exConn->drainInput();
			//for (first = rel->first(); first; first = first->next());
			try
			{
				first = rel.first();
				//if (first != NULL)
				//	first = first->next();

				if (first)
				{
				// Grab its state repository, where we can inspect its data.
				DtEntityStateRepository *esr = first->entityStateRep();

				// Create a topographic view on the state repository, so we
				// can look at position information in topographic coordinates.
				DtFloat64 refLatitude = DtDeg2Rad(35.699760);
				DtFloat64 refLongitude = DtDeg2Rad(-121.326577);
				DtFloat64 refAltitude = 0.0;

				DtTopoView topoView(esr, refLatitude, refLongitude);


				// Print the position.
				// Since it returns a DtString, we need to force it to const char*
				// with a cast.
				//std::cout << "Position of first entity: "
				//	<< topoView.location().string() << std::endl;

				printf("\n");

				szBuffer = objNP.readInput(topoView.location().x(), topoView.location().y(), topoView.location().z());

				if (objNP.writeMessage(szBuffer))
					return 1;

				printf("\nOther Airplanes Location: %s", szBuffer);

				}
			}
			catch (exception e)
			{
				
			}
			
			
			outputData = objDataReceiver.getNextData(&receivedNewData);

			





			if (!receivedNewData)
			{
				objNP.writeMessage(&emptyChar);
				//objNP.writeMessage(szBuffer);
				//printf("\n\n\n\n\nTimeout\n\n\n\n");
				continue;
			}


			//outputData = objDataReceiver.fillAndReturnOutputBuffer();

			refLatitude = *(outputData + 7);
			refLongitude = *(outputData + 8);
			refAltitude = *(outputData + 9);

			//topographicLocation.setLat(refLatitude);
			//topographicLocation.setLon(refLongitude);
			//topographicLocation.setAlt(refAltitude);

			printf("First Output Data");

			for (int i = 0; i < 3; i++)
			{
				printf("%f     ", *(topoView.location() + i));
			}
			cout << endl;

			printf("Latitude: %f, Longitude: %f, Altitude: %f\n", refLatitude, refLongitude, refAltitude);

			DtGeocToTopoTransform(DtDeg2Rad(refLatitude), DtDeg2Rad(refLongitude), &geocToTopo);

			topoToGeoc.coordTrans(topographicLocation, geocentricPosition);

			szBuffer = objNP.readInput(refLatitude, refLongitude, refAltitude);

			geocentricPosition[0] = refLatitude;
			geocentricPosition[1] = refLongitude;
			geocentricPosition[2] = refAltitude;

			//if (objNP.writeMessage(szBuffer))
			//	return 1;

			printf("\nServer sent the following message: %s", szBuffer);

			//position = geodLoc.geocentric();
			//cout << *geodLoc.x << endl;
			//position = geodLoc.geocentric();

			// Tell VR-Link the current value of simulation time.
			clock->setSimTime(simTime);

			// Process any incoming messages.
			exConn->drainInput();

			// Set the current position information.
			topoView.setLocation(geocentricPosition);
			//         topoView.setVelocity(velocity);

			// Call tick, which insures that any data that needs to be
			// updated is sent.
			ep->tick();
			simTime += dt;

			// Wait till real time equals simulation time of next step.
			//DtSleep(simTime - clock->elapsedRealTime());

		}
		net::ShutdownSockets();
		objNP.closeHandle();
	}

	DtCATCH_AND_WARN(std::cout);
	return 0;
}

int calculateFrameRate(clock_t &end_time, clock_t &start_time)
{
	int framerate = 0;

	//cout << endl << endl << "start time is: " << start_time;
	//cout << endl << "end time is: " << end_time << endl;
	//cout << "difference is: " << end_time - start_time << endl;
	//cout << "Ticks per second: " << CLOCKS_PER_SEC << endl;

	//if (start_time != end_time)
	//{
	framerate = CLOCKS_PER_SEC / (double)(end_time - start_time);
	cout << "Frame rate: " << framerate << endl;
	//}
	return framerate;

}
