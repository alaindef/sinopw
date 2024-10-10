#define WIN32_LEAN_AND_MEAN

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPLMNavigation.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <vector>
#include <list>
#include <math.h>
#include <string>
#include <thread>
#include <mutex>

#ifdef IBM
#pragma comment(lib, "Ws2_32.lib")
#endif

using namespace std;

#define PI 3.1415926
#define DEG2RAD(d) (d / 57.2957795)
#define RAD2DEG(r) (r * 57.2957795)
#define FEET2METERS(f) (f * 0.305)
#define METERS2FEET(m) (m / 0.305)
#define _snprintf_s(a,b,c,...) snprintf(a,b,__VA_ARGS__)

double DiffTrack(double track1, double track2)
{
    double diff = track2 - track1;
    if (diff >= 180)
        diff -= 360;
    else if (diff <= -180)
        diff += 360;
    return diff;
}

uint64_t GetTimerMs()
{
    uint64_t ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

// define name
#define NAME "PIDPLUGIN"
#define NAME_LOWERCASE "pidplugin"

////////////////X-Plane Data///////////////////////////

////////////////////////////////////////
#define UDPDATAPORT_PIDPLUGIN 9225
#define UDPCOMMANDPORT_PIDPLUGIN 9226
struct DataFromXPlane
{
    char header[8];
    float bankangle;
    float heading;                  //''''''''''''''''''''''''''
    float indicatedairspeed;
    float trueairspeed;
    float verticalspeed;
    float pitchangle;
    float pitchrate;
    float rollrate;
    float yawrate;
    float truetrack;
    float elevatortrimpos;
    float elevatorpos;

    double latitude;
    double longitude;
    float altitude;
    float groundspeed;
    float truewinddir;
    float windspeed;
    float magvar;

    double gforce;
};
struct DataToXPlane
{
    char header[12];
    union {
        float fValue;
        bool bValue;
        int iValue;
    } unionValue;
};
DataFromXPlane fsData;

XPLMDataRef DR_BankAngle;
XPLMDataRef DR_Heading;                     //''''''''''''''''''''''''''''''''
XPLMDataRef DR_indicatedairspeed;
XPLMDataRef DR_trueairspeed;
XPLMDataRef DR_verticalspeed;
XPLMDataRef DR_pitchangle;
XPLMDataRef DR_pitchrate;
XPLMDataRef DR_rollrate;
XPLMDataRef DR_yawrate;
XPLMDataRef DR_truetrack;
XPLMDataRef DR_elevatortab;

XPLMDataRef DR_AileronPosition;
XPLMDataRef DR_OverrideRollControl;
XPLMDataRef DR_ElevatorPosition;
XPLMDataRef DR_OverridePitchControl;
XPLMDataRef DR_RudderPosition;
XPLMDataRef DR_OverrideYawControl;

XPLMDataRef DR_latitide;
XPLMDataRef DR_longitude;
XPLMDataRef DR_altitude;
XPLMDataRef DR_groundspeed;
XPLMDataRef DR_truewinddir;
XPLMDataRef DR_windspeed;
XPLMDataRef DR_magvar;
XPLMDataRef DR_gforce;

XPLMCommandRef CR_Pause_TOGGLE;
XPLMCommandRef CR_TrimUp;
XPLMCommandRef CR_TrimDown;
bool gIsTrimmingUp = false, gTrimUpRequested = false;
bool gIsTrimmingDown = false, gTrimDownRequested = false;

//Callback functions called by X-plane itself or on request by the plugin
float MyCallbackFunction(float, float, int, void*);

//Socket Data
int SendingSock;
struct sockaddr_in Broadcast_addr;
int ListeningSock;
struct sockaddr_in Any_addr;

//Thread Data
std::thread ListeningThreadID;
void* ListeningThread(void* pvoid);
bool gListeningThreadEnabled = false;
std::mutex mutexCallback;

//error handling
void MessageBox(void*, char* msg, char* title, int){ XPLMDebugString(msg); }
/////////////////////////////////////////////////////////////////////////////////////////////

PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{
    // Plugin Info
    strcpy(outName, "PIDPLUGIN");
    strcpy(outSig, "XPRT.PID.PLUGIN");
    strcpy(outDesc, "This plugin is required to provide data to the external PID Controller.");

    XPLMDebugString("starting plugin: PID-Plugin\n");

    //Attach X-Plane data to the datarefs declared above

    //Real Time Datarefs
    strcpy(fsData.header, "PID_DTA");

    DR_BankAngle = XPLMFindDataRef("sim/cockpit/gyros/phi_ind_deg3");
    if (!DR_BankAngle) { MessageBox(NULL, "Dataref DR_BankAngle Not Found", "PID Plugin error", 0);	return 0; }
    DR_Heading = XPLMFindDataRef("sim/cockpit/gyros/psi_ind_degm3");
    if (!DR_Heading) { MessageBox(NULL, "Dataref DR_BankAngle Not Found", "PID Plugin error", 0);	return 0; }
    DR_indicatedairspeed = XPLMFindDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot");
    if (!DR_indicatedairspeed) { MessageBox(NULL, "Dataref DR_indicatedairspeed Not Found", "PID Plugin error", 0);	return 0; }
    DR_trueairspeed = XPLMFindDataRef("sim/flightmodel/position/true_airspeed");
    if (!DR_trueairspeed) { MessageBox(NULL, "Dataref DR_trueairspeed Not Found", "PID Plugin error", 0);	return 0; }
    DR_verticalspeed = XPLMFindDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot");
    if (!DR_verticalspeed) { MessageBox(NULL, "Dataref DR_verticalspeed Not Found", "PID Plugin error", 0);	return 0; }
    DR_pitchangle = XPLMFindDataRef("sim/cockpit/gyros/the_ind_deg3");
    if (!DR_pitchangle) { MessageBox(NULL, "Dataref DR_pitchangle Not Found", "PID Plugin error", 0);	return 0; }
    DR_pitchrate = XPLMFindDataRef("sim/flightmodel/position/Q");
    if (!DR_pitchrate) { MessageBox(NULL, "Dataref DR_pitchrate Not Found", "PID Plugin error", 0);	return 0; }
    DR_rollrate = XPLMFindDataRef("sim/flightmodel/position/P");
    if (!DR_rollrate)	{MessageBox(NULL, "Dataref DR_rollrate Not Found", "PID Plugin error",0);	return 0;}
    DR_yawrate = XPLMFindDataRef("sim/flightmodel/position/R");
    if (!DR_yawrate) { MessageBox(NULL, "Dataref DR_yawrate Not Found", "PID Plugin error", 0);	return 0; }
    DR_truetrack = XPLMFindDataRef("sim/flightmodel/position/hpath");
    if (!DR_truetrack) { MessageBox(NULL, "Dataref DR_truetrack Not Found", "PID Plugin error", 0);	return 0; }
    DR_elevatortab = XPLMFindDataRef("sim/flightmodel/controls/elv_trim");
    if (!DR_elevatortab) { MessageBox(NULL, "Dataref DR_elevatortab Not Found", "PID Plugin error", 0);	return 0; }

    DR_AileronPosition = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");
    if (!DR_AileronPosition) { MessageBox(NULL, "Dataref DR_AileronPosition Not Found", "PID Plugin error", 0);	return 0; }
    DR_OverrideRollControl = XPLMFindDataRef("sim/operation/override/override_joystick_roll");
    if (!DR_OverrideRollControl) { MessageBox(NULL, "Dataref DR_OverrideRollControl Not Found", "PID Plugin error", 0);	return 0; }
    DR_ElevatorPosition = XPLMFindDataRef("sim/joystick/yoke_pitch_ratio");
    if (!DR_ElevatorPosition) { MessageBox(NULL, "Dataref DR_ElevatorPosition Not Found", "PID Plugin error", 0);	return 0; }
    DR_OverridePitchControl = XPLMFindDataRef("sim/operation/override/override_joystick_pitch");
    if (!DR_OverridePitchControl) { MessageBox(NULL, "Dataref DR_OverridePitchControl Not Found", "PID Plugin error", 0);	return 0; }
    DR_RudderPosition = XPLMFindDataRef("sim/joystick/yoke_heading_ratio");
    if (!DR_RudderPosition) { MessageBox(NULL, "Dataref DR_RudderPosition Not Found", "PID Plugin error", 0);	return 0; }
    DR_OverrideYawControl = XPLMFindDataRef("sim/operation/override/override_joystick_heading");
    if (!DR_OverrideYawControl) { MessageBox(NULL, "Dataref DR_OverrideYawControl Not Found", "PID Plugin error", 0);	return 0; }

    DR_latitide = XPLMFindDataRef("sim/flightmodel/position/latitude");
    if (!DR_latitide) { MessageBox(NULL, "Dataref DR_latitide Not Found", "PID Plugin error", 0);	return 0; }
    DR_longitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
    if (!DR_longitude) { MessageBox(NULL, "Dataref DR_longitude Not Found", "PID Plugin error", 0);	return 0; }
    DR_altitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
    if (!DR_altitude) { MessageBox(NULL, "Dataref DR_altitude Not Found", "PID Plugin error", 0);	return 0; }
    DR_groundspeed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
    if (!DR_groundspeed) { MessageBox(NULL, "Dataref DR_groundspeed Not Found", "PID Plugin error", 0);	return 0; }
    DR_truewinddir = XPLMFindDataRef("sim/weather/wind_direction_degt");
    if (!DR_truewinddir) { MessageBox(NULL, "Dataref DR_truewinddir Not Found", "PID Plugin error", 0);	return 0; }
    DR_windspeed = XPLMFindDataRef("sim/weather/wind_speed_kt");
    if (!DR_windspeed) { MessageBox(NULL, "Dataref DR_windspeed Not Found", "PID Plugin error", 0);	return 0; }
    DR_magvar = XPLMFindDataRef("sim/flightmodel/position/magnetic_variation");
    if (!DR_magvar) { MessageBox(NULL, "Dataref DR_magvar Not Found", "PID Plugin error", 0);	return 0; }
    /*DR_gforce = XPLMFindDataRef("sim/flightmodel/forces/g_nrm");
    if (!DR_gforce) { MessageBox(NULL, "Dataref DR_gforce Not Found", "PID Plugin error", 0);	return 0; }*/


    CR_TrimUp = XPLMFindCommand("sim/flight_controls/pitch_trim_up");
    if (!CR_TrimUp) { MessageBox(NULL, "CommandRef CR_TrimUp Not Found", "PID Plugin error", 0);	return 0; }
    CR_TrimDown = XPLMFindCommand("sim/flight_controls/pitch_trim_down");
    if (!CR_TrimDown) { MessageBox(NULL, "CommandRef CR_TrimDown Not Found", "PID Plugin error", 0);	return 0; }

	//Setup socket to send UDP packets
    //WSAStartup(MAKEWORD(2,2), &wsaData);

	//Setup Sending socket so that it sends to the broadcast address
	SendingSock = socket(AF_INET,SOCK_DGRAM,0);
    if (SendingSock < 0)
    {
        MessageBox(NULL,"Error in opening Sending-Socket options", "error", 0);
        return 0;
    }
#ifdef _WIN32
    char broadcast = '1';
    char reuseaddr = '1';
#else
    int broadcast = 1;
    int reuseaddr = 1;
#endif
    if(setsockopt(SendingSock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast)) < 0 ||
	   setsockopt(SendingSock,SOL_SOCKET, SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr)) < 0)
    {
        MessageBox(NULL,"Error in setting Sending-Socket options", "error", 0);
#ifdef _WIN32
        closesocket(SendingSock);
#else
        close(SendingSock);
#endif
        return 0;
    }
    u_long iMode=1;
#ifdef _WIN32
    ioctlsocket(SendingSock, FIONBIO, &iMode);
#else
    ioctl(SendingSock, FIONBIO, &iMode);
#endif
	//struct sockaddr_in Broadcast_addr;  
    Broadcast_addr.sin_family       = AF_INET;         
	Broadcast_addr.sin_port         = htons(UDPDATAPORT_PIDPLUGIN);    
    Broadcast_addr.sin_addr.s_addr  = htonl(INADDR_BROADCAST); 

	//Setup Listening Socket so that it listens on any address
	ListeningSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int BufferSize = 65535;
	if(setsockopt(ListeningSock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast)) < 0 ||
	   setsockopt(ListeningSock,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr)) < 0 )
    {
        MessageBox(NULL,"Error in setting Listening-Socket options", "error", 0);
#ifdef _WIN32
        closesocket(SendingSock);
#else
        close(SendingSock);
#endif
        return 0;
    }

	//struct sockaddr_in Broadcast_addr;  
    Any_addr.sin_family       = AF_INET;         
	Any_addr.sin_port         = htons(UDPCOMMANDPORT_PIDPLUGIN);
    Any_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
	
	if (bind(ListeningSock,(sockaddr*)&Any_addr, sizeof (Any_addr)) < 0)
    {
        MessageBox(NULL,"Error in BINDING", "error",0);
#ifdef _WIN32
        closesocket(SendingSock);
#else
        close(SendingSock);
#endif
        return 0;
    }

    broadcast = 1;
    reuseaddr = 1;

	//Register Callback function that Xplane calls to update the fsdatastructures
	XPLMRegisterFlightLoopCallback(MyCallbackFunction, -1.0, NULL); //-1.0 means every framerate

	//Create a thread that listens for messages from clients
    gListeningThreadEnabled = true;
    void* pvoid = nullptr;
    ListeningThreadID = std::thread(ListeningThread, pvoid);

	return 1;
}

//Thread that listens for messages from the clients
void* ListeningThread(void* pvoid)
{	
	//create the buffer to store the messages from the clients
	char recvbuff[512];
	int recvbufflen = 512;

	struct sockaddr_in Client_addr; //stores the address of the client
	int len = sizeof(struct sockaddr_in);

	//Continue receiving messages until the XPluginStop function is called (Which sets gListeningThreadEnabled to false)
	while (gListeningThreadEnabled)
	{
		//Wait until message is received, and store it in recvbuff
        recvfrom(ListeningSock, recvbuff, recvbufflen, 0, (sockaddr*)&Client_addr, (socklen_t*)&len);
		
        static bool controlOverrideRoll = false;
        static bool controlOverridePitch = false;
        if (strncmp(recvbuff, "PID_BNK_VA", 11) == 0)
		{			
            if (controlOverrideRoll)
            {
                mutexCallback.lock();
                DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
                XPLMSetDataf(DR_AileronPosition, dataToXPlanePointer->unionValue.fValue);
                mutexCallback.unlock();
            }
		}
        else if (strncmp(recvbuff, "PID_PCH_VA", 11) == 0)
        {
            
            if (controlOverridePitch)
            {
                mutexCallback.lock();
                DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
                XPLMSetDataf(DR_ElevatorPosition, dataToXPlanePointer->unionValue.fValue);
                mutexCallback.unlock();
            }
        }
        if (strncmp(recvbuff, "PID_RUD_VA", 11) == 0)
        {
            if (controlOverrideRoll)
            {
                mutexCallback.lock();
                DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
                XPLMSetDataf(DR_RudderPosition, dataToXPlanePointer->unionValue.fValue);
                mutexCallback.unlock();
            }
        }
        else if (strncmp(recvbuff, "PID_BNK_OVR", 11) == 0)
        {            
            DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
            controlOverrideRoll = dataToXPlanePointer->unionValue.bValue;
            mutexCallback.lock();
            XPLMSetDatai(DR_OverrideRollControl, dataToXPlanePointer->unionValue.bValue ? 1 : 0);
            XPLMSetDatai(DR_OverrideYawControl, dataToXPlanePointer->unionValue.bValue ? 1 : 0);
            mutexCallback.unlock();
        }
        else if (strncmp(recvbuff, "PID_PCH_OVR", 11) == 0)
        {
            DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
            controlOverridePitch = dataToXPlanePointer->unionValue.bValue;
            mutexCallback.lock();
            XPLMSetDatai(DR_OverridePitchControl, dataToXPlanePointer->unionValue.bValue ? 1 : 0);
            mutexCallback.unlock();
        }
        else if (strncmp(recvbuff, "PID_TRIM_UP", 11) == 0)
        {
            DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
            mutexCallback.lock();
            gTrimUpRequested = dataToXPlanePointer->unionValue.bValue;
            mutexCallback.unlock();
        }
        else if (strncmp(recvbuff, "PID_TRIM_DN", 11) == 0)
        {
            DataToXPlane* dataToXPlanePointer = (DataToXPlane*)&recvbuff;
            mutexCallback.lock();
            gTrimDownRequested = dataToXPlanePointer->unionValue.bValue;
            mutexCallback.unlock();
        }
    }
    return 0;
}

PLUGIN_API void    XPluginStop(void)
{
	//Cleanup Callbackfunctions
	XPLMUnregisterFlightLoopCallback(MyCallbackFunction, NULL);

    //Cleanup Socket
#ifdef _WIN32
    closesocket(SendingSock);
    closesocket(ListeningSock);
#else
    close(SendingSock);
    close(ListeningSock);
#endif

	//Let the listening thread end, then wait for it to end
	gListeningThreadEnabled = false;
    ListeningThreadID.join();
	
    //WSACleanup();
}


//This function is called every framerate of X-Plane
float MyCallbackFunction(float, float, int, void*)
{	
    static uint64_t lastUpdateMS = 0;
    uint64_t currTimeMS = GetTimerMs();
    uint64_t elapsedMS = 0;
    if (lastUpdateMS > 0)
        elapsedMS = currTimeMS - lastUpdateMS;

    static float lastHeading = 0.0;

    mutexCallback.lock();

	//Fill the structure with data from the datarefs
	fsData.bankangle = XPLMGetDataf(DR_BankAngle);
    fsData.heading = XPLMGetDataf(DR_Heading);
    fsData.indicatedairspeed = XPLMGetDataf(DR_indicatedairspeed);
    fsData.trueairspeed = (XPLMGetDataf(DR_trueairspeed) * 1.9438444924406); // MPS2KTS;
    fsData.verticalspeed = XPLMGetDataf(DR_verticalspeed);
    fsData.pitchangle = XPLMGetDataf(DR_pitchangle);
    fsData.pitchrate = XPLMGetDataf(DR_pitchrate);
    fsData.rollrate = XPLMGetDataf(DR_rollrate);
    fsData.yawrate = XPLMGetDataf(DR_yawrate);
    //fsData.yawrate = ((DiffTrack(lastHeading, fsData.heading)) / (float)elapsedMS) / 1000.f;
    fsData.truetrack = XPLMGetDataf(DR_truetrack);
    fsData.elevatortrimpos = XPLMGetDataf(DR_elevatortab);
    fsData.elevatorpos = XPLMGetDataf(DR_ElevatorPosition);

    fsData.latitude = XPLMGetDatad(DR_latitide);
    fsData.longitude = XPLMGetDatad(DR_longitude);
    fsData.altitude = XPLMGetDataf(DR_altitude) * 3.28084; // M2FT
    fsData.groundspeed = (XPLMGetDataf(DR_groundspeed) * 1.9438444924406); // MPS2KTS;
    fsData.truewinddir = XPLMGetDataf(DR_truewinddir);
    fsData.windspeed = XPLMGetDataf(DR_windspeed);
    fsData.magvar = -XPLMGetDataf(DR_magvar);
    fsData.gforce = 0; //XPLMGetDataf(DR_gforce);

	//Send the structure to the clients	
    int res = sendto(SendingSock, (char*)&fsData, sizeof(fsData)+1, 0,( sockaddr*)&Broadcast_addr ,sizeof(Broadcast_addr));
    if (res < 0)
	{
        std::string message = "Error in Sending:";
        message += res;
        MessageBox(NULL, (char*)message.c_str(), "Error in Sending", 0);//<<WSAGetLastError();
#ifdef _WIN32
        closesocket(SendingSock);
#else
        close(SendingSock);
#endif
        return 0;
	}

    mutexCallback.unlock();

    lastHeading = fsData.heading;
    lastUpdateMS = currTimeMS;

    if (gTrimUpRequested && !gIsTrimmingUp)
    {
        XPLMCommandBegin(CR_TrimUp);
        gIsTrimmingUp = true;
        gTrimDownRequested = false;
    }
    else if (!gTrimUpRequested && gIsTrimmingUp)
    {
        XPLMCommandEnd(CR_TrimUp);
        gIsTrimmingUp = false;
    }
    if (gTrimDownRequested && !gIsTrimmingDown)
    {
        XPLMCommandBegin(CR_TrimDown);
        gIsTrimmingDown = true;
        gTrimUpRequested = false;
    }
    else if (!gTrimDownRequested && gIsTrimmingDown)
    {
        XPLMCommandEnd(CR_TrimDown);
        gIsTrimmingDown = false;
    }
	
	return -1.0; //let X-PLane know this function should again be called next frame
}

PLUGIN_API void XPluginDisable(void)
{

}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
			 XPLMPluginID    inFromWho,
			 long            inMessage,
			 void *          inParam)
{

}
