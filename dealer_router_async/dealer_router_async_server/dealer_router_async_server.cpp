#include <iostream>
using namespace std;
#include <zmq.hpp>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>

class ServerWorker {
public:
	ServerWorker(zmq::context_t &context, int id) : context(context), id(id) {}
	
	void run() {
		zmq::socket_t worker(context, zmq::socket_type::dealer);
		worker.connect("inproc://backend");

		cout << "Worker#" << id << " started" << endl;

		while (true) {
			zmq::message_t ident;
			zmq::message_t msg;

			worker.recv(ident, zmq::recv_flags::none);
			worker.recv(msg, zmq::recv_flags::none);

			string ident_str(static_cast<char*>(ident.data()), ident.size());
			string msg_str(static_cast<char*>(msg.data()), msg.size());
		
			cout << "Worker#" << id << " received " << msg_str << " from " << ident_str << endl;
		
			worker.send(ident, zmq::send_flags::sndmore);
			worker.send(msg, zmq::send_flags::none);
		}

		worker.close();
	}
private:
	zmq::context_t& context;
	int id;
};

class ServerTask {
public: 
	ServerTask(int num_server) : num_server(num_server), context(1) {}

	void run() {
		zmq::socket_t frontend(context, zmq::socket_type::router);
		frontend.bind("tcp://*:5570");

		zmq::socket_t backend(context, zmq::socket_type::dealer);
		backend.bind("inproc://backend");

		for (int i = 0; i < num_server; i++) {
			ServerWorker* w = new ServerWorker(context, i);
			thread t(&ServerWorker::run, w);
			t.detach();
		}

		zmq::proxy(frontend, backend);

		frontend.close();
		backend.close();
		context.close();
	}
private:
	int num_server;
	zmq::context_t context;
	vector<std::thread> worker_threads;
};

int main(int argc, char* argv[]) {
	int num_server = 1;

	if (argc >= 2) {
		int parsed = atoi(argv[1]);
		if (parsed > 0) {
			num_server = parsed;
		}
	}

	ServerTask server(num_server);
	server.run();

	return 0;
}