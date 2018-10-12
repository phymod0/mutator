#ifndef LOADER_H
#define LOADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;


typedef unsigned long long frequency;


class frequency_data_loader {
	private:
	// Vectors of prefixes and their frequencies
	vector<string> prefix_strings;
	vector<frequency> prefix_frequencies;
	// 256 vectors of possible leading character replacements and 256
	// vectors of their corresponding frequencies
	vector<char> leading_replacement_chars[256];
	vector<frequency> leading_replacement_frequencies[256];
	// 256 vectors of possible non-leading character replacements and 256
	// vectors of their corresponding frequencies
	vector<char> normal_replacement_chars[256];
	vector<frequency> normal_replacement_frequencies[256];
	// Vectors of suffixes and their frequencies
	vector<string> suffix_strings;
	vector<frequency> suffix_frequencies;
	// Loaders:
	bool load_prefix_frequencies(ifstream& input_stream);
	bool load_leadingchar_frequencies(ifstream& input_stream);
	bool load_normalchar_frequencies(ifstream& input_stream);
	bool load_suffix_frequencies(ifstream& input_stream);
	public:
	bool load_frequency_file(string freqfile_path);
	void get_prefix_frequencies(vector<string>& prefixes,
				    vector<frequency>& freqs);
	void get_leadingchar_frequencies(unsigned char leadingchar,
					 vector<char>& leadingchars,
					 vector<frequency>& freqs);
	void get_normalchar_frequencies(unsigned char normalchar,
					vector<char>& normalchars,
					vector<frequency>& freqs);
	void get_suffix_frequencies(vector<string>& suffixes,
				    vector<frequency>& freqs);
};


#endif /* LOADER_H */
