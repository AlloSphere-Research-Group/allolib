#include "al/core.hpp"
#include <iostream>

using namespace std;
using namespace al;

int main(int argc, char* argv[]) {
	auto now = al_system_time();
	cout << "start: " << now << endl;

	al_sleep(2);

	auto later = al_system_time();
	// should print 2
	cout << "slept: " << later - now << endl;

	al_start_steady_clock();
	auto t0 = al_steady_time();
	cout << "t0: " << t0 << endl;

	al_sleep_until(now + 5);

	auto t1 = al_steady_time();
	// should print 3
	cout << "t1: " << t1 << endl;

	al_reset_steady_clock();

	al_sleep(2);

	auto t2 = al_steady_time();
	// should print 2
	cout << "t2: " << t2 << endl;

    return 0;
}