#include <iostream>
using namespace std;

#include <zmq.hpp>
#include <string>
#include <cstdlib>
#include <ctime>

int main() {
	cout << "Publishing updates at weather server..." << endl;

	zmq::context_t context(1);
	zmq::socket_t socket(context, zmq::socket_type::pub);
	socket.bind("tcp://*:5566");
	srand(static_cast<unsigned int>(time(nullptr)));

	while (true) {
		int zipcode = 1 + rand() % 100000;
		int temperature = -80 + rand() % (135 - (-80) + 1);
		int relhumidity = 10 + rand() % (60 - 10 + 1);

		string update = to_string(zipcode) + " " + to_string(temperature) + " " + to_string(relhumidity);

		zmq::message_t message(update.size());
		memcpy(message.data(), update.data(), update.size());
		socket.send(message, zmq::send_flags::none);
	}

	return 0;
}