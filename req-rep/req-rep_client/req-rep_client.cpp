#include <iostream>
using namespace std;

#include <string>
#include <zmq.hpp>

int main() {
	zmq::context_t context(1);

	cout << "Connecting to hello world server..." << endl;
	zmq::socket_t socket(context, zmq::socket_type::req);
	socket.connect("tcp://localhost:5555");

	for (int request = 0; request < 10; ++request) {
		cout << "Sending request " << request << " ... " << endl;

		const std::string hello = "Hello";
		socket.send(zmq::buffer(hello), zmq::send_flags::none);

		zmq::message_t reply;
		socket.recv(reply, zmq::recv_flags::none);

		string reply_str(static_cast<char*>(reply.data()), reply.size());
		cout << "Received reply " << request << " [ " << reply_str << " ]" << endl;
	}

	return 0;
}