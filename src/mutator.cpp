#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include "loader.hpp"
#include "event_iterator.hpp"

#define FREQDATA_DEFAULT_PATH ((char*)("/usr/share/mutator/mt_freqdata.frq"))


using namespace std;


typedef unsigned long long frequency;


struct ev_data {
	char *seed;
	int n_events;
	vector<string> prefixes, suffixes;
	vector<char> leadingchar_replacements;
	vector<vector<char>> normal_replacements;
};


static void event_iteration_cb(const vector<int>& outcomes, void *cb_data)
{
#if 0
	cout << "Got outcomes in event iteration:";
	for (int idx : outcomes)
		cout << " " << idx;
	cout << "\n";
#endif
	struct ev_data *data = (struct ev_data*)cb_data;
	data->seed[0] = data->leadingchar_replacements[outcomes[1]];
	for (int i=2; i<data->n_events-1; i++)
		data->seed[i-1] = data->normal_replacements[i-2][outcomes[i]];
	cout << data->prefixes[outcomes[0]];
	cout << data->seed;
	cout << data->suffixes[outcomes[data->n_events-1]] << endl;
}


int main(int argc, char *argv[])
{
	int n_events = 0;
	event_list *ev_list;
	frequency_data_loader loader;
	char *freq_file = FREQDATA_DEFAULT_PATH, *seed;
	struct ev_data cb_data;
	vector<int> event_indices;
	vector<frequency> outcome_freqs;

	if (argc != 2 && argc != 3) {
		cerr << "Usage: " << argv[0] << " <seed word> [<custom path "
		     << "to frequency data file>]\n";
		return -1;
	}
	seed = argv[1];
	if (argc == 3)
		freq_file = argv[2];

	if (!loader.load_frequency_file(freq_file)) {
		cerr << "No frequency data loaded from " << freq_file << "\n";
		cerr << "Frequency data missing or corrupt...\n";
		return -1;
	}

	// Initialize event list
	n_events += strlen(seed);
	n_events += 1 + 1; // Prefix and suffix events
	cb_data.seed = seed;
	cb_data.n_events = n_events;
	ev_list = new event_list(n_events);

	// Prefix events
	loader.get_prefix_frequencies(cb_data.prefixes, outcome_freqs);
	event_indices.clear();
	for (int i=0; i<cb_data.prefixes.size(); i++)
		event_indices.push_back(i);
	ev_list->set_event_sample_space(0, event_indices, outcome_freqs);

	// Leading character events
	loader.get_leadingchar_frequencies(seed[0],
					   cb_data.leadingchar_replacements,
					   outcome_freqs);
	event_indices.clear();
	for (int i=0; i<cb_data.leadingchar_replacements.size(); i++)
		event_indices.push_back(i);
	ev_list->set_event_sample_space(1, event_indices, outcome_freqs);

	// Normal character events
	for (int i=1; seed[i]; i++) {
		vector<char> replacements;
		loader.get_normalchar_frequencies(seed[i], replacements,
						  outcome_freqs);
		event_indices.clear();
		for (int j=0; j<replacements.size(); j++)
			event_indices.push_back(j);
		ev_list->set_event_sample_space(i+1, event_indices,
						outcome_freqs);
		cb_data.normal_replacements.push_back(replacements);
	}

	// Suffix events
	loader.get_suffix_frequencies(cb_data.suffixes, outcome_freqs);
	event_indices.clear();
	for (int i=0; i<cb_data.suffixes.size(); i++)
		event_indices.push_back(i);
	ev_list->set_event_sample_space(n_events-1, event_indices,
					outcome_freqs);

	// Main iteration
	ev_list->iterate_sorted(event_iteration_cb, &cb_data);

	delete ev_list;
	return 0;
}
