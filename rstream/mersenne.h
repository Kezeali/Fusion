class RStream {
public:
	// Constructs a new state with the given seed.
	RStream( const unsigned int SEED=0 );

	unsigned int rand();
	float normalized();
	bool even();
	float ranged(const float min, const float max);
	float distributed(const float mean, const float deviation);
private:
	void randomize();
	unsigned int state[624];

	unsigned int index;
};
