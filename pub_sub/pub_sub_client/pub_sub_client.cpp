#include <iostream>
using namespace std;

#include <zmq.hpp>
#include <string>
#include <sstream>

int main(int argc, char* argv[]) {
	zmq::context_t context(1);
	zmq::socket_t socket(context, zmq::socket_type::sub);
	
	cout << "Collecting updates from weather server..." << endl;
	
	socket.connect("tcp://localhost:5566");

	string zip_filter;
	if (argc > 1) {
		zip_filter = string(argv[1]);
	} else {
		zip_filter = "10001";
	}

	socket.set(zmq::sockopt::subscribe, zip_filter);
	long total_temp = 0;
	int update_nbr = 0;

	for (update_nbr = 0; update_nbr < 20; update_nbr++) {
		zmq::message_t message;
		socket.recv(message);

		string received(
			static_cast<char*>(message.data()), message.size()
		);
		istringstream iss(received);
		string zipcode;
		int temperature;
		int relhumidity;

		iss >> zipcode >> temperature >> relhumidity;

		total_temp += temperature;

		cout << "Receive temperature for zipcode " << zip_filter << " was " << temperature << " F" << endl;

		double average = static_cast<double>(total_temp) / update_nbr;

		cout << "Average temperature for zipcode " << zip_filter << " was " << average << " F" << endl;
	}

	return 0;
}