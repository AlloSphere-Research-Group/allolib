#include <iostream>

#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

using namespace al;
using namespace std;

// Parameter declaration
// Access to these parameters via the get and set functions is competely
// thread safe
Parameter freq("Frequency", "", 440.0);
Parameter amp("Amplitude", "", 0.1f);

Parameter naming("Name", "Group/0", 0.1f);

// will be used for sending values to ParameterServer
osc::Send sender(9010, "127.0.0.1");

double interval_sec = 1.0; // time between random parameter changes

int main() {
  // A ParameterServer exposes parameters on the network for OSC control
  ParameterServer paramServer("127.0.0.1", 9010);
  // You can register parameters through the registerParameter() function
  //	paramServer.registerParameter(freq);
  //	paramServer.registerParameter(amp);
  // Or you can use the streaming operators:
  paramServer << freq << amp << naming;

  paramServer.verbose();
  // Print information about the server. Shows address, port and OSC parameter
  // addresses
  paramServer.print();

  // Try listening for OSC on this port to check that values are being forwarded
  paramServer.addListener("127.0.0.1", 13560);

  //  // Set the function to be called whenever the value of the "freq"
  //  // parameter changes
  freq.registerChangeCallback([&](float newFreq) {
    cout << "frequency has changed from " << freq.get() << " to " << newFreq
         << endl;
    return amp;
  });

  //  // Specify a function to transform incoming values for amp
  amp.setProcessingCallback([&](float ampdb) {
    float amp = powf(10.0f, ampdb / 20.0);
    cout << "Converting " << ampdb << "db to " << amp << endl;
    return amp;
  });

  int count = 0;
  while (count < 3) {
    //    cout << "\n\n" << endl; // separate print statements in ouput terminal

    // set value in c++ program
    {
      float newFreq = 440.0 * (1 + count / 2.0f);
      float newAmpDb = -40.0f * count / 2.0f;
      cout << "Setting through C++: Frequency " << newFreq << endl;
      freq =
          newFreq; // The parameters can be set through regular C++ assignment
      cout << "Setting through C++: Amplitude " << newAmpDb << endl;
      amp.set(newAmpDb); // or the set function
      cout << "check values set: freq = " << freq.get()
           << ", amp = " << amp.get() << '\n'
           << endl;
      al_sleep(interval_sec);
    }

    // Now do it through OSC:
    {
      float newFreq = 440.0 * (1 + (2 - count) / 2.0f);
      float newAmpDb = -40.0f * (2 - count) / 2.0f;
      cout << "Setting through OSC: Frequency " << newFreq << " Amplitude "
           << newAmpDb << endl;

      sender.send("/Frequency", newFreq);
      sender.send("/Amplitude", newAmpDb);
      al_sleep(0.1); // wait for short time so param server can get message
      cout << "check values set: freq = " << freq.get()
           << ", amp = " << amp.get() << '\n'
           << endl;
      al_sleep(interval_sec);
    }

    count += 1; // count to repeat 3 times and exit
  }

} // main
