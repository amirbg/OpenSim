

/*
DrawAircraft example
Written by Sandy Barbour - 11/02/2003

Modified by Sandy Barbour - 07/12/2009
Combined source files and fixed a few bugs.

This examples Draws 7 AI aircraft around the user aicraft.
It also uses an Aircraft class to simplify things.

This is a very simple example intended to show how to use the AI datarefs.
In a production plugin Aircraft Aquisition and Release would have to be handled.
Also loading the approriate aircraft model would also have to be done.
This example may be updated to do that at a later time.

NOTE
Set the aircraft number to 8 in the XPlane Aircraft & Situations settings screen.
*/

#include <string.h>
#include <math.h>
#include "XPLMPlanes.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMGraphics.h"

#include "windows.h"
#include "iostream"




const	double	kMaxPlaneDistance = 5280.0 / 3.2 * 10.0;
const	double	kFullPlaneDist = 5280.0 / 3.2 * 3.0;
#define BUFFER_SIZE 1024 //1k
#define ACK_MESG_RECV "Message received successfully"
// Aircraft class, allows access to an AI aircraft

class Aircraft
{
private:
	XPLMDataRef	dr_plane_x;
	XPLMDataRef	dr_plane_y;
	XPLMDataRef	dr_plane_z;
	XPLMDataRef	dr_plane_the;
	XPLMDataRef	dr_plane_phi;
	XPLMDataRef	dr_plane_psi;
	XPLMDataRef	dr_plane_gear_deploy;
	XPLMDataRef	dr_plane_throttle;
public:
	float		plane_x;
	float		plane_y;
	float		plane_z;
	float		plane_the;
	float		plane_phi;
	float		plane_psi;
	float		plane_gear_deploy[5];
	float		plane_throttle[8];
	Aircraft(int AircraftNo);
	void GetAircraftData(void);
	void SetAircraftData(void);
};

Aircraft::Aircraft(int AircraftNo)
{
	char	x_str[80];
	char	y_str[80];
	char	z_str[80];
	char	the_str[80];
	char	phi_str[80];
	char	psi_str[80];
	char	gear_deploy_str[80];
	char	throttle_str[80];

	strcpy(x_str, "sim/multiplayer/position/planeX_x");
	strcpy(y_str, "sim/multiplayer/position/planeX_y");
	strcpy(z_str, "sim/multiplayer/position/planeX_z");
	strcpy(the_str, "sim/multiplayer/position/planeX_the");
	strcpy(phi_str, "sim/multiplayer/position/planeX_phi");
	strcpy(psi_str, "sim/multiplayer/position/planeX_psi");
	strcpy(gear_deploy_str, "sim/multiplayer/position/planeX_gear_deploy");
	strcpy(throttle_str, "sim/multiplayer/position/planeX_throttle");

	char cTemp = (AircraftNo + 0x30);
	x_str[30] = cTemp;
	y_str[30] = cTemp;
	z_str[30] = cTemp;
	the_str[30] = cTemp;
	phi_str[30] = cTemp;
	psi_str[30] = cTemp;
	gear_deploy_str[30] = cTemp;
	throttle_str[30] = cTemp;

	dr_plane_x = XPLMFindDataRef(x_str);
	dr_plane_y = XPLMFindDataRef(y_str);
	dr_plane_z = XPLMFindDataRef(z_str);
	dr_plane_the = XPLMFindDataRef(the_str);
	dr_plane_phi = XPLMFindDataRef(phi_str);
	dr_plane_psi = XPLMFindDataRef(psi_str);
	dr_plane_gear_deploy = XPLMFindDataRef(gear_deploy_str);
	dr_plane_throttle = XPLMFindDataRef(throttle_str);
}

void Aircraft::GetAircraftData(void)
{
	plane_x = XPLMGetDataf(dr_plane_x);
	plane_y = XPLMGetDataf(dr_plane_y);
	plane_z = XPLMGetDataf(dr_plane_z);
	plane_the = XPLMGetDataf(dr_plane_the);
	plane_phi = XPLMGetDataf(dr_plane_phi);
	plane_psi = XPLMGetDataf(dr_plane_psi);
	XPLMGetDatavf(dr_plane_gear_deploy, plane_gear_deploy, 0, 5);
	XPLMGetDatavf(dr_plane_throttle, plane_throttle, 0, 8);
}

void Aircraft::SetAircraftData(void)
{
	XPLMSetDataf(dr_plane_x, plane_x);
	XPLMSetDataf(dr_plane_y, plane_y);
	XPLMSetDataf(dr_plane_z, plane_z);
	XPLMSetDataf(dr_plane_the, plane_the);
	XPLMSetDataf(dr_plane_phi, plane_phi);
	XPLMSetDataf(dr_plane_psi, plane_psi);
	XPLMSetDatavf(dr_plane_gear_deploy, plane_gear_deploy, 0, 5);
	XPLMSetDatavf(dr_plane_throttle, plane_throttle, 0, 8);
}



static LPCSTR pipeName = "\\\\.\\Pipe\\MyNamedPipe";
static 	HANDLE hPipe;
static char *szBuffer;
static char nullchar = 'N';


static double latit = 0;
static double longit = 0;
static double altit = 0;

class NamedPipe
{
public:
	NamedPipe(HANDLE *arghPipe, LPCSTR argpipeName);
	bool pipeCreator();
	bool pipeReaderCreator();
	bool waitForClient();
	char *readMessage();
	bool writeMessage(char *szBuffer);
	char *readInput(double *inpt);
	void closeHandle();
	void retrieveData(char *szBuffer, double *lat, double *Lon, double *Alt);

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

		return 1;
	}
	else
		return 0;


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

		//One might want to check whether the server pipe is busy
		//This sample will error out if the server pipe is busy
		//Read on ERROR_PIPE_BUSY and WaitNamedPipe() for that
		return 1;  //Error
	}
	else
	{

		return 0;
	}
}

bool NamedPipe::waitForClient()
{
	BOOL bClientConnected = ConnectNamedPipe(hPipe, NULL);

	if (FALSE == bClientConnected)
	{

		CloseHandle(hPipe);
		return 1;  //Error
	}
	else
	{

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
		return &nullchar;
		//CloseHandle(hPipe);
		//return 0x0;  //Error
	}
	else
	{

		return classBuffer;
	}
}

bool NamedPipe::writeMessage(char *szBuffer)
{
	BOOL bResult = WriteFile(
		hPipe,                // handle to pipe 
		szBuffer,             // buffer to write from 
		strlen(classBuffer) + 1,   // number of bytes to write, include the NULL 
		&cbBytes,             // number of bytes written 
		NULL);                // not overlapped I/O 

	if ((!bResult) || (strlen(classBuffer) + 1 != cbBytes))
	{

		CloseHandle(hPipe);
		return 1;  //Error
	}
	else
	{
		szBuffer = classBuffer;
		return 0;
	}

}

char *NamedPipe::readInput(double *inpt)
{
	int length = 0;
	for (int i = 0; i < 5; i++)
		classBuffer[i] = *inpt / 10;
	/*std::string str = std::to_string(*inpt);
	length = str.length();
	for (int i = 0; i < length; i++)
	classBuffer[i] = str[i];*/
	classBuffer[5] = NULL;
	return classBuffer;
}

void NamedPipe::retrieveData(char *szBuffer, double *Lat, double *Lon, double *Alt)
{
	int counter = 0;
	char item = 0;
	bool negative;
	double decimal;
	char decimalCounter;
	double localLat = 0;
	double localLon = 0;
	double localAlt = 0;

	while (szBuffer[counter] != NULL)
	{

		negative = false;
		decimalCounter = 1;
		decimal = 0;

		switch (item)
		{
		case(0) :
			if (szBuffer[counter] == '-')
			{
				counter++;
				negative = true;
			}

				while (szBuffer[counter] != '.')
				{
					localLat = 10 * localLat + (szBuffer[counter] - 48);
					counter++;
				}

				while (szBuffer[++counter] != 'N')
					decimal = decimal + pow(0.1, decimalCounter++) * (szBuffer[counter] - 48);

				localLat += decimal;

				if (negative)
					localLat = localLat * -1;

				counter++;

				item++;
				break;

		case(1) :
			if (szBuffer[counter] == '-')
			{
				counter++;
				negative = true;
			}

				while (szBuffer[counter] != '.')
				{
					localLon = 10 * localLon + (szBuffer[counter] - 48);
					counter++;
				}

				while (szBuffer[++counter] != 'N')
					decimal = decimal + pow(0.1, decimalCounter++) * (szBuffer[counter] - 48);

				localLon += decimal;

				if (negative)
					localLon = localLon * -1;

				counter++;

				item++;
				break;

		case(2) :
			if (szBuffer[counter] == '-')
			{
				counter++;
				negative = true;
			}

				while (szBuffer[counter] != '.')
				{
					localAlt = 10 * localAlt + (szBuffer[counter] - 48);
					counter++;
				}


				while (szBuffer[++counter] != 'N' || szBuffer[counter] != NULL)
				{
					if (szBuffer[counter] == NULL)	break;
					decimal = decimal + pow(0.1, decimalCounter++) * (szBuffer[counter] - 48);

				}


				localAlt += decimal;

				if (negative)
					localAlt = localAlt * -1;

				item++;
				break;

		default:
			break;
		}
	}

	*Lat = localLat;
	*Lon = localLon;
	*Alt = localAlt;
}


void NamedPipe::closeHandle()
{
	CloseHandle(hPipe);
}








// Datarefs for the User Aircraft

static XPLMDataRef		gPlaneX = NULL;
static XPLMDataRef		gPlaneY = NULL;
static XPLMDataRef		gPlaneZ = NULL;
static XPLMDataRef		gPlaneTheta = NULL;
static XPLMDataRef		gPlanePhi = NULL;
static XPLMDataRef		gPlanePsi = NULL;
static XPLMDataRef		gOverRidePlanePosition = NULL;
static XPLMDataRef		gAGL = NULL;


// Create 7 instances of the Aircraft class.

Aircraft Aircraft1(1);


NamedPipe objNP(&hPipe, pipeName);

// Used to disable AI so we have control.

static float	MyFlightLoopCallback0(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon);

// Used to update each aircraft every frame.

static float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon);


PLUGIN_API int XPluginStart(char *		outName,
	char *		outSig,
	char *		outDesc)
{
	strcpy(outName, "Draw");
	strcpy(outSig, "xplanesdk.examples.drawaircraft");
	strcpy(outDesc, "A plugin that draws aircraft.");

	/* Prefetch the sim variables we will use. */
	gPlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
	gPlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
	gPlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
	gPlaneTheta = XPLMFindDataRef("sim/flightmodel/position/theta");
	gPlanePhi = XPLMFindDataRef("sim/flightmodel/position/phi");
	gPlanePsi = XPLMFindDataRef("sim/flightmodel/position/psi");
	gOverRidePlanePosition = XPLMFindDataRef("sim/operation/override/override_planepath");
	gAGL = XPLMFindDataRef("sim/flightmodel/position/y_agl");

	if (objNP.pipeCreator())
		return 1;



	XPLMRegisterFlightLoopCallback(
		MyFlightLoopCallback0,	/* Callback */
		1.0,					/* Interval */
		NULL);					/* refcon not used. */

	XPLMRegisterFlightLoopCallback(
		MyFlightLoopCallback,	/* Callback */
		1.0,					/* Interval */
		NULL);					/* refcon not used. */

	return 1;
}


PLUGIN_API void	XPluginStop(void)
{
	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback0, NULL);
	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
	objNP.closeHandle();
}

PLUGIN_API void XPluginDisable(void)
{


}

PLUGIN_API int XPluginEnable(void)
{
	objNP.waitForClient();
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID	inFromWho,
	int				inMessage,
	void *			inParam)
{
}

float	MyFlightLoopCallback0(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon)
{
	int AircraftIndex;

	// Disable AI for each aircraft.
	for (AircraftIndex = 1; AircraftIndex<8; AircraftIndex++)
		XPLMDisableAIForPlane(AircraftIndex);

	return 0;
}


float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon)
{
	int	GearState;

	double	x, y, z, theta, phi, psi;

	//double Lat = 0, Lon = 0, Alt = 0;
	float Heading = 0, Pitch = 0, Roll = 0, Altitude;

	szBuffer = objNP.readMessage();
	//objNP.writeMessage(szBuffer);

	if (szBuffer[0] != '*')
		objNP.retrieveData(szBuffer, &latit, &longit, &altit);

	// Get User Aircraft data
	//XPLMWorldToLocal(latit, longit, altit, &x, &y, &z);

	//x = XPLMGetDataf(gPlaneX);
	//y = XPLMGetDataf(gPlaneY);
	//z = XPLMGetDataf(gPlaneZ);
	theta = XPLMGetDataf(gPlaneTheta);
	phi = XPLMGetDataf(gPlanePhi);
	psi = XPLMGetDataf(gPlanePsi);
	Altitude = XPLMGetDataf(gAGL);

	// Copy it to each aircraft using different offsets for each aircraft.

	Aircraft1.plane_x = latit + 15.0;
	Aircraft1.plane_y = longit;
	Aircraft1.plane_z = altit + 15.0;
	Aircraft1.plane_the = theta;
	Aircraft1.plane_phi = phi;
	Aircraft1.plane_psi = psi;


	// Raise the gear when above 200 feet.
	if (Altitude > 200)
		GearState = 0;
	else
		GearState = 1;

	/// Changed from 5 to 6 - Sandy Barbour 18/01/2005
	/// This will be changed to handle versions when the
	/// increase to 10 is implemented in the glue.

	Aircraft1.plane_gear_deploy[0] = GearState;


	// Now set the data in each instance.
	Aircraft1.SetAircraftData();

	return -1;
}
