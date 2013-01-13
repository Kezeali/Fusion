namespace * FusionEngine.Interprocess

struct Test {
	1: i32 uid,
	2: string name,
	3: binary data
}
service Editor {
	void test(1: Test t)
	oneway void stop()
}