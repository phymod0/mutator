#ifndef EVENT_ITERATOR
#define EVENT_ITERATOR

#include <vector>


using namespace std;


typedef void (*iterator_cb_t)(const vector<int>& outcomes, void *cb_data);


struct outcome {
	int identifier, added_index;
	double logarithmic_probability;
};

class event_list {
	public:
	int n_events;
	vector<vector<outcome>> events;
	event_list(int n_events);
	void set_event_sample_space(int event_idx, vector<int> outcomes,
				    vector<unsigned long long> freqs);
	void iterate_sorted(iterator_cb_t iterator_cb, void *cb_data);
	~event_list(void);
};
#endif /* EVENT_ITERATOR */
