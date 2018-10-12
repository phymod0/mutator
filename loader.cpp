#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include "loader.hpp"


using namespace std;


void
frequency_data_loader::get_prefix_frequencies(vector<string>& prefixes,
					      vector<frequency>& freqs)
{
	prefixes = this->prefix_strings;
	freqs = this->prefix_frequencies;
}


void
frequency_data_loader::get_leadingchar_frequencies(unsigned char leadingchar,
						   vector<char>& leadingchars,
						   vector<frequency>& freqs)
{
	leadingchars = this->leading_replacement_chars[leadingchar];
	freqs = this->leading_replacement_frequencies[leadingchar];
}


void
frequency_data_loader::get_normalchar_frequencies(unsigned char normalchar,
						  vector<char>& normalchars,
						  vector<frequency>& freqs)
{
	normalchars = this->normal_replacement_chars[normalchar];
	freqs = this->normal_replacement_frequencies[normalchar];
}


void
frequency_data_loader::get_suffix_frequencies(vector<string>& suffixes,
					      vector<frequency>& freqs)
{
	suffixes = this->suffix_strings;
	freqs = this->suffix_frequencies;
}


bool
frequency_data_loader::load_frequency_file(string freqfile_path)
{
	string section;
	ifstream input_stream(freqfile_path);
	if (!input_stream.is_open()) {
		cerr << "Failed to open file \"" << freqfile_path << "\"\n";
		return false;
	}

	bool ret = true;
	while (input_stream >> section) {
		if (section == ":prefix:")
			ret = ret && load_prefix_frequencies(input_stream);
		else if (section == ":leading:")
			ret = ret && load_leadingchar_frequencies(input_stream);
		else if (section == ":normal:")
			ret = ret && load_normalchar_frequencies(input_stream);
		else if (section == ":suffix:")
			ret = ret && load_suffix_frequencies(input_stream);
		else {
			cerr << "Error: Section \"" << section
			     << "\" not supported..." << endl;
			return false;
		}
	}

	return ret;
}


bool
frequency_data_loader::load_prefix_frequencies(ifstream& input_stream)
{
	string tag;
	int n_items;
	vector<string> items;
	vector<frequency> freqs;

	if (!(input_stream >> tag >> n_items) || tag != "START") {
		cerr << __func__ << ": Corrupt start tag...\n";
		return false;
	}
	for (int i=0; i<n_items; i++) {
		string prefix;
		frequency freq;
		if (!(input_stream >> prefix)) {
			cerr << __func__ << ": Corrupt entry prefix...\n";
			return false;
		}
		items.push_back(prefix.substr(1));
		if (!(input_stream >> freq)) {
			cerr << __func__ << ": Corrupt entry frequency...\n";
			return false;
		}
		freqs.push_back(freq);
	}
	if (!(input_stream >> tag) || tag != "END") {
		cerr << __func__ << ": Corrupt end tag...\n";
		return false;
	}

	prefix_strings = items;
	prefix_frequencies = freqs;
	return true;
}


#if 0
static frequency
stol(string num)
{
	frequency cap;
	stringstream ss(num);
	ss >> cap;
	return cap;
}
#endif


static bool
read_char_freq_line(ifstream& input_stream, vector<char>& chars,
		    vector<frequency>& freqs)
{
	int n_pairs;
	if (!(input_stream >> n_pairs)) {
		cerr << __func__ << ": Corrupt entry...\n";
		return false;
	}
	for (int i=0; i<n_pairs; i++) {
		string pair;
		int delim_idx;
		unsigned char repchar;
		frequency repfreq;
		if (!(input_stream >> pair)) {
			cerr << __func__ << ": Bad entry count...\n";
			return false;
		}
		if ((delim_idx = pair.find(':')) < 0) {
			cerr << __func__ << ": Corrupt entry...\n";
			return false;
		}
		repchar = (unsigned char)(int)stoi(pair.substr(0, delim_idx));
		repfreq = (frequency)stol(pair.substr(delim_idx + 1));
		chars.push_back((char)repchar);
		freqs.push_back(repfreq);
	}

	return true;
}


bool
frequency_data_loader::load_leadingchar_frequencies(ifstream& input_stream)
{
	string tag;
	int n_items;

	if (!(input_stream >> tag >> n_items)) {
		cerr << __func__ << ": Corrupt start tag...\n";
		return false;
	}
	if (tag != "START" || n_items != 256) {
		cerr << __func__ << ": Corrupt start tag...\n";
		return false;
	}

	for (int i=0; i<n_items; i++) {
		vector<char> chars;
		vector<frequency> freqs;
		read_char_freq_line(input_stream, chars, freqs);
		leading_replacement_chars[i] = chars;
		leading_replacement_frequencies[i] = freqs;
	}
	if (!(input_stream >> tag) || tag != "END") {
		cerr << __func__ << ": Corrupt end tag...\n";
		return false;
	}

	return true;
}


bool
frequency_data_loader::load_normalchar_frequencies(ifstream& input_stream)
{
	string tag;
	int n_items;

	if (!(input_stream >> tag >> n_items)) {
		cerr << __func__ << ": Corrupt start tag...\n";
		return false;
	}
	if (tag != "START" || n_items != 256) {
		cerr << __func__ << ": Corrupt start tag...\n";
		return false;
	}

	for (int i=0; i<n_items; i++) {
		vector<char> chars;
		vector<frequency> freqs;
		read_char_freq_line(input_stream, chars, freqs);
		normal_replacement_chars[i] = chars;
		normal_replacement_frequencies[i] = freqs;
	}
	if (!(input_stream >> tag) || tag != "END") {
		cerr << __func__ << ": Corrupt end tag...\n";
		return false;
	}

	return true;
}


bool
frequency_data_loader::load_suffix_frequencies(ifstream& input_stream)
{
	string tag;
	int n_items;
	vector<string> items;
	vector<frequency> freqs;

	if (!(input_stream >> tag >> n_items) || tag != "START") {
		cerr << __func__ << ": Corrupt start tag...\n";
		return false;
	}
	for (int i=0; i<n_items; i++) {
		string suffix;
		frequency freq;
		if (!(input_stream >> suffix)) {
			cerr << __func__ << ": Corrupt entry prefix...\n";
			return false;
		}
		items.push_back(suffix.substr(1));
		if (!(input_stream >> freq)) {
			cerr << __func__ << ": Corrupt entry frequency...\n";
			return false;
		}
		freqs.push_back(freq);
	}
	if (!(input_stream >> tag) || tag != "END") {
		cerr << __func__ << ": Corrupt end tag...\n";
		return false;
	}

	this->suffix_strings = items;
	this->suffix_frequencies = freqs;
	return true;
}
