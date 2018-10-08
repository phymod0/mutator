#include <iostream>
#include <vector>
#include "event_iterator.hpp"


using namespace std;


static void event_iteration_cb(const vector<int>& outcomes, void *cb_data)
{
	cout << "Got outcomes in event iteration:";
	for (int idx : outcomes)
		cout << " " << idx;
	cout << "\n";
}


int main(int argc, char *argv[])
{
	event_list ev_list(3);
	ev_list.set_event_sample_space(0, vector<int>({1, 2, 3, 4}),
			vector<unsigned long long>({100, 100, 100, 100}));
	ev_list.set_event_sample_space(1, vector<int>({5, 6}),
			vector<unsigned long long>({75, 25}));
	ev_list.set_event_sample_space(2, vector<int>({7, 8, 9}),
			vector<unsigned long long>({80, 80, 640}));

	ev_list.iterate_sorted(event_iteration_cb, NULL);

	return 0;
}
