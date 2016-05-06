/****************************************************************************
 * Copyright (c) 2014 VT MAK
 * All rights reserved.
 ****************************************************************************/

//! \file listen.cxx
//! \brief Contains the simple listen example.
//! \ingroup examples

#include <vl/exerciseConn.h>
#include <vl/exerciseConnInitializer.h>

#include <vlutil/vlProcessControl.h>
#include <vlutil/vlMiniDumper.h>

#include <vl/reflectedEntityList.h>
#include <vl/entityStateRepository.h>
#include <vl/reflectedEntity.h>
#include <vl/fireInteraction.h>
#include <vl/topoView.h>
#include <iostream>

int keybrdTick(void);



// Define a callback to process fire interactions.
void fireCb(DtFireInteraction* fire, void* /*usr*/)
{
   std::cout << "Fire Interaction from "
      <<  fire->attackerId().string() << std::endl;
}

int main(int argc, char** argv)
{
   DtINIT_MINIDUMPER( "Listen" );

   try
   {

      // Create an exercise conn initializer. This will parse the command
      // line, as well as parse an mtl file with the same name as this application.
      DtVrlApplicationInitializer appInit(argc, argv, "VR-Link Listen");

#if DtDIS
      // Please note, this is the only protocol specific code in this example. For
      // HLA there is no network connection, so there is no concept of AsyncIO at the
      // VR-Link level.
      appInit.setUseAsynchIO(true);
#endif

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
      DtExerciseConn exConn(appInit);

      // Register a callback to handle fire interactions.
      DtFireInteraction::addCallback(&exConn, fireCb, NULL);

      // Create an object to manage entities that we hear about
      // on the network.
      DtReflectedEntityList rel(&exConn);

      // Initialize VR-Link time.
      DtClock* clock = exConn.clock();

      while (true)
      {
         // Check if user hit 'q' to quit.
         if (keybrdTick() == -1)
            break;

         // Tell VR-Link the current value of simulation time.
         clock->setSimTime(clock->elapsedRealTime());

         // Process any incoming messages.
         exConn.drainInput();

         // Find the first entity in the reflected entity list.
         DtReflectedEntity *first = rel.first();

         if (first)
         {
            // Grab its state repository, where we can inspect its data.
            DtEntityStateRepository *esr = first->entityStateRep();

            // Create a topographic view on the state repository, so we
            // can look at position information in topographic coordinates.
            double refLatitude  = DtDeg2Rad(  35.699760);
            double refLongitude = DtDeg2Rad(-121.326577);
            DtTopoView topoView(esr, refLatitude, refLongitude);

            // Print the position.
            // Since it returns a DtString, we need to force it to const char*
            // with a cast.
            std::cout << "Position of first entity: "
               << topoView.location().string() << std::endl;

         }

         // Sleep till next iteration.
         DtSleep(0.1);
      }
   }
   DtCATCH_AND_WARN(std::cout);
   return 0;
}

int keybrdTick()
{
   char *keyPtr = DtPollBlockingInputLine();
   if (keyPtr && (*keyPtr == 'q' || *keyPtr == 'Q'))
      return -1;
   else
      return 0;
}


