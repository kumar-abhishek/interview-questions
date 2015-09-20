/* Given a tree where each node has an arbitrary number of children, write a
 * program that prints the first and last node of each level in the tree.
 *
 * If a level has only one node, that node is considered to be both the first
 * and the last, so it should be printed twice.
 *
 * EXAMPLE
 *
 * Consider the following tree:
 *
 *                   1
 *                 /   \
 *                /     \
 *               2       3
 *                \     / \
 *                 4   5   6
 *                    /
 *                   7
 *                    \
 *                     8
 *
 * For this tree, the output is:
 *
 * 1 1
 * 2 3
 * 4 6
 * 7 7
 * 8 8
 *
 * Note that this example shows a binary tree, but the problem deals with n-ary
 * trees, so each node can have 0 or more children.
 *
 * Source: Careercup (Amazon interview)
 *
 */

#include <iostream>
#include <vector>
#include <queue>
#include <sstream>
#include <cassert>

using namespace std;

struct tree_node {
	int val;
	vector<tree_node *> children;
	tree_node(int v) : val(v) { }
};

void first_last_level(tree_node *root) {
	queue<tree_node *> nodes_q;
	nodes_q.push(root);

	queue<tree_node*>::size_type curr_level = 1;
	queue<tree_node*>::size_type next_level;

	while (!nodes_q.empty()) {
		tree_node *first = nodes_q.front();
		tree_node *last = NULL;

		next_level = 0;

		for (queue<tree_node*>::size_type i = 0; i < curr_level; i++) {
			tree_node *n = nodes_q.front();
			if (i+1 == curr_level)
				last = n;
			for (vector<tree_node*>::iterator child_it = n->children.begin(); child_it != n->children.end(); child_it++) {
				nodes_q.push(*child_it);
				next_level++;
			}

			nodes_q.pop();
		}

		assert(last != NULL);

		cout << first->val << "\t" << last->val << endl;
		curr_level = next_level;
	}
}

tree_node *deserialize(const string &tree_str) {
	if (tree_str == "")
		return NULL;

	queue<tree_node*> queue;
	istringstream iss(tree_str);
	int value;
	iss >> value;

	tree_node *root = new tree_node(value);
	queue.push(root);

	while (!queue.empty()) {
		tree_node *curr = queue.front();
		queue.pop();
		int count;
		iss >> count;
		for (int i = 0; i < count; i++) {
			iss >> value;
			tree_node *new_node = new tree_node(value);
			curr->children.push_back(new_node);
			queue.push(new_node);
		}
	}

	return root;
}

static void destroy_tree(tree_node *root) {
	if (root == NULL)
		return;
	for (vector<tree_node*>::iterator it = root->children.begin();
	     it != root->children.end();
	     it++)
		destroy_tree(*it);
	delete root;
}

int main(void) {
	cout << "Enter a tree level by level, indicating the children count for each node before listing it." << endl;
	cout << "See trees_graphs/serialize_ntree to learn more about the input format" << endl;
	cout << "> ";

	string line;
	while (getline(cin, line)) {
		tree_node *root = deserialize(line);
		first_last_level(root);
		destroy_tree(root);
		cout << "> ";
	}

	return 0;
}
