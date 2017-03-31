// Compile away asserts() and trie dump() command
#define NDEBUG

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <set>
#include <cassert>
#include <cctype>

using namespace std;

struct trie_node {
	map<char, trie_node*> children;
	set<string> ids;
};

class Trie {
private:
	struct trie_node *root;

	void add_impl(const string &id, const string &word, string::size_type curr, trie_node *node) {

		node->ids.insert(id);

		if (curr == word.size())
			return;

		char letter = tolower(word[curr]);
		trie_node *next;
		map<char, trie_node *>::const_iterator next_it = node->children.find(letter);

		if (next_it == node->children.end()) {
			next = new trie_node;
			node->children[letter] = next;
		} else {
			next = next_it->second;
		}

		add_impl(id, word, curr+1, next);
	}

	void del_impl(const string &id, const string &word, string::size_type curr, trie_node *node) {

		node->ids.erase(id);

		if (curr == word.size())
			return;

		char letter = tolower(word[curr]);
		map<char, trie_node *>::iterator next_it = node->children.find(letter);

		if (next_it == node->children.end())
			return;

		del_impl(id, word, curr+1, next_it->second);

		if (next_it->second->ids.size() == 0) {
			node->children.erase(letter);
			delete next_it->second;
			next_it->second = NULL;
		}
	}

	set<string> *find_impl(const string &word, string::size_type curr, trie_node *node) {

		if (curr == word.size())
			return &node->ids;

		char letter = tolower(word[curr]);
		map<char, trie_node*>::const_iterator next = node->children.find(letter);

		if (next == node->children.end())
			return NULL;

		return find_impl(word, curr+1, next->second);
	}

	void del_all(trie_node *node) {

		if (node->children.size() == 0)
			return;

		for (map<char, trie_node*>::iterator it = node->children.begin();
		     it != node->children.end();
		     it++) {
			del_all(it->second);
			delete it->second;
			it->second = NULL;
		}

		node->children.clear();
	}

#ifndef NDEBUG
	void dump_impl(trie_node *node, int depth) {
		string spaces(depth*2, ' ');
		cout << spaces;
		cout << "(";
		if (node->ids.begin() != node->ids.end()) {
			set<string>::const_iterator it = node->ids.begin();
			cout << *it;
			for (it++; it != node->ids.end(); it++)
				cout << ", " << *it;
		}
		cout << ")" << endl;
		for (map<char, trie_node*>::const_iterator it = node->children.begin();
		     it != node->children.end();
		     it++) {
			cout << spaces << "-> " << it->first << endl;
			dump_impl(it->second, depth+1);
		}
	}
#endif

public:
	Trie() {
		root = new trie_node;
	}

	~Trie() {
		del_all(root);
		delete root;
	}

	void add(const string &id, const string &word) {
		add_impl(id, word, 0, root);
	}

	void del(const string &id, const string &word) {
		del_impl(id, word, 0, root);
	}

	set<string> *find(const string &word) {
		return find_impl(word, 0, root);
	}

#ifndef NDEBUG
	void dump() {
		dump_impl(root, 0);
	}
#endif

};

class Record;
float get_boosted_score(const Record &r, const map<string, float> *id_boosts, const map<string, float> *type_boosts);

class Record {
private:
	string type;
	string id;
	float score;
	unsigned long long add_timestamp;
	vector<string> data;

public:
	Record(const string &ntype, const string &nid, const float nscore, const string &ndata, unsigned long long date) :
		type(ntype), id(nid), score(nscore), add_timestamp(date) {

		istringstream iss(ndata);
		string token;

		while (iss >> token)
			data.push_back(token);
	}

	const vector<string> &get_tokens() const { return data; }
	float get_score() const { return score; }
	const string &get_id() const { return id; }
	const string &get_type() const { return type; }
	const unsigned long long get_timestamp() const { return add_timestamp; }

	bool compare_to(const Record &r, const map<string, float> *id_boosts, const map<string, float> *type_boosts) const {
		float new_score = get_boosted_score(*this, id_boosts, type_boosts);
		float new_r_score = get_boosted_score(r, id_boosts, type_boosts);
		if (new_score > new_r_score)
			return true;
		if (new_score == new_r_score && add_timestamp > r.get_timestamp())
			return true;
		return false;
	}
};


static Trie tokens_store;
static map<string, Record*> records;
static unsigned long long records_clock;

struct RecordComparator {
	map<string, float> *id_boosts;
	map<string, float> *type_boosts;

	RecordComparator(map<string, float> *id_b, map<string, float> *type_b) :
		id_boosts(id_b), type_boosts(type_b) { }

	bool operator()(const string &id_a, const string &id_b) {

		map<string, Record *>::const_iterator it_a = records.find(id_a);
		map<string, Record *>::const_iterator it_b = records.find(id_b);
		assert(it_a != records.end());
		assert(it_b != records.end());
		
		return it_a->second->compare_to(*it_b->second, id_boosts, type_boosts);
	}
};

float get_boosted_score(const Record &r, const map<string, float> *id_boosts, const map<string, float> *type_boosts) {
	float res = r.get_score();

	if (id_boosts != NULL) {
		map<string, float>::const_iterator search_res = id_boosts->find(r.get_id());
		if (search_res != id_boosts->end())
			res *= search_res->second;
	}

	if (type_boosts != NULL) {
		map<string, float>::const_iterator search_res = type_boosts->find(r.get_type());
		if (search_res != type_boosts->end())
			res *= search_res->second;
	}

	return res;
}

void insert_record(const string &type, const string &id, float score, const string &data) {

	Record *new_record = new Record(type, id, score, data, records_clock++);
	assert(records.find(id) == records.end());
	records[id] = new_record;

	const vector<string> &tokens = new_record->get_tokens();
	for (vector<string>::const_iterator it = tokens.begin();
	     it != tokens.end();
	     it++)
		tokens_store.add(id, *it);
}

void delete_record(const string &id) {
	map<string, Record*>::const_iterator del_it = records.find(id);

	//assert(del_it != records.end());
	if (del_it == records.end())
		return;

	Record *to_del = del_it->second;

	const vector<string> &tokens = to_del->get_tokens();
	for (vector<string>::const_iterator it = tokens.begin();
	     it != tokens.end();
	     it++)
		tokens_store.del(id, *it);

	records.erase(id);
	delete to_del;
}

void do_query(unsigned limit, const string &query, vector<string> &results, map<string, float> *id_boosts, map<string, float> *type_boosts) {
	istringstream iss(query);
	string word;
	vector<set<string>*> matches;
	vector<set<string>*>::size_type min_i;
	set<string> *match;

	results.clear();
	iss >> word;
	assert(iss);
	match = tokens_store.find(word);

	if (match == NULL)
		return;

	matches.push_back(match);
	min_i = 0;

	while (iss >> word && (match = tokens_store.find(word)) != NULL) {
		matches.push_back(match);
		if (match->size() < matches[min_i]->size())
			min_i = matches.size()-1;
	}

	if (match == NULL)
		return;

	for (set<string>::const_iterator it = matches[min_i]->begin();
	     it != matches[min_i]->end();
	     it++) {
		vector<set<string>*>::size_type i;
		for (i = 0; i < matches.size(); i++) {
			if (i == min_i)
				continue;
			if (matches[i]->find(*it) == matches[i]->end())
				break;
		}
		if (i == matches.size())
			results.push_back(*it);
	}

	if (results.size() <= limit)
		sort(results.begin(), results.end(), RecordComparator(id_boosts, type_boosts));
        else
		partial_sort(results.begin(), results.begin()+limit, results.end(), RecordComparator(id_boosts, type_boosts));
}

void op_add() {
	string type, id, data;
	float score;

	cin >> type >> id >> score;
	assert(cin);

	getline(cin, data);
	assert(data.size() > 0);

	insert_record(type, id, score, data);
}

void op_del() {
	string id;

	cin >> id;
	assert(cin);

	delete_record(id);
}

void op_query() {
	unsigned limit;
	string query_str;

	cin >> limit;
	assert(cin);

	getline(cin, query_str);
	assert(query_str.size() > 0);

	vector<string> results;
	do_query(limit, query_str, results, NULL, NULL);

	if (results.size() > 0 && limit > 0)
		cout << results[0];
	for (vector<string>::size_type i = 1; i < limit && i < results.size(); i++)
		cout << " " << results[i];

	cout << endl;
}

bool is_type(const string &str);
void op_wquery() {
	unsigned limit, boosts;
	map<string, float> id_boosts;
	map<string, float> type_boosts;

	cin >> limit >> boosts;
	assert(cin);

	while (boosts--) {
		string a_boost;
		cin >> a_boost;
		assert(cin);

		size_t delim = a_boost.find(':', 0);
		assert(delim < a_boost.size()-1);

		string boost_t = a_boost.substr(0, delim);
		istringstream iss(a_boost.substr(delim+1, a_boost.size()-delim-1));
		float boost_val;
		iss >> boost_val;
		assert(iss);

		if (is_type(boost_t))
			type_boosts[boost_t] = boost_val;
		else
			id_boosts[boost_t] = boost_val;		
	}

	string query_str;
	getline(cin, query_str);
	assert(query_str.size() > 0);

	vector<string> results;

	do_query(limit, query_str, results, &id_boosts, &type_boosts);

	if (results.size() > 0 && limit > 0)
		cout << results[0];
	for (vector<string>::size_type i = 1; i < limit && i < results.size(); i++)
		cout << " " << results[i];

	cout << endl;
}

void clear_records() {
	for (map<string, Record*>::const_iterator it = records.begin();
	     it != records.end();
	     it++)
		delete it->second;
	records.clear();
}

bool is_type(const string &str) {
	return str == "user" 
		|| str == "topic"
		|| str == "question"
		|| str == "board";
}

int main() {

	unsigned queries;

	cin >> queries;
	assert(cin);

	while (queries--) {
		string op;
		cin >> op;
		assert(cin);

		if (op == "ADD") {
			op_add();
		} else if (op == "DEL") {
			op_del();
		} else if (op == "QUERY") {
			op_query();
		} else if (op == "WQUERY") {
			op_wquery();
		} 
#ifndef NDEBUG
		else if (op == "DUMP") {
			tokens_store.dump();
		} 
#endif
		else {
			assert(0);
		}
	}

	clear_records();

	return 0;
}
