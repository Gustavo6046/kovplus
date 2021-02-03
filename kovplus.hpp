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

	const std::string &get(int index);
	int get(std::string token);

	int add(std::string token);
};

class SentenceView;

class Sentence {
private:
	WordBag &bag;
	std::vector<int> tokens;
	int my_size;
	const char separator;

public:
	Sentence(WordBag &bag, std::vector<int> tokens, const char separator = ' ');
	Sentence(WordBag &bag, const std::string tokens, const char separator = ' ');

	WordBag &get_bag() {
		return bag;
	}
	
	int size() {
		return my_size;
	}

	const std::string &token(int index) {
		return bag.get(tokens[index]);
	}
	
	int token_id(int index) {
		return tokens[index];
	}
	
	std::string str();
	void append(int index);
	void append(std::string token);

	SentenceView view();
	SentenceView slice(int start = 0, int end = -1);
};

class SentenceView {
private:
	Sentence &sentence;
	int start;
	int end;

public:
	SentenceView(Sentence &sentence, int start, int end) : sentence(sentence), start(start), end(end) {}
	
	const std::string &token(int index) {
		return sentence.token(index + start);
	}
	
	int token_id(int index) {
		return sentence.token_id(index + start);
	}
	
	int size() {
		return end - start;
	}

	WordBag &get_bag() {
		return sentence.get_bag();
	}

	std::string str();
};

class SentenceIterator {
private:
	SentenceView &view;
	int curr;

public:
	SentenceIterator(SentenceView &view, int curr) : view(view), curr(curr) {}

	int id() {
		return view.token_id(curr); 
	}
	
	std::string token() {
		return view.token(curr);
	}
	
	bool next() {
		return (curr++) < view.size();
	}
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