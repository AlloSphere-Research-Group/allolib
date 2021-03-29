
#include "al/app/al_App.hpp"
#include "al/io/al_PersistentConfig.hpp"

using namespace al;

struct MyApp : App {
  int64_t count = 0;
  std::string lastRun = "First run!";

  void onInit() override {
    PersistentConfig config;
    config.registerInt("runs", &count);
    config.registerString("lastRun", &lastRun);

    if (!config.read()) {
      std::cout << "Error reading persistent config!" << std::endl;
    }
    std::cout << "This app last ran on:" << lastRun << std::endl;
    std::cout << "Num runs: " << ++count << std::endl;

    // Get time as string
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    lastRun = buffer;

    config.write();
    quit();
  }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
