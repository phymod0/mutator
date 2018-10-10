#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "loader.h"


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

	while (input_stream >> section) {
		if (section == "Prefixes:")
			load_prefix_frequencies(input_stream);
		else if (section == "Leading characters:")
			load_leadingchar_frequencies(input_stream);
		else if (section == "Normal characters:")
			load_normalchar_frequencies(input_stream);
		else if (section == "Suffixes:")
			load_suffix_frequencies(input_stream);
		else {
			cerr << "Error: Section \"" << section
			     << "\" not supported..." << endl;
			return false;
		}
	}

	return true;
}


bool
frequency_data_loader::load_prefix_frequencies(ifstream input_stream)
{
	string tag;
	int n_items;
	vector<string> items;
	vector<frequency> freqs;

	if (!(input_stream >> tag >> n_items) || tag != "START")
		return false;
	for (int i=0; i<n_items; i++) {
		string prefix;
		frequency freq;
		if (!(input_stream >> prefix))
			return false;
		items.push_back(prefix);
		if (!(input_stream >> freq))
			return false;
		freqs.push_back(freq);
	}
	if (!(input_stream >> line) || line != "END")
		return false;

	prefix_strings = items;
	prefix_frequencies = freqs;
	return true;
}


static frequency
stol(string num)
{
	frequency cap;
	stringstream ss(num);
	ss >> cap;
	return cap;
}


static bool
read_char_freq_line(ifstream input_stream, vector<char>& chars,
		    vector<frequency>& freqs)
{
	int n_pairs;
	string pair;
	if (!(input_stream >> n_pairs))
		return false;
	for (int i=0; i<n_pairs; i++) {
		int delim_idx;
		unsigned char repchar;
		frequency freq;
		if (!(input_stream >> pair))
			return false;
		if ((delim_idx = pair.find(':')) < 0)
			return false;
		repchar = (unsigned char)(int)stoi(pair.substr(0, delim_idx));
		repfreq = (frequency)stol(pair.substr(delim_idx + 1));
		chars.push_back((char)repchar);
		freqs.push_back(repfreq);
	}

	return true;
}


bool
frequency_data_loader::load_leadingchar_frequencies(ifstream input_stream)
{
	string tag;
	int n_items;

	if (!(input_stream >> tag >> n_items))
		return false;
	if (tag != "START" || n_items != 256)
		return false;

	for (int i=0; i<n_items; i++) {
		vector<char> chars;
		vector<frequency> freqs;
		read_char_freq_line(input_stream, chars, freqs);
		leading_replacement_chars[i] = chars;
		leading_replacement_frequencies[i] = freqs;
	}
	if (!(input_stream >> tag) || tag != "END")
		return false;

	return true;
}


bool
frequency_data_loader::load_normalchar_frequencies(ifstream input_stream)
{
	string tag;
	int n_items;

	if (!(input_stream >> tag >> n_items))
		return false;
	if (tag != "START" || n_items != 256)
		return false;

	for (int i=0; i<n_items; i++) {
		vector<char> chars;
		vector<frequency> freqs;
		read_char_freq_line(input_stream, chars, freqs);
		normal_replacement_chars[i] = chars;
		normal_replacement_frequencies[i] = freqs;
	}
	if (!(input_stream >> tag) || tag != "END")
		return false;

	return true;
}


bool
frequency_data_loader::load_suffix_frequencies(ifstream input_stream)
{
	string tag;
	int n_items;
	vector<string> items;
	vector<frequency> freqs;

	if (!(input_stream >> tag >> n_items) || tag != "START")
		return false;
	for (int i=0; i<n_items; i++) {
		string suffix;
		frequency freq;
		if (!(input_stream >> suffix))
			return false;
		items.push_back(suffix);
		if (!(input_stream >> freq))
			return false;
		freqs.push_back(freq);
	}
	if (!(input_stream >> line) || line != "END")
		return false;

	suffix_strings = items;
	suffix_frequencies = freqs;
	return true;
}
