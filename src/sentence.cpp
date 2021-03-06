#include "sentence.hpp"


// WordBag
WordBag::WordBag() {
	add("");
}

const std::string &WordBag::get(int index) const {
	if (index == -1) {
		return NULL_TOKEN;
	}

	return token_list.at(index);
}

int WordBag::get(std::string token) const {
	return token_index.at(token);
}

int WordBag::add(std::string token) {
	auto found = token_index.find(token);

	if (found != token_index.end()) {
		return found->second;
	}

	int new_index = token_list.size();
	
	token_list.push_back(token);
	token_index[token] = new_index;

	return new_index;
}

// Sentence
Sentence::Sentence(WordBag &bag, std::vector<int> tokens) : bag(bag), tokens(tokens), my_size(tokens.size()) {}

Sentence::Sentence(WordBag &bag, const std::string tokens, const char separator) : bag(bag), my_size(0) {
	std::istringstream token_reader(tokens);
	std::string token;
	std::vector<std::string> token_list;

	while (std::getline(token_reader, token, separator)) {
		this->tokens.push_back(bag.add(token));
		my_size++;
	}
}

std::string Sentence::str() const {
	std::ostringstream stringifier;

	for (int i = 0; i < my_size; i++) {
		stringifier << token(i);

		if (i + 1 < my_size) {
			stringifier << " ";
		}
	}

	return stringifier.str();
}

void Sentence::append(int index) {
	tokens.push_back(index);
	my_size++;
}

void Sentence::append(std::string token) {
	tokens.push_back(bag.add(token));
	my_size++;
}

SentenceView Sentence::view() {
	return SentenceView(*this, 0, my_size);
}

SentenceView Sentence::slice(int start, int end) {
	return SentenceView(*this, start, end);
}

SentenceCursor Sentence::iterator(int start, int end) {
	return SentenceCursor(view(), start, end);
}

// SentenceView
std::string SentenceView::str() const {
	std::ostringstream stringifier;

	int my_size = size();

	for (int i = 0; i < my_size; i++) {
		stringifier << token(i);

		if (i + 1 < my_size) {
			stringifier << " ";
		}
	}

	return stringifier.str();
}

SentenceCursor SentenceView::iterator() {
	return SentenceCursor(*this, 0, end - start);
}

// SentenceCursor
bool SentenceCursor::has() const {
	return curr < view.size() && (end == -1 || curr < end);
}

bool SentenceCursor::has_next() const {
	return (curr + 1) < view.size() && (end == -1 || (curr + 1) < end);
}
	
void SentenceCursor::next() {
	curr++;

	if (has()) {
		update_self();
	}
}

void SentenceCursor::update_self() {
	curr_id = view.token_id(curr);
	curr_token = &view.token(curr);
}
