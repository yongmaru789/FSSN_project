#include <zmq.hpp>
#include <iostream>
using namespace std;
#include <string>
#include <random>
#include <chrono>
#include <thread>

int main() {
	zmq::context_t context(1);

	zmq::socket_t subscriber(context, zmq::socket_type::sub);
	subscriber.set(zmq::sockopt::subscribe, "");
	subscriber.connect("tcp://localhost:5557");

	zmq::socket_t publisher(context, zmq::socket_type::push);
	publisher.connect("tcp://localhost:5558");

	random_device rd;
	mt19937 rng(rd());
	uniform_int_distribution<int> dist(1, 100);

	while (true) {
		zmq::pollitem_t items[] = {
			{ static_cast<void*>(subscriber), 0, ZMQ_POLLIN, 0 }
		};
		zmq::poll(items, 1, chrono::milliseconds(100));

		if (items[0].revents & ZMQ_POLLIN) {
			zmq::message_t message;
			subscriber.recv(message, zmq::recv_flags::none);

			string msg_str(static_cast<char*>(message.data()), message.size());
			cout << "I: received message " << msg_str << endl;
		}
		else {
			int rand_val = dist(rng);

			if (rand_val < 10) {
				string msg_str = to_string(rand_val);
				publisher.send(zmq::buffer(msg_str), zmq::send_flags::none);
				cout << "I: sending message " << rand_val << endl;
			}
		}
	}

	return 0;
}