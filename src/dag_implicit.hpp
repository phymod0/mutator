#ifndef ACYCLIC_ITERABLE
#define ACYCLIC_ITERABLE

#include <iostream>
#include <vector>
#include <queue>
#include <set>


using namespace std;


template <class T>
struct node_cost {
	double total_cost;
	T* destination_node;
	// A depth count can be used to avoid redundant visits in the iteration
	// if edge composition is commutative. This can speed up iterations on
	// e.g. a 'manhattan' graph.
	unsigned long long depth;
	inline bool operator <(const node_cost& rhs) const
	{
		if (total_cost == rhs.total_cost)
			return depth < rhs.depth;
		return total_cost < rhs.total_cost;
	}
	inline bool operator >(const node_cost& rhs) const
	{
		if (total_cost == rhs.total_cost)
			return depth > rhs.depth;
		return total_cost > rhs.total_cost;
	}
	inline bool operator ==(const node_cost& rhs) const
	{
		if (total_cost != rhs.total_cost)
			return false;
		if (depth != rhs.depth)
			return false;
		return true;
	}
	inline bool operator !=(const node_cost& rhs) const
	{
		if (total_cost != rhs.total_cost)
			return true;
		if (depth != rhs.depth)
			return true;
		return false;
	}
};

/* NOTE: Strict weak ordering of T required for set implementation */
template <class T>
class dag_implicit {
	typedef vector<T> (*expander_t)(T*);
	typedef double (*edge_cost_t)(T*,T*);
	private:
	T *start_node;
	expander_t expand;
	edge_cost_t edge_cost;
	public:
	class dijkstra_iterator {
		private:
		set<T> nodes_enqueued;
		expander_t expand;
		edge_cost_t edge_cost;
		priority_queue<node_cost<T>, vector<node_cost<T>>,
			       greater<node_cost<T>>> dijkstra_q;
		public:
		void operator ++();
		const T& operator *();
		bool operator ==(const dijkstra_iterator& rhs);
		bool operator !=(const dijkstra_iterator& rhs);
		void set_start(T start_node);
		void set_graph(expander_t, edge_cost_t);
		bool complete(void) const;
	};
	dag_implicit(expander_t, edge_cost_t);
	void set_start(T start_node);
	inline dijkstra_iterator begin(void)
	{
		dijkstra_iterator iterator;
		iterator.set_graph(expand, edge_cost);
		iterator.set_start(*start_node);
		return iterator;
	}
	inline dijkstra_iterator end(void)
	{
		dijkstra_iterator empty_iterator;
		return empty_iterator;
	}
	~dag_implicit(void);
};


template <class T>
dag_implicit<T>::dag_implicit(expander_t expand, edge_cost_t edge_cost)
{
	this->start_node = NULL;
	this->expand = expand;
	this->edge_cost = edge_cost;
}


template <class T>
void
dag_implicit<T>::set_start(T start_node)
{
	if (this->start_node)
		delete this->start_node;
	this->start_node = new T(start_node);
}


template <class T>
dag_implicit<T>::~dag_implicit(void)
{
	if (this->start_node)
		delete this->start_node;
}


template <class T>
void
dag_implicit<T>::dijkstra_iterator::operator ++()
{
	if (dijkstra_q.empty())
		return;
	const node_cost<T>& nc = dijkstra_q.top();
	double total_cost = nc.total_cost;
	T* max_node = nc.destination_node;
	unsigned long long depth = nc.depth;
	const vector<T>& children = expand(max_node);

	dijkstra_q.pop();
	nodes_enqueued.erase(*max_node);
	for (const T& child : children) {
		T* next;
		if (nodes_enqueued.find(child) != nodes_enqueued.end())
			continue;
		next = new T(child);
		node_cost<T> next_cost = (node_cost<T>){
			.total_cost = total_cost + edge_cost(max_node, next),
			.destination_node = next,
			.depth = depth + 1,
		};
		dijkstra_q.push(next_cost);
		nodes_enqueued.insert(child);
	}
	delete max_node;
}


template <class T>
const T&
dag_implicit<T>::dijkstra_iterator::operator *()
{
	return *(dijkstra_q.top().destination_node);
}


template <class T>
bool
dag_implicit<T>::dijkstra_iterator::operator ==(const dijkstra_iterator& x)
{
	return dijkstra_q.empty() && x.complete();
}


template <class T>
bool
dag_implicit<T>::dijkstra_iterator::operator !=(const dijkstra_iterator& x)
{
	return !dijkstra_q.empty() || !x.complete();
}


template <class T>
void
dag_implicit<T>::dijkstra_iterator::set_start(T start_node)
{
	T* first_node;

	while (!dijkstra_q.empty())
		dijkstra_q.pop();
	nodes_enqueued.clear();

	first_node = new T(start_node);
	dijkstra_q.push((node_cost<T>){
		.total_cost = 0,
		.destination_node = first_node,
		.depth = 0,
	});
	nodes_enqueued.insert(start_node);
}


template <class T>
void
dag_implicit<T>::dijkstra_iterator::set_graph(expander_t expand,
						  edge_cost_t edge_cost)
{
	this->expand = expand;
	this->edge_cost = edge_cost;
}


template <class T>
bool
dag_implicit<T>::dijkstra_iterator::complete(void) const
{
	return dijkstra_q.empty();
}
#endif /* ACYCLIC_ITERABLE */
