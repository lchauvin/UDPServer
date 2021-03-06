/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

  ==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkUDPServerLogic.h"
#include "vtkMultiThreader.h"
#include "vtkKWTkUtilities.h"

#include <string>

vtkCxxRevisionMacro(vtkUDPServerLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkUDPServerLogic);

//---------------------------------------------------------------------------
vtkUDPServerLogic::vtkUDPServerLogic()
{
  // Timer Handling
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkUDPServerLogic::DataCallback);
  this->ServerStopFlag = true;
  this->ThreadID = -1;
  this->Thread = vtkMultiThreader::New();

  this->sock = 0;
  this->port = 0;
  this->received = 0;
  this->echolen = 0;
  this->clientlen = 0;
  this->serverlen = 0;
  this->ImportedData = NULL;

  // Dump UDP data
  this->UDPServerNode = NULL;
}

//---------------------------------------------------------------------------
vtkUDPServerLogic::~vtkUDPServerLogic()
{
  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  if(this->Thread)
    {
    this->Thread->Delete();
    }

  if (this->UDPServerNode)
    {
    this->UDPServerNode->Delete();
    }
}

//---------------------------------------------------------------------------
void vtkUDPServerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkUDPServerLogic:             " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkUDPServerLogic::DataCallback(vtkObject *caller,
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkUDPServerLogic *self = reinterpret_cast<vtkUDPServerLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkUDPServerLogic DataCallback");
  self->UpdateAll();
}

//---------------------------------------------------------------------------
void vtkUDPServerLogic::UpdateAll()
{
}

//---------------------------------------------------------------------------
int vtkUDPServerLogic::StartServerConnection()
{
  //Create the UDP socket
  if ((this->sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
    return 0;
    }
  else
    {
    if(this->port <= 0 || this->port > 65535)
      {
      return 0;
      }
    // Construct the server sockaddr_in structure
    memset(&echoserver, 0, sizeof(this->echoserver));     // Clear struct
    this->echoserver.sin_family = AF_INET;                // Internet IP
    this->echoserver.sin_addr.s_addr = htonl(INADDR_ANY); // Any IP address
    this->echoserver.sin_port = htons(this->port);        // server port

    // Setup the socket option to reuse
    int on = 1;
    setsockopt( this->sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    //Non blocking flag (useful if no data are coming, not wait for ever)
    int flags = fcntl(sock, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sock, F_SETFL, flags);

    // Bind the socket
    this->serverlen = sizeof(this->echoserver);
    if (bind(this->sock, (struct sockaddr *) &echoserver, this->serverlen) < 0)
      {
      return 0;
      }
    return 1;
    }
}

//--------------------------------------------------------------------------
void vtkUDPServerLogic::ImportData()
{
  // Receive a message from the client
  this->clientlen = sizeof(this->echoclient);
  this->received = 0;
  if ((this->received = recvfrom(this->sock, this->buffer, this->BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0)
    {
    //std::cerr << "Failed to receive message" << std::endl;
    }
  this->buffer[received]= '\0';

  this->ImportedData = this->buffer;
}

//-------------------------------------------------------------------------------
int vtkUDPServerLogic::Start(int p)
{
  //Assign port value
  this->port = p;
  this->ServerStopFlag = false;

  // Check if thread is detached
  if (this->ThreadID >= 0)
    {
    return 0;
    }
  //Start Thread
  this->ThreadID = this->Thread->SpawnThread((vtkThreadFunctionType) &vtkUDPServerLogic::ThreadFunction, this);

  // DUMP UDP Data

  time_t rawtime;
  time(&rawtime);
  struct tm* timeinfo;
  timeinfo = localtime(&rawtime);

  char timebuffer[128];
  strftime(timebuffer, 128, "BetaProbe_UDP_Dump_[%m_%d_%Y]_%H%M%S", timeinfo);

  std::stringstream filename;
  filename << timebuffer << ".log";
  this->udpDumpLogFile.open(filename.str().c_str(), std::ios::out);

#if DUMP_UDP
  this->DumpUDPDataTimerEvents();
#endif

  return 1;
}

//---------------------------------------------------------------------------
void* vtkUDPServerLogic::ThreadFunction(void* ptr)
{
  vtkMultiThreader::ThreadInfo* vinfo = static_cast<vtkMultiThreader::ThreadInfo*>(ptr);
  vtkUDPServerLogic* server = static_cast<vtkUDPServerLogic*>(vinfo->UserData);

  //Start the Server Connection
  int state = server->StartServerConnection();
  if (state == 0)
    {
    server->ServerStopFlag = true;
    }

  // Communication -- common to both Server and Client
  while (!server->ServerStopFlag)
    {
    server->ImportData();
    }

  server->ThreadID = -1;
  return NULL;
}

//---------------------------------------------------------------------------
int vtkUDPServerLogic::Stop()
{
  // Check if thread exists
  if (this->ThreadID >= 0)
    {
    this->ServerStopFlag = true;
    this->Thread->TerminateThread(this->ThreadID);
    this->ThreadID = -1;
    memset(buffer,0,sizeof(buffer));
    return 1;
    }
  else
    {
    return 0;
    }
}

#if DUMP_UDP
//---------------------------------------------------------------------------
void vtkUDPServerLogic::DumpUDPDataTimerEvents()
{
  if (this->UDPServerNode)
    {
    if (this->UDPServerNode->GetDumpUDP() == 1)
      {
      if (this->udpDumpLogFile.is_open())
        {
        this->udpDumpLogFile << this->ImportedData;
        }
      }
    }
  vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(),
				       100,
				       this, "DumpUDPDataTimerEvents");
  std::cerr << "DUMP UDP DATA" << std::endl;
}
#endif
