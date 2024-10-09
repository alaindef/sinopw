// FSData.h: interface for the CFSData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FSDATA_H__36FF2E9D_D4C2_49F6_B01E_1BFF69C6053E__INCLUDED_)
#define AFX_FSDATA_H__36FF2E9D_D4C2_49F6_B01E_1BFF69C6053E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <stdlib.h>

//#include <netinet/in.h>
//#include <sys/ioctl.h>
//#include <unistd.h>
#include <sys/types.h>
//#include <unistd.h>

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

void* ListeningThread(void* pvoid);

class CFSData
{
public:
	CFSData();
	virtual ~CFSData();
	bool CreateSocket();
	void CloseSocket();

	void InitData();
	void ReadFromFS();
	void SetAileron(float aileronPos);
	void SetAileronOverride(bool isOverride);
	void SetElevator(float elevatorPos);
	void SetRuddder(float rudderPos);
	void SetElevatorOverride(bool isOverride);
	void SetElevatorTrimUp(bool trimActive);
	void SetElevatorTrimDown(bool trimActive);
	bool IsConnectedToFS() { return m_SocketConnected; }

	float GetBankAngle();
	float GetHeading();                   //''''''''''''''''''''''''''''''
	float GetIndicatedairspeed();
	float GetTrueairspeed();
	float GetVerticalspeed();
	float GetPitchangle();
	float GetPitchrate();
	float GetRollrate();
	float GetYawrate();
	float GetTruetrack();
	float GetElevatorPosition();
	float GetElevatorTrimPosition();

	double GetLatitude();
	double GetLongitude();
	float GetAltitude();
	float GetGroundspeed();
	float GetTrueWinddirection();
	float GetWindspeed();
	float GetMagneticvatiation();
	float GetVerticalAccelerationG();

	/* YokeforcePitch dient om het vliegtuig te trimmen, zodat er geen kracht
	*  zit op de yoke: dus YokeForcePitch = 0
	*  X-plane heeft geen yokeforce dataref, de yokeforce van joysticks (zonder 
	*  forcefeedback) is 0 als hij int midden staat, dus wanneer elevatorposition = 0,
	*  vandaar deze tijdelijke implementatie van GetYokeForcePitch().
	* De C-130 sim zal wel de yokeforce moeten geven aan de autopiloot.
	*/
	float GetYokeForcePitch() { return (GetElevatorPosition()) / 2.0f; } 
	//

private:
	//Socket data	
    int SendingSock;
    int ListeningSock;
	struct sockaddr_in Broadcast_addr;   
    struct sockaddr_in Any_addr; 
    socklen_t m_len_sockaddr_in;

    DataFromXPlane* m_datafromxplane;
	
	char m_buffer[512];
	bool m_SocketConnected;

	float m_BankAngle;
    float m_Heading;                               //------------------
	float m_indicatedairspeed;
	float m_trueairspeed;
	float m_verticalspeed;
	float m_pitchangle;
	float m_pitchrate;
	float m_rollrate;
	float m_yawrate;
	float m_truetrack;
	float m_elevatorposition;
	float m_elevatortrimposition;

	double m_latitude;
	double m_longitude;
	float m_altitude;
	float m_groundspeed;
	float m_truewinddir;
	float m_windspeed;
	float m_magvar;

	double m_gforce;


	// Structures To Set FSData
	DataToXPlane m_SetDataControlPosition;
	int m_SetDataLength;
};
#endif // !defined(AFX_FSDATA_H__36FF2E9D_D4C2_49F6_B01E_1BFF69C6053E__INCLUDED_)

