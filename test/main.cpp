#include "al/core.hpp"
#include <iostream>

using namespace std;
using namespace al;

int main(int argc, char* argv[]) {
	auto now = al_system_time();
	al_sleep(2);
	auto later = al_system_time();
	al_start_steady_clock();
	auto t0 = al_steady_time();
	al_sleep_until(now + 5);
	auto t1 = al_steady_time();
	al_reset_steady_clock();
	al_sleep(2);
	auto t2 = al_steady_time();

	cout << "start: " << now << "\n";
	// should print 2
	cout << "slept: " << later - now << "\n";
	cout << "t0: " << t0 << "\n";
	// should print 3
	cout << "t1: " << t1 << "\n";
	// should print 2
	cout << "t2: " << t2 << endl;

    return 0;
}