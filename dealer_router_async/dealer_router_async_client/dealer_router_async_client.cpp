#include <iostream>
using namespace std;
#include <zmq.hpp>
#include <string>
#include <thread>
#include <chrono>

class ClientTask {
public:
	ClientTask(const string& id) : id(id) {}

	void run() {
		zmq::context_t context(1);
		zmq::socket_t socket(context, zmq::socket_type::dealer);

		string identity = string("client-") + id;
		socket.set(zmq::sockopt::routing_id, identity);
		socket.connect("tcp://localhost:5570");

		cout << "Client " << identity << " started" << endl;

		int reqs = 0;

		while (true) {
			reqs = reqs + 1;
			cout << "Req #" << reqs << " sent.." << endl;

			string request = string("request #") + to_string(reqs);
			socket.send(zmq::buffer(request), zmq::send_flags::none);

			this_thread::sleep_for(chrono::seconds(1));

			zmq::pollitem_t items[] = {
				{ static_cast<void*>(socket.handle()), 0, ZMQ_POLLIN, 0 }
			};
			zmq::poll(items, 1, 1000);

			if (items[0].revents & ZMQ_POLLIN) {
				zmq::message_t msg;
				socket.recv(msg, zmq::recv_flags::none);
				string msg_str(static_cast<char*>(msg.data()), msg.size());
				cout << identity << " received: " << msg_str << endl;
			}
		}
	}
private:
	string id;
};

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "usage :" << argv[0] << " client_id" << endl;
		return 0;
	}
	string client_id = argv[1];
	ClientTask client(client_id);
	client.run();

	return 0;
}