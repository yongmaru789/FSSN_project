#include <zmq.hpp>
#include <iostream>
using namespace std;
#include <string>

int main() {
	zmq::context_t context(1);

	zmq::socket_t publisher(context, zmq::socket_type::pub);
	publisher.bind("tcp://*:5557");

	zmq::socket_t collector(context, zmq::socket_type::pull);
	collector.bind("tcp://*:5558");

	while (true) {
		zmq::message_t message;
		collector.recv(message, zmq::recv_flags::none);

		string msg_str(static_cast<char*>(message.data()), message.size());
		cout << "I: publishing update " << msg_str << endl;
		publisher.send(message, zmq::send_flags::none);
	}
	
	return 0;
}