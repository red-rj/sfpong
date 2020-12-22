#pragma once

namespace pong {
	int random_num(int min, int max);
	bool coin_flip();
}

namespace dice {
	inline int D6() { return pong::random_num(1, 6); }
}
