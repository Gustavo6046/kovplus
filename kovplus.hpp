#ifndef INCLUDED_KOVPLUS_HPP
#define INCLUDED_KOVPLUS_HPP

#include <vector>
#include <unordered_map>
#include <sstream>
#include <string>


class WordBag {
private:
	std::vector<std::string> token_list;
	std::unordered_map<std::string, int> token_index;

public:
	WordBag();

	const std::string &get(int index) const;
	int get(std::string token) const;

	int add(std::string token);
};

class SentenceView;
class SentenceCursor;

class Sentence {
private:
	WordBag &bag;
	std::vector<int> tokens;
	int my_size;
	const char separator;

public:
	Sentence(WordBag &bag, std::vector<int> tokens, const char separator = ' ');
	Sentence(WordBag &bag, const std::string tokens, const char separator = ' ');

	WordBag &get_bag() const {
		return bag;
	}
	
	int size() const {
		return my_size;
	}

	const std::string &token(int index) const {
		return bag.get(tokens[index]);
	}
	
	int token_id(int index) const {
		return tokens[index];
	}

	void set_token_id(int index, int token) {
		tokens[index] = token;
	}

	void set_token(int index, std::string token) {
		tokens[index] = bag.add(token);
	}
	
	std::string str() const;

	void append(int index);
	void append(std::string token);

	SentenceView view();
	SentenceView slice(int start = 0, int end = -1);
	SentenceCursor iterator(int start = 0, int end = -1);
};

class SentenceView {
private:
	Sentence &sentence;
	int start;
	int end;

public:
	SentenceView(Sentence &sentence, int start, int end) : sentence(sentence), start(start), end(end) {}
	
	const std::string &token(int index) const {
		return sentence.token(index + start);
	}
	
	int token_id(int index) const {
		return sentence.token_id(index + start);
	}

	void set_token_id(int index, int token) {
		sentence.set_token_id(index + start, token);
	}

	void set_token(int index, std::string token) {
		sentence.set_token(index + start, token);
	}
	
	int size() const {
		return end - start;
	}

	WordBag &get_bag() const {
		return sentence.get_bag();
	}

	std::string str() const;

	SentenceCursor iterator(int start = 0, int end = -1);
};

class SentenceCursor {
private:
	SentenceView view;
	int curr;
	int end;

	int curr_id;
	const std::string *curr_token;

	void update_self();

public:
	SentenceCursor(SentenceView view, int start = 0, int end = -1) : view(view), curr(start), end(end) {
		update_self();
	}

	int id() const {
		return curr_id;
	}
	
	const std::string &token() const {
		return *curr_token;
	}

	void set_token(std::string token) {
		view.set_token(curr, token);
	}
	
	bool has() const;
	void next();
};

class AttentionAssessor {
private:
	WordBag &bag;
	SentenceView expected;
	double strength;
	size_t width;

public:
	AttentionAssessor(WordBag &bag, SentenceView &expected, double strength = 1.0);
	double assess(SentenceView &tokens);
};

class ResponseLog {
private:
	WordBag &bag;
	std::vector<int> query;
	std::vector<int> response;

public:
	ResponseLog(std::vector<int> query, std::string response);
	AttentionAssessor getAssessor(int index);
};


#endif // INCLUDED_KOVPLUS_HPP