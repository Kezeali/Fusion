#include "mersenne.h"
#include <cmath>

const double PI = 3.14159265358979323846264338327950288419716939937510;
const double sqrt2pi = sqrt(2*PI);

RStream::RStream( const unsigned int SEED ) {
	state[0] = SEED;

	for (int i=1; i<624; i++) {
		// Multiply the previous value, add 1 in case of 0, and use the lower
		// 32 bits.
		state[i] = (0x10dcd * state[i-1] +1)&0xffffffff;
	}
	randomize();
	index=0;
}

// Creates a list of 624 randomized (untempered) values
void RStream::randomize() {
	for (int i=0; i<623; i++) {
		// The highest bit of state[i] + the lower 31 bits of state[i+1]
		unsigned int q = state[i]&0x80000000 + state[i+1]&0x7fffffff;

		// The state is the state of i + the offset (397), bitwise
		// xor'd with (y right shifted by 1 bit)
		unsigned int w = state[(i+397)%624]^(q>>1);
		if (q&0x1 == 0) {// If q is even
			state[i] = w;
		} else { // q is odd
			state[i] = w^0x9908b0df;
		}
	}
	unsigned int q = state[623]&0x80000000 + state[0]&0x7fffffff;
	unsigned int w = state[396]^(q >> 1);
	if (q&0x1 == 0) {
		state[623]=w;
	} else {
		state[623]=w^0x9908b0df;
	}
}

// Extracts a (tempered) random number from the list
unsigned int RStream::rand() {
	unsigned int q=state[index];
	index++;
	if (index > 623) {
		index=0;
		randomize();
	}
	return q;
}

bool RStream::even() {
	unsigned int q=rand();
	if (rand()&0x1 == 1) {
		return true;
	} else {
		return false;
	}
}

float RStream::normalized() {
	return float(rand())/float(0xffffffff);
}

float RStream::ranged(const float min, const float max) {
	float range = max-min;
	return min+range*normalized();
}

const float l2 = 1.0f/log(2.0f);

float RStream::distributed(const float mean, const float deviation) {
	// The function is simple... for X deviations, and a probability P that the
	// result will lie X deviations from the mean, P=1-(1/2^x)
	// Solving this for X, we have X=log(1/(1-P))/log(2)
	// Of course, since P lies between 0..1, we can just use X=log(1/P)/log(2)

	float x=log(1/normalized())*l2;

	if (even())
		return mean + x*deviation;
	return mean - x*deviation;
}
