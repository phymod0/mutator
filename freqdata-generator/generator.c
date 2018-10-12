#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "avl.h"

#define ALLOC(x, n) (x = malloc((n) * sizeof(*(x))))
#define STATIC_ARRLEN(array) (sizeof(array)/sizeof(array[0]))
#define ENTER_FUNCTION_CB() \
	// fprintf(stderr, "At function %s...\n", __func__);
/*
 * Minimum match percentage to consider the current password as a derivative of
 * the word we are matching passwords with
 */
#define WORDMATCH_CRITERION 0.8


struct table_entry {
	char character;
	unsigned long long frequency;
};

struct table_row {
	int n_entries;
	struct table_entry entries[256];
};

struct table {
	struct table_row rows[256];
};


static struct table *table_create_empty(void)
{
	ENTER_FUNCTION_CB();
	int i;
	struct table *table;

	if (!ALLOC(table, 1))
		goto oom;
	for (i=0x00; i<=0xFF; i++)
		// Number of entries is dynamic, default to 0
		table->rows[i].n_entries = 0;

	return table;
 oom:
	fprintf(stderr, "%s: Out of memory...\n", __func__);
	return NULL;
}


static void table_new_add_cb(struct table *table, unsigned char original,
			     unsigned char replacement, int frequency)
{
	ENTER_FUNCTION_CB();
	int i;
	struct table_row *row = &table->rows[original];
	for (i=0; i<row->n_entries; i++)
		if (row->entries[i].character == replacement) {
			row->entries[i].frequency += frequency;
			return;
		}
	// Replacement character was not in the row at this point
	row->entries[row->n_entries++] = (struct table_entry){
		.frequency = frequency,
		.character = replacement,
	};
}


static void table_dump_to_file(struct table *table, FILE *fd)
{
	ENTER_FUNCTION_CB();
	int i;
	for (i=0x00; i<=0xFF; i++) {
		int j;
		char *space = "";
		struct table_row row = table->rows[i];
		fprintf(fd, "%d", row.n_entries);
		for (j=0; j<row.n_entries; j++) {
			struct table_entry entry = row.entries[j];
			fprintf(fd, " %u:%llu", (unsigned int)entry.character,
				entry.frequency);
		}
		fprintf(fd, "\n");
	}
}


static inline int is_whitespace(char c)
{
	ENTER_FUNCTION_CB();
	int i;
	char whitespaces[] = " \t\n\0";
	for (i=0; i<STATIC_ARRLEN(whitespaces); i++)
		if (c == whitespaces[i])
			return 1;
	return 0;
}


static int get_next_entry_idx(int prev_idx, char *text)
{
	ENTER_FUNCTION_CB();
	int i;
	for (i=prev_idx; text[i]; i++)
		if (is_whitespace(text[i]) && !is_whitespace(text[i+1]))
			return i+1;
	return -1;
}


static void table_read_from_file(struct table *table, FILE *fd)
{
	ENTER_FUNCTION_CB();
	char *linebuf;
	unsigned char i = 0;

	if (!ALLOC(linebuf, 65536)) {
		fprintf(stderr, "%s: Out of memory...\n", __func__);
		return;
	}

	while (fgets(linebuf, 65536, fd)) {
		int entry_idx = 0, n = 0;
		while (linebuf[0] != '\0') {
			unsigned int character;
			unsigned long long frequency;
			sscanf(&linebuf[entry_idx], "%llu:%u", &frequency,
			       &character);
			table->rows[i].entries[n].frequency = frequency;
			table->rows[i].entries[n].character = character;
			entry_idx = get_next_entry_idx(entry_idx, linebuf);
			if (entry_idx < 0)
				break;
			n += 1;
		}
		i += 1;
	}
	if (i != '\0')
		fprintf(stderr, "WARNING: Read less than 256 lines...\n");

	free(linebuf);
}


static void update_prefix_tree(char *word, char *pwd, int match_offset,
			       struct avl_tree *prefix_tree)
{
	ENTER_FUNCTION_CB();
	struct avl_node *prefix_node;
	char prefix[1024];

	if (match_offset >= sizeof prefix)
		// Potential buffer overflow
		return;
	strncpy(prefix, pwd, match_offset);
	prefix[match_offset] = '\0';
#ifdef IGNORE_IDENTITY_PREFIX
	if (prefix[0] == '\0')
		return;
#endif /* IGNORE_IDENTITY_PREFIX */
	if (!(prefix_node = avl_get_node(prefix_tree, prefix))) {
		// Not found
		char *prefix_insertion = strdup(prefix);
		if (!prefix_insertion)
			goto oom;
		avl_insert(prefix_tree, prefix_insertion, 1);
	} else
		// Found
		prefix_node->value += 1;

	return;
 oom:
	fprintf(stderr, "%s: Out of memory...\n", __func__);
	return;
}


static void update_suffix_tree(char *word, char *pwd, int match_offset,
			       struct avl_tree *suffix_tree)
{
	ENTER_FUNCTION_CB();
	struct avl_node *suffix_node;
	char *suffix = pwd + match_offset + strlen(word);

#ifdef IGNORE_IDENTITY_SUFFIX
	if (suffix[0] == '\0')
		return;
#endif /* IGNORE_IDENTITY_SUFFIX */
	if (!(suffix_node = avl_get_node(suffix_tree, suffix))) {
		// Not found
		char *suffix_insertion = strdup(suffix);
		if (!suffix_insertion)
			goto oom;
		avl_insert(suffix_tree, suffix_insertion, 1);
	} else
		// Found
		suffix_node->value += 1;

	return;
 oom:
	fprintf(stderr, "%s: Out of memory...\n", __func__);
	return;
}


static void update_leading_replacement_table(char *word, char *pwd,
					     int match_offset,
					     struct table *replacement_table)
{
	ENTER_FUNCTION_CB();
	pwd += match_offset;
#ifdef IGNORE_IDENTITY_REPLACMENT
	if (*word == *pwd)
		return;
#endif /* IGNORE_IDENTITY_REPLACMENT */
	table_new_add_cb(replacement_table, *word, *pwd, 1);
}


static void update_normal_replacement_table(char *word, char *pwd,
					    int match_offset,
					    struct table *replacement_table)
{
	ENTER_FUNCTION_CB();
	int i;
	char *matched_pwd = &pwd[match_offset];

	if (word[0] == '\0')
		return;
	for (i=1; word[i]; i++) {
		char original = word[i];
		char replacement = matched_pwd[i];
#ifdef IGNORE_IDENTITY_REPLACMENT
		if (original == replacement)
			continue;
#endif /* IGNORE_IDENTITY_REPLACMENT */
		table_new_add_cb(replacement_table, original, replacement, 1);
	}
}


static void update_map_data(char *word, char *pwd, int match_offset,
			    struct avl_tree *prefix_tree,
			    struct avl_tree *suffix_tree,
			    struct table *leading_replacement_table,
			    struct table *normal_replacement_table)
{
	ENTER_FUNCTION_CB();
	update_prefix_tree(word, pwd, match_offset, prefix_tree);
	update_suffix_tree(word, pwd, match_offset, suffix_tree);
	update_leading_replacement_table(word, pwd, match_offset,
					 leading_replacement_table);
	update_normal_replacement_table(word, pwd, match_offset,
					normal_replacement_table);
}


static double match_index(char *word, char *pwd)
{
	ENTER_FUNCTION_CB();
	int i;
	double matching = 0.0, non_matching = 0.0;
	for (i=0; word[i]; i++)
		tolower(word[i]) == tolower(pwd[i]) ?
			(matching += 1.0) : (non_matching += 1.0);
	return matching / (matching + non_matching);
}


static double best_match(char *word, char *pwd, int *index)
{
	ENTER_FUNCTION_CB();
	int offset, best_offset;
	double best_match = -1.0;
	int word_len = strlen(word), pwd_len = strlen(pwd);

	for (offset = 0; offset <= pwd_len - word_len; offset++) {
		double current_match = match_index(word, &pwd[offset]);
		if (best_match < current_match) {
			best_match = current_match;
			best_offset = offset;
		}
	}
	*index = best_offset;
	return best_match;
}


static inline void remove_trailing_newline(char *text)
{
	ENTER_FUNCTION_CB();
	int lf_idx = strlen(text) - 1;
	if (text[lf_idx] == '\n')
		text[lf_idx] = '\0';
}


static void accumulate_map_data(char **words, int n_words,
				struct avl_tree *prefix_tree,
				struct avl_tree *suffix_tree,
				struct table *leading_replacement_table,
				struct table *normal_replacement_table)
{
	ENTER_FUNCTION_CB();
	char pwd[1024];
	while (fgets(pwd, sizeof pwd, stdin)) {
		int i, match_offset;
		remove_trailing_newline(pwd);
		for (i=0; i<n_words; i++) {
			char *word = words[i];
			double match = best_match(word, pwd, &match_offset);
			if (match < WORDMATCH_CRITERION)
				continue;
			update_map_data(word, pwd, match_offset, prefix_tree,
					suffix_tree, leading_replacement_table,
					normal_replacement_table);
		}
	}
}


static int dump_avl_node(char *pwd, unsigned long long frequency,
			 void *cb_data)
{
	ENTER_FUNCTION_CB();
	fprintf(stdout, ">%s\n%llu\n", pwd, frequency);
}


static int __return_1(char *key, unsigned long long val, void *cb_data)
{
	return 1;
}


static int avl_tree_size(struct avl_tree *tree)
{
	return avl_traverse(tree, __return_1, NULL);
}


static void dump_map_data(struct avl_tree *prefix_tree,
			  struct avl_tree *suffix_tree,
			  struct table *leading_replacement_table,
			  struct table *normal_replacement_table)
{
	ENTER_FUNCTION_CB();
	fprintf(stdout, ":prefix:\n");
	fprintf(stdout, "START %d\n", avl_tree_size(prefix_tree));
	avl_traverse(prefix_tree, dump_avl_node, NULL);
	fprintf(stdout, "END\n");

	fprintf(stdout, ":suffix:\n");
	fprintf(stdout, "START %d\n", avl_tree_size(suffix_tree));
	avl_traverse(suffix_tree, dump_avl_node, NULL);
	fprintf(stdout, "END\n");

	fprintf(stdout, ":leading:\n");
	fprintf(stdout, "START %d\n", 256);
	table_dump_to_file(leading_replacement_table, stdout);
	fprintf(stdout, "END\n");

	fprintf(stdout, ":normal:\n");
	fprintf(stdout, "START %d\n", 256);
	table_dump_to_file(normal_replacement_table, stdout);
	fprintf(stdout, "END\n");
}


static int __avl_strcmp(char *str_a, char *str_b)
{
	ENTER_FUNCTION_CB();
	return strcmp(str_a, str_b);
}

static void __avl_free(char *ptr)
{
	ENTER_FUNCTION_CB();
	return free(ptr);
}

static void stub(unsigned long long frequency)
{
	ENTER_FUNCTION_CB();
	return;
}


static int get_linecount(char *filename)
{
	ENTER_FUNCTION_CB();
	FILE *fd;
	int linecount = 0;
	char linebuf[1024];

	if (!(fd = fopen(filename, "rb"))) {
		fprintf(stderr, "%s: Failed to open file %s...\n", __func__,
			filename);
		return -1;
	}
	while (fgets(linebuf, sizeof linebuf, fd))
		linecount += 1;

	fclose(fd);
	return linecount;
}


static int gen_map_data(char *filename, struct avl_tree *prefix_tree,
			 struct avl_tree *suffix_tree,
			 struct table *leading_replacement_table,
			 struct table *normal_replacement_table)
{
	ENTER_FUNCTION_CB();
	FILE *fd;
	int linecount, n_words=0;
	char **words, word[1024];

	if ((linecount = get_linecount(filename)) < 0)
		goto badfd;
	if (!(fd = fopen(filename, "rb")))
		goto badfd;
	if (!ALLOC(words, linecount))
		goto oom;

	while (fgets(word, sizeof word, fd) && n_words < linecount) {
		remove_trailing_newline(word);
		if (!(words[n_words++] = strdup(word)))
			goto oom;
	}

	accumulate_map_data(words, n_words, prefix_tree, suffix_tree,
			    leading_replacement_table,
			    normal_replacement_table);
 out:
	while (n_words --> 0)
		free(words[n_words]);
	free(words);
	fclose(fd);
	return 0;
 badfd:
	fprintf(stderr, "%s: Failed to open file %s...\n", __func__, filename);
	return -1;
 oom:
	fprintf(stderr, "%s: Out of memory...\n", __func__);
	fclose(fd);
	return -1;
}


int main(int argc, char *argv[])
{
	ENTER_FUNCTION_CB();
	int res;
	char *wordlist;
	struct avl_tree *prefix_tree, *suffix_tree;
	struct table *leading_replacement_table, *normal_replacement_table;

	if (argc != 2) {
		fprintf(stderr, "Usage: cat <password data> | %s <wordlist to "
			"compare> > <output file>\n", argv[0]);
		return -1;
	}

	wordlist = argv[1];
	if (!(prefix_tree = avl_create_tree(__avl_strcmp, __avl_free, stub)) ||
	    !(suffix_tree = avl_create_tree(__avl_strcmp, __avl_free, stub)) ||
	    !(leading_replacement_table = table_create_empty()) ||
	    !(normal_replacement_table = table_create_empty()))
		goto oom;
	fprintf(stderr, "Generating map data...\n");
	res = gen_map_data(wordlist, prefix_tree, suffix_tree,
			   leading_replacement_table,
			   normal_replacement_table);
	if (res < 0)
		return -1;

	fprintf(stderr, "All recorded:\n");
	dump_map_data(prefix_tree, suffix_tree, leading_replacement_table,
		      normal_replacement_table);

	free(normal_replacement_table);
	free(leading_replacement_table);
	avl_destroy_tree(suffix_tree);
	avl_destroy_tree(prefix_tree);
	return 0;
 oom:
	fprintf(stderr, "%s: Out of memory...\n", __func__);
	return -1;
}
