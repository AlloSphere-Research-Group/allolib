#include "al/util/ui/al_Parameter.hpp"

#include <iostream>

using namespace al;
using namespace std;

// Parameter declaration
// Access to these parameters via the get and set functions is competely
// thread safe
Parameter freq("Frequency", "", 440.0);
Parameter amp("Amplitude", "", 0.1);

// will be used for sending values to ParameterServer
osc::Send sender(9010, "127.0.0.1");

double interval_sec = 1.0; // time between random parameter changes

// This callback is registered below using the
// Parameter::setProcessingCallback() function. It is called any time the
// parameter's value is changed, no matter how that change occured.
float ampCallback(float ampdb, void *data)
{
  float amp = powf(10.0, ampdb / 20.0);
  cout << "`ampCallback` called. converting " << ampdb << "db to " << amp
       << endl;
  return amp;
}

int main(int argc, char *argv[])
{

  // Set the function to be called whenever the value of the "amp"
  // parameter changes
  // amp.setProcessingCallback(ampCallback, nullptr);
  amp.setProcessingCallback([&](float ampdb){
    float amp = powf(10.0, ampdb / 20.0);
    cout << "`ampCallback` called. converting " << ampdb << "db to " << amp
         << endl;
    return amp;
  });

  ParameterServer paramServer("127.0.0.1", 9010);
  // You can register parameters through the registerParameter() function
  //	paramServer.registerParameter(freq);
  //	paramServer.registerParameter(amp);
  // Or you can use the streaming operators:
  paramServer << freq << amp;

  // Print information about the server. Shows address, port and OSC parameter
  // addresses
  cout << "printing parameter server info" << endl;
  paramServer.print();

  // Try listening for OSC on this port to check that values are being forwarded
  paramServer.addListener("127.0.0.1", 13560); 

  int count = 0;
  while (count < 3) {
    cout << "\n\n" << endl; // separate print statements in ouput terminal

    // set value in c++ program
    {
      float newFreq = 440.0 * (1 + count / 2.0);
      float newAmpDb = -40.0 * count / 2.0;
      cout << "Setting through C++: Frequency " << newFreq
                               << " Amplitude " << newAmpDb << endl;
      freq = newFreq;    // The parameters can be set through C++ assignment:
      amp.set(newAmpDb); // or setter
      cout << "check values set: freq = " << freq.get()
                            << ", amp = " << amp.get() << '\n' << endl;
      al_sleep(interval_sec);
    }

    // Now do it through OSC:
    {
      float newFreq = 440.0 * (1 + (2 - count) / 2.0);
      float newAmpDb = -40.0 * (2 - count) / 2.0;
      cout << "Setting through OSC: Frequency " << newFreq <<
                                  " Amplitude " << newAmpDb << endl;

      sender.send("/Frequency", newFreq);
      sender.send("/Amplitude", newAmpDb);
      al_sleep(0.1); // wait for short time so param server can get message
      cout << "check values set: freq = " << freq.get()
                            << ", amp = " << amp.get() << '\n' << endl;
      al_sleep(interval_sec);
    }

    count += 1; // count to repeat 3 times and exit
  }

} // main
