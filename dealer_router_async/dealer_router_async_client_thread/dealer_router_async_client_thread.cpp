#include <iostream>
using namespace std;
#include <zmq.hpp>
#include <string>
#include <thread>
#include <chrono>

class ClientTask {
public:
	ClientTask(const string& id) : id(id), context(1), socket(context, zmq::socket_type::dealer) {}
	
	void run() {
		identity = string("client-") + id;
		socket.set(zmq::sockopt::routing_id, identity);
		socket.connect("tcp://localhost:5570");

		cout << "Client " << identity << " started" << endl;

		thread recv_thread(&ClientTask::recvHandler, this);
		recv_thread.detach();

		int reqs = 0;

		while (true) {
			reqs = reqs + 1;
			cout << "Req #" << reqs << " sent.." << endl;

			string request = string("request #") + to_string(reqs);
			socket.send(zmq::buffer(request), zmq::send_flags::none);

			this_thread::sleep_for(chrono::seconds(1));
		}
	}
private:
	void recvHandler() {
		while (true) {
			zmq::pollitem_t items[] = {
				{static_cast<void*>(socket.handle()), 0, ZMQ_POLLIN, 0}
			};

			zmq::poll(items, 1, chrono::milliseconds(1000));

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
	zmq::context_t context;
	zmq::socket_t socket;
	string identity;
};

int main(int argc, char* argv[]) {
	string client_id = "default";
	if (argc >= 2 && argv[1] != nullptr) {
		client_id = argv[1];
	}

	ClientTask client(client_id);
	client.run();

	return 0;
}