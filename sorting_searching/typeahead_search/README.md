# Typeahead Search

(From the [Quora challenges](http://www.quora.com/challenges#typeahead_search) page)

The search bar at the top of every page on Quora allows you to search the most up-to-date people, topics and questions on our site.

We want to quickly return the most relevant results that match the search query entered into the input text field.  Every time a new user, question or topic is added (or old ones deleted), subsequent queries must reflect those changes immediately.  We currently use a fast in-memory service to handle this.

Input comes into the service as the following commands:

* `ADD <type> <id> <score> <data string that can contain spaces>`: adds the following `<type>` of item (`user | topic | question | board`) with the unique `<id>` string and `<score>` float, corresponding to the `<data string>` that would be used to match queries.
* `DEL <id>`: deletes the item specified by unique identifier `<id>`.
* `QUERY <number of results> <query string that can contain spaces>`: queries for the specified integer number of results (up to 20) that match a given query string.  For a data item to be considered a matching result to a query, each token in the query must be found in the data string as a case-insensitive prefix to any token in the data string. The results are ranked in descending order of score, and we only take the top few results as specified. When there is a tie in the score, newer items (more recently added) should be ranked higher.  If there are no results, just output the empty string on that line.
* `WQUERY <number of results> <number of boosts> (<type>:<boost>)* (<id>:<boost>)* <query string that can contain spaces>`: same as `QUERY`, except that instead of using the raw input score specified by `ADD` for the particular item, the scores are weighted by the optional number of boosting factors.  The boosts are floats that should be multiplied to the raw score, and affect either whole types, or specific items with the given `<id>`s.  If there are multiple boosts applicable, they each are multiplied commutatively to the raw score.


Your task will be to write an equivalent service as a standalone program, with input files that correspond to the queries and updates to the data, and expected output files that correspond to the results obtained for each query.

Input format (read from `STDIN`):
Your program will be given an integer `N` on the first line of `stdin`, followed by `N` lines of the form:

`<command> <command data>`

The input commands are: `ADD | DEL | QUERY | WQUERY`

Types are: `user | topic | question | board`

Ids are alphanumeric strings without spaces or punctuation and will not include the same strings used for types.

Data strings can be any ASCII character, but are delimited by spaces or tabs. We will not be using anything special unprintable characters or `\r` and `\n` in the data string.

Command data for each command is as specified above.  For example:

```
15
ADD user u1 1.0 Adam D’Angelo
ADD user u2 1.0 Adam Black
ADD topic t1 0.8 Adam D’Angelo
ADD question q1 0.5 What does Adam D’Angelo do at Quora?
ADD question q2 0.5 How did Adam D’Angelo learn programming?
QUERY 10 Adam
QUERY 10 Adam D’A
QUERY 10 Adam Cheever
QUERY 10 LEARN how
QUERY 1 lear H
QUERY 0 lea
WQUERY 10 0 Adam D’A
WQUERY 2 1 topic:9.99 Adam D’A
DEL u2
QUERY 2 Adam
```

Output format (write to `STDOUT`):

For each `QUERY` and `WQUERY` command, you should output the following line:

```
<sorted result ids>
```

Where each line contains the `<id>`s in descending score order, up to the required number of results, as specified above according to the `QUERY` and `WQUERY` commands.  If there are no matches, output the empty line.  For example:

```
u2 u1 t1 q2 q1
u1 t1 q2 q1

q2
q2

u1 t1 q2 q1
t1 u1
u1 t1
```

Constraints:

* 0 < `N` < 100000
* 0 < number of `ADD`s < 40000
* 0 < number of `DEL`s < 10000
* 0 < number of `QUERY`s < 20000
* 0 < number of `WQUERY`s < 1000
* 0 < number of boosts < 25
* 0.0 < each score < 100.0
* 0 < data string length < 100 chars

You should aim to have your algorithm be fast enough to solve our largest test inputs in under 5 seconds, or be as close to that as possible.

Notes
In this problem, we are presenting a representative simplified version of our service for the purposes of the contest.
