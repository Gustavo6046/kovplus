#include "kovplus.hpp"

#include <iostream>


int main() {
	KovPlusChain my_chain(10);

	std::default_random_engine rng;

	for (std::string line; std::getline(std::cin, line);) {
		my_chain.add_sentence(line);

		KovPlusQuery query(my_chain, line, ' ', &rng);
		
		query.make_up_to(50);

		std::cout << query.str() << std::endl;
	}

	return 0;
}
