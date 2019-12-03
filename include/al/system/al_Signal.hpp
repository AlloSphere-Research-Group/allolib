#ifndef AL_SIGNAL_H
#define AL_SIGNAL_H

/*

Manages signals & interrupts

need to invoke

registerSigInt(this);

in onCreate

This will allow onExit to execute even when app is abruptly killed.

Kon Hyong Kim, 2019
konhyong@gmail.com

*/

#include <csignal>
#include "al/app/al_App.hpp"

namespace al {

static void* userApp;
void* getUser() { return userApp; }

static void userHandler(int s) {
  // printf("Caught signal %d\n", s);
  ((App*)getUser())->onExit();
  
  exit(1);
}

void registerSigInt(void* app) {
  userApp = app;

#ifdef _WIN32
  signal(SIGINT, userHandler);
  signal(SIGTERM, userHandler);
  signal(SIGABRT, userHandler);
#else
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = userHandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
#endif
}

}  // namespace al

#endif