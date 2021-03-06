/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLUDPServerNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $

  =========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include "vtkObjectFactory.h"
#include "vtkMRMLUDPServerNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkObjectFactory.h"

//------------------------------------------------------------------------------
vtkMRMLUDPServerNode* vtkMRMLUDPServerNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUDPServerNode");
  if(ret)
    {
    return (vtkMRMLUDPServerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUDPServerNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLUDPServerNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUDPServerNode");
  if(ret)
    {
    return (vtkMRMLUDPServerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUDPServerNode;
}

//----------------------------------------------------------------------------
vtkMRMLUDPServerNode::vtkMRMLUDPServerNode()
{
  this->HideFromEditors = 0;
  this->SmoothedCounts = 0;
  this->BetaCounts = 0;
  this->GammaCounts = 0;
  this->strDate = "01/01/01";
  this->strTime = "01:01:01";
  this->TInterval = 1000;

  this->DumpUDP = 0;

  this->SmoothedLock = vtkMutexLock::New();
}

//----------------------------------------------------------------------------
vtkMRMLUDPServerNode::~vtkMRMLUDPServerNode()
{
  if(this->SmoothedLock)
    {
    this->SmoothedLock->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLUDPServerNode::WriteXML(ostream& of, int nIndent)
{

}

//----------------------------------------------------------------------------
void vtkMRMLUDPServerNode::ReadXMLAttributes(const char** atts)
{

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLUDPServerNode::Copy(vtkMRMLNode *anode)
{

}

//----------------------------------------------------------------------------
void vtkMRMLUDPServerNode::PrintSelf(ostream& os, vtkIndent indent)
{

}

//----------------------------------------------------------------------------
int vtkMRMLUDPServerNode::GetSmoothedCounts()
{
  return this->SmoothedCounts;
}

//----------------------------------------------------------------------------
void vtkMRMLUDPServerNode::SetSmoothedCounts(int sc)
{
  this->SmoothedLock->Lock();
  this->SmoothedCounts = sc;
  this->SmoothedLock->Unlock();
}
