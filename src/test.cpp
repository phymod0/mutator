#include <iostream>
#include <vector>
#include "loader.hpp"
#include "event_iterator.hpp"


using namespace std;


static void event_iteration_cb(const vector<int>& outcomes, void *cb_data)
{
	cout << "Got outcomes in event iteration:";
	for (int idx : outcomes)
		cout << " " << idx;
	cout << "\n";
}


/* TODO: Test loader.cpp for file reading errors */
int main(int argc, char *argv[])
{
	frequency_data_loader loader;
	if (!loader.load_frequency_file("outfile")) {
		cerr << "Failed to load file!\n";
		return -1;
	}

	

	cout << "Exiting with return value " << 0 << "\n";
	return 0;
}
