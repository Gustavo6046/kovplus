#include "kovplus.hpp"

#include <iostream>
#include <fstream>


int main(int argc, char **argv) {
	KovPlusChain my_chain(10);

	int line_count = 0;

	// parse passed files
	for (int i = 0; i < argc; i++) {
		char *arg = argv[i];

		std::ifstream my_file;
		my_file.open(arg, std::ios::in);

		for (std::string line; std::getline(my_file, line);) {
			my_chain.add_sentence(line);
			line_count++;

			if (line_count < 200 || line_count % 3 == 0) {
				std::cerr << "Parsed lines: " << line_count << "\r";
			}
		}
	}

	if (argc) {
		std::cerr << "Parsed " << line_count << " lines, now operating on stdin and stdout.\n";
	}

	// main query-respond loop
	std::default_random_engine rng;

	std::cerr << "> ";

	for (std::string line; std::getline(std::cin, line);) {
		if (line == "") {
			std::cerr << "> ";
			continue;
		}
	
		my_chain.add_sentence(line);

		KovPlusQuery query(my_chain, line, ' ', &rng);
		
		query.make_up_to(query.size() + 75);

		std::cerr << ": ";
		std::cout << query.str() << "\n";
		std::cerr << "> ";
	}

	return 0;
}
