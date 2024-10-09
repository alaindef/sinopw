// FSData.cpp: implementation of the CFSData class.
//
//////////////////////////////////////////////////////////////////////

#include "FSData.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <cstring>
//#include <netinet/in.h>                                                                     //adf
//#include <arpa/inet.h>                                                                      //adf

using namespace std;
#define DE2RA 0.017453292
#define PI 3.14159265
#define _snprintf_s(a, b, c, ...) snprintf(a, b, __VA_ARGS__)

// Thread Data
std::thread ListeningThreadID;
std::mutex mutexCallback;
bool gListeningThreadEnabled = false;

CFSData::CFSData()
//: mG1000Object(NULL)
{
    InitData();
}
void CFSData::InitData()
{
    m_SocketConnected = false;
    int iResult;
}

CFSData::~CFSData()
{
    if (m_SocketConnected)
    {
        CloseSocket();
    }
}

//////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS
//////////////////////////////////////////////////////////////////////

// CreateSocket
//
// Called at the beginnig of the program.
// Creates 2 sockets using the WINSOCK library (for reading and writing from and to X-Plane
//
// Initializes both sockets to
//  1) broadcast udp packets
//  2) being able to re-use an address/port, if the current computer already has a socket initialized
//     with same address/port
//
// Initialize structs to send data to x-plane.
//(Set the header)
bool CFSData::CreateSocket()
{
    m_SocketConnected = false;
    // setup socket to receive data from X-plane
    ListeningSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ListeningSock < 0)
        return false;
    int errorcode = errno;

    char broadcast = '1';
    socklen_t broadcast_len = sizeof(broadcast);
    char reuseaddr = '1';
    socklen_t reuseaddr_len = sizeof(reuseaddr);
    DWORD buff_size;
    socklen_t buff_size_len = sizeof(buff_size);
    if (getsockopt(ListeningSock, SOL_SOCKET, SO_BROADCAST, &broadcast, &broadcast_len) < 0)
        cout << "RCV SO_BROADCAST problem" << endl;
    errorcode = errno;
    if (getsockopt(ListeningSock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &reuseaddr_len) < 0)
        cout << "RCV SO_REUSEADDR problem" << endl;
    errorcode = errno;
    if (getsockopt(ListeningSock, SOL_SOCKET, SO_RCVBUF, (char*) &buff_size, &buff_size_len))
        cout << "RCV SO_RCVBUF problem" << endl;
    errorcode = errno;
    cout << "Receive buffer size: " << buff_size << std::endl;

    char *errormsg = strerror(errorcode);
    if (errorcode)
    {
        cout << "error code received" << endl;
        closesocket(ListeningSock);
        return false;
    }
    Any_addr.sin_family = AF_INET;
    Any_addr.sin_port = htons(UDPDATAPORT_PIDPLUGIN);
    Any_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int binderror = ::bind(ListeningSock, (sockaddr *)&Any_addr, sizeof(Any_addr));
    if (binderror < 0)
    {
        closesocket(ListeningSock);
        return false;
    }

    // mdf
    //int sock_timeout_ms = 2000;
    DWORD tv = 100;
    // tv.tv_sec = sock_timeout_ms / 1000;
    
    // adf errno 33 = EDOM
    // adf https://www.linuxquestions.org/questions/programming-9/problem-with-setsockopt-errornumbers-580642/
    // adf https://www.ibm.com/docs/en/zos/2.2.0?topic=codes-sockets-return-errnos
    // tv.tv_sec = 100;                                                                                  //adf 
    // tv.tv_usec = 500000;                                                                            //adf
    setsockopt(ListeningSock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof(tv));
    //

    m_len_sockaddr_in = sizeof(struct sockaddr_in);

    // setup socket to send events to X-Plane
    SendingSock = socket(AF_INET, SOCK_DGRAM, 0);
    broadcast = 1;
    if (setsockopt(SendingSock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
        cout << "send -- SO_BROADCAST problem" << endl;
    int senderror = errno;
    cout << "SO_BROADCAST senderror = " << senderror << " broodkast = " << broadcast << endl;

    if (setsockopt(SendingSock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0)
        cout << "send -- SO_REUSEADDR problem" << endl;
    senderror = errno;
    cout << "SO_REUSEADDR senderror = " << senderror << " broodkast = " << broadcast << endl;
    
    cout << "port UDPCOMMANDPORT_PIDPLUGIN = " << UDPCOMMANDPORT_PIDPLUGIN << endl;
    if (senderror)
    {
        closesocket(SendingSock);
        closesocket(ListeningSock);
        return false;
    }
    Broadcast_addr.sin_family = AF_INET;
    Broadcast_addr.sin_port = htons(UDPCOMMANDPORT_PIDPLUGIN);
    cout << "port UDPCOMMANDPORT_PIDPLUGIN = " << UDPCOMMANDPORT_PIDPLUGIN << endl; 
    Broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    // Broadcast_addr.sin_addr.s_addr = inet_addr("192.168.0.255");                                  //adf

    m_SetDataLength = sizeof(DataToXPlane) + 1;

    m_SocketConnected = true;

    // Create a thread that listens for messages from clients
    gListeningThreadEnabled = true;
    ListeningThreadID = std::thread(ListeningThread, (void *)this);

    cout << "Socket OK"  << endl;
 
    return m_SocketConnected;
}

// CloseSocket
//-----------
//
// Closes both sockets
void CFSData::CloseSocket()
{
    // Let the listening thread end, then wait for it to end
    gListeningThreadEnabled = false;
    ListeningThreadID.join();

    // close socket
    closesocket(SendingSock);
    closesocket(ListeningSock);
    m_SocketConnected = false;
}

//
void CFSData::ReadFromFS()
{
    // test -->
    /*m_BankAngle = 20;
    m_Heading = 20;
    m_indicatedairspeed = 200;
    m_trueairspeed = 200;
    m_verticalspeed = 1000;
    m_pitchangle = 5;
    m_pitchrate = 0;
    m_rollrate = 0;
    m_yawrate = 0;
    m_truetrack = 20;
    m_elevatorposition = 5;
    m_elevatortrimposition = 5;

    m_latitude = 0.0;
    m_longitude = 0.0;
    m_altitude = 4000;
    m_groundspeed = 200;
    m_truewinddir = 0;
    m_windspeed = 0;
    m_magvar = 0;
    return;*/
    // <-- test

    // check if there is a socket connected to the server (the x-plane plugin)
    if (!m_SocketConnected)
        return;

    // wait until there is a UDP packet
    recvfrom(ListeningSock, (char *)&m_buffer, sizeof(m_buffer), 0, (sockaddr *)&Any_addr, &m_len_sockaddr_in);
    // cout<<"received UDP : "<<m_buffer<<endl<<endl;
    // if it's a message from the X-Plane plugin it starts with "PID_DTA"
    if (strncmp(m_buffer, "PID_DTA", 7) == 0)
    {
        // interpret the data as a DataFromXPlane structure
        m_datafromxplane = (DataFromXPlane *)m_buffer;
        mutexCallback.lock();
        m_BankAngle = m_datafromxplane->bankangle;
        m_Heading = m_datafromxplane->heading;
        m_indicatedairspeed = m_datafromxplane->indicatedairspeed;
        m_trueairspeed = m_datafromxplane->trueairspeed;
        m_verticalspeed = m_datafromxplane->verticalspeed;
        m_pitchangle = m_datafromxplane->pitchangle;
        m_pitchrate = m_datafromxplane->pitchrate;
        m_rollrate = m_datafromxplane->rollrate;
        m_yawrate = m_datafromxplane->yawrate;
        m_truetrack = m_datafromxplane->truetrack;
        m_elevatorposition = m_datafromxplane->elevatorpos;
        m_elevatortrimposition = m_datafromxplane->elevatortrimpos;

        m_latitude = m_datafromxplane->latitude;
        m_longitude = m_datafromxplane->longitude;
        m_altitude = m_datafromxplane->altitude;
        m_groundspeed = m_datafromxplane->groundspeed;
        m_truewinddir = m_datafromxplane->truewinddir;
        m_windspeed = m_datafromxplane->windspeed;
        m_magvar = m_datafromxplane->magvar;
        m_gforce = m_datafromxplane->gforce;

        mutexCallback.unlock();
    }
}

float CFSData::GetBankAngle()
{
    float value;
    mutexCallback.lock();
    value = m_BankAngle;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetHeading()
{
    float value;
    mutexCallback.lock();
    value = m_Heading;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetIndicatedairspeed()
{
    float value;
    mutexCallback.lock();
    value = m_indicatedairspeed;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetTrueairspeed()
{
    float value;
    mutexCallback.lock();
    value = m_trueairspeed;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetVerticalspeed()
{
    float value;
    mutexCallback.lock();
    value = m_verticalspeed;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetPitchangle()
{
    float value;
    mutexCallback.lock();
    value = m_pitchangle;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetPitchrate()
{
    float value;
    mutexCallback.lock();
    value = m_pitchrate;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetRollrate()
{
    float value;
    mutexCallback.lock();
    value = m_rollrate;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetYawrate()
{
    float value;
    mutexCallback.lock();
    value = m_yawrate;
    mutexCallback.unlock();
    return value;
}
float CFSData::GetTruetrack()
{
    float value;
    mutexCallback.lock();
    value = m_truetrack;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetElevatorPosition()
{
    float value;
    mutexCallback.lock();
    value = m_elevatorposition;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetElevatorTrimPosition()
{
    float value;
    mutexCallback.lock();
    value = m_elevatortrimposition;
    mutexCallback.unlock();
    return value;
}

double CFSData::GetLatitude()
{
    double value;
    mutexCallback.lock();
    value = m_latitude;
    mutexCallback.unlock();
    return value;
}

double CFSData::GetLongitude()
{
    double value;
    mutexCallback.lock();
    value = m_longitude;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetAltitude()
{
    float value;
    mutexCallback.lock();
    value = m_altitude;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetGroundspeed()
{
    float value;
    mutexCallback.lock();
    value = m_groundspeed;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetTrueWinddirection()
{
    float value;
    mutexCallback.lock();
    value = m_truewinddir;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetWindspeed()
{
    float value;
    mutexCallback.lock();
    value = m_windspeed;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetMagneticvatiation()
{
    float value;
    mutexCallback.lock();
    value = m_magvar;
    mutexCallback.unlock();
    return value;
}

float CFSData::GetVerticalAccelerationG()
{
    float value;
    mutexCallback.lock();
    value = m_gforce;
    mutexCallback.unlock();
    return value;
}

void CFSData::SetAileron(float aileronPos)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_BNK_VAL");
    m_SetDataControlPosition.unionValue.fValue = aileronPos;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void CFSData::SetAileronOverride(bool isOverride)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_BNK_OVR");
    m_SetDataControlPosition.unionValue.bValue = isOverride;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void CFSData::SetElevator(float elevatorPos)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_PCH_VAL");
    m_SetDataControlPosition.unionValue.fValue = elevatorPos;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void CFSData::SetRuddder(float rudderPos)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_RUD_VAL");
    m_SetDataControlPosition.unionValue.fValue = rudderPos;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void CFSData::SetElevatorOverride(bool isOverride)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_PCH_OVR");
    m_SetDataControlPosition.unionValue.bValue = isOverride;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void CFSData::SetElevatorTrimUp(bool trimActive)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_TRIM_UP");
    m_SetDataControlPosition.unionValue.bValue = trimActive;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void CFSData::SetElevatorTrimDown(bool trimActive)
{
    if (!m_SocketConnected)
        return;

    strcpy(m_SetDataControlPosition.header, "PID_TRIM_DN");
    m_SetDataControlPosition.unionValue.bValue = trimActive;
    sendto(SendingSock, (char *)&m_SetDataControlPosition, m_SetDataLength, 0, (sockaddr *)&Broadcast_addr, sizeof(Broadcast_addr));
}

void *ListeningThread(void *pvoid)
{
    std::cout << "starting thread" << endl;
    if (!pvoid)
        return NULL;
    CFSData *fsData = (CFSData *)pvoid;
    while (gListeningThreadEnabled)
    {
        fsData->ReadFromFS();
    }
    std::cout << "stopping thread" << endl;
    return NULL;
}
