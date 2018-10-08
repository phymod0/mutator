#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "dag_implicit.hpp"
#include "event_iterator.hpp"

#define ALLOC(x, n) (x = (typeof(x))malloc(n * sizeof(*(x))))
#define STATIC_ARRLEN(array) (sizeof(array)/sizeof(*(array)))
#define THROW_ERROR(str) \
	(fprintf(stderr, "Error in %s: " str "...\n", __func__))


// XXX: Global variable to emulate the expand closure (do not run the iterator
// in more than one thread).
static event_list *__global_event_list;


class outcome_list {
	public:
	int n_events;
	int *outcome_idx_list;
	inline outcome_list(void) {}
	outcome_list(int n_events, int *outcome_idx_list);
	outcome_list(const outcome_list& list);
	bool operator <(const outcome_list& rhs) const;
	bool operator >(const outcome_list& rhs) const;
	bool operator <=(const outcome_list& rhs) const;
	bool operator >=(const outcome_list& rhs) const;
	bool operator ==(const outcome_list& rhs) const;
	bool operator !=(const outcome_list& rhs) const;
	~outcome_list(void);
};


static int likelier(struct outcome a, struct outcome b)
{
	return a.logarithmic_probability > b.logarithmic_probability;
}


static double outcome_list_delta(outcome_list* l_prev, outcome_list* l_next)
{
	int i, n_events = l_prev->n_events;
	struct outcome prev, next;
	for (i=0; i<n_events; i++)
		if (l_prev->outcome_idx_list[i] != l_next->outcome_idx_list[i])
			break;
	if (i == n_events)
		return 0.0;
	prev = __global_event_list->events[i][l_prev->outcome_idx_list[i]];
	next = __global_event_list->events[i][l_next->outcome_idx_list[i]];
	return prev.logarithmic_probability - next.logarithmic_probability;
}


static void print_outcome_list(outcome_list *list)
{
	char *sep = (char*)"{";
	int n_events = list->n_events;
	int *outcome_idx_list = list->outcome_idx_list;
	const vector<vector<outcome>>& ev_table = __global_event_list->events;
	for (int i=0; i<n_events; i++) {
		int outcome_idx = outcome_idx_list[i];
		cout << sep << ev_table[i][outcome_idx].identifier;
		sep = (char*)", ";
	}
	cout << "}";
}


static vector<outcome_list> expand_outcome_list(outcome_list *list)
{
	int n_events, *outcome_idx_list;
	vector<outcome_list> next_outcomes;

	n_events = list->n_events;
	outcome_idx_list = list->outcome_idx_list;
	const vector<vector<outcome>>& ev_table = __global_event_list->events;
	outcome_list next_list(n_events, NULL);
	for (int i=0; i<n_events; i++) {
		int old_outcome_idx = outcome_idx_list[i];
		int new_outcome_idx = old_outcome_idx + 1;
		if (new_outcome_idx >= ev_table[i].size())
			continue;
		outcome_idx_list[i] = new_outcome_idx;
		next_list.outcome_idx_list = outcome_idx_list;
		next_outcomes.push_back(next_list);
		outcome_idx_list[i] = old_outcome_idx;
	}

	// Do not let the destructor free the base outcome list
	next_list.outcome_idx_list = NULL;
	return next_outcomes;
}


void event_list::iterate_sorted(iterator_cb_t iterator_cb, void *cb_data)
{
	int *seed_list;
	__global_event_list = this;
	dag_implicit<outcome_list> event_base(expand_outcome_list,
						  outcome_list_delta);
	if (!ALLOC(seed_list, n_events))
		goto oom;
	for (int i=0; i<n_events; i++)
		seed_list[i] = 0;

	event_base.set_start(outcome_list(n_events, seed_list));
	for (const outcome_list& event : event_base) {
		vector<int> outcome_identifiers;
		for (int i=0; i<event.n_events; i++) {
			int outcome_idx = event.outcome_idx_list[i];
			int id = events[i][outcome_idx].identifier;
			outcome_identifiers.push_back(id);
		}
		iterator_cb(outcome_identifiers, cb_data);
	}

	return;
 oom:
	THROW_ERROR("Out of memory");
}


static void sort_sample_space(vector<struct outcome>& unsorted)
{
	std::sort(unsorted.begin(), unsorted.end(), likelier);
}


bool outcome_list::operator <(const outcome_list& rhs) const
{
	if (n_events != rhs.n_events)
		return false;
	for (int i=0; i<n_events; i++) {
		if (outcome_idx_list[i] < rhs.outcome_idx_list[i])
			return true;
		if (outcome_idx_list[i] > rhs.outcome_idx_list[i])
			return false;
	}
	return false;
}


bool outcome_list::operator >(const outcome_list& rhs) const
{
	if (n_events != rhs.n_events)
		return false;
	for (int i=0; i<n_events; i++) {
		if (outcome_idx_list[i] > rhs.outcome_idx_list[i])
			return true;
		if (outcome_idx_list[i] < rhs.outcome_idx_list[i])
			return false;
	}
	return false;
}


bool outcome_list::operator <=(const outcome_list& rhs) const
{
	if (n_events != rhs.n_events)
		return false;
	for (int i=0; i<n_events; i++) {
		if (outcome_idx_list[i] < rhs.outcome_idx_list[i])
			return true;
		if (outcome_idx_list[i] > rhs.outcome_idx_list[i])
			return false;
	}
	return true;
}


bool outcome_list::operator >=(const outcome_list& rhs) const
{
	if (n_events != rhs.n_events)
		return false;
	for (int i=0; i<n_events; i++) {
		if (outcome_idx_list[i] > rhs.outcome_idx_list[i])
			return true;
		if (outcome_idx_list[i] < rhs.outcome_idx_list[i])
			return false;
	}
	return true;
}


bool outcome_list::operator ==(const outcome_list& rhs) const
{
	if (n_events != rhs.n_events)
		return false;
	for (int i=0; i<n_events; i++) {
		if (outcome_idx_list[i] != rhs.outcome_idx_list[i])
			return false;
	}
	return true;
}


bool outcome_list::operator !=(const outcome_list& rhs) const
{
	if (n_events != rhs.n_events)
		return true;
	for (int i=0; i<n_events; i++) {
		if (outcome_idx_list[i] != rhs.outcome_idx_list[i])
			return true;
	}
	return false;
}


outcome_list::outcome_list(int n_events, int *outcome_idx_list)
{
	this->n_events = n_events;
	this->outcome_idx_list = outcome_idx_list;
}


outcome_list::outcome_list(const outcome_list& list)
{
	n_events = list.n_events;
	if (!ALLOC(outcome_idx_list, list.n_events))
		goto oom;
	for (int i=0; i<list.n_events; i++)
		outcome_idx_list[i] = list.outcome_idx_list[i];
	return;
 oom:
	THROW_ERROR("Out of memory");
}


outcome_list::~outcome_list(void)
{
	if (outcome_idx_list)
		free(outcome_idx_list);
}


event_list::event_list(int n_events)
{
	this->n_events = n_events;
	for (int i=0; i<n_events; i++) {
		vector<outcome> empty;
		events.push_back(empty);
	}
}


event_list::~event_list(void)
{
	return;
}


void event_list::set_event_sample_space(int event_idx, vector<int> outcomes,
					vector<unsigned long long> freqs)
{
	double freq_sum_log = 0.0;
	unsigned long long freq_sum = 0;
	for (auto freq : freqs)
		freq_sum += freq;
	freq_sum_log = log((double)freq_sum);

	for (int i=0; i<outcomes.size(); i++) {
		double freq_log = log((double)freqs[i]);
		events[event_idx].push_back((struct outcome){
			.identifier = outcomes[i],
			.added_index = i,
			.logarithmic_probability = freq_log - freq_sum_log,
		});
	}
	sort_sample_space(events[event_idx]);

	return;
 oom:
	THROW_ERROR("Out of memory");
}
