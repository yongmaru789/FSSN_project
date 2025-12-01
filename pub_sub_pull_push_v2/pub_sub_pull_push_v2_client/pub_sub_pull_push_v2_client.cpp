#include <iostream>
using namespace std;
#include <zmq.hpp>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <cstring>

int main(int argc, char* argv[]) {
    string clientID = argv[1];
    
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, zmq::socket_type::sub);
    subscriber.set(zmq::sockopt::subscribe, "");
    subscriber.connect("tcp://localhost:5557");
    zmq::socket_t publisher(context, zmq::socket_type::push);
    publisher.connect("tcp://localhost:5558");

    srand(static_cast<unsigned int>(time(NULL)));

    while (true) {
        zmq::pollitem_t items[1];
        items[0].socket = static_cast<void*>(subscriber);
        items[0].events = ZMQ_POLLIN;
        items[0].fd = 0;
        items[0].revents = 0;

        zmq::poll(items, 1, chrono::milliseconds(100));

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t message;
            subscriber.recv(message, zmq::recv_flags::none);

            string msg_str(
                static_cast<char*>(message.data()),
                message.size()
            );
            cout << clientID << ": receive status => " << msg_str << endl;
        }
        else {
            int randValue = rand() % 100 + 1;

            if (randValue < 10) {
                this_thread::sleep_for(chrono::seconds(1));

                string msg = "(" + clientID + ":ON)";
                zmq::message_t out_msg(msg.size());
                memcpy(out_msg.data(), msg.data(), msg.size());

                publisher.send(out_msg, zmq::send_flags::none);
                cout << clientID << ": send status - activated" << endl;
            }
            else if (randValue > 90) {
                this_thread::sleep_for(chrono::seconds(1));

                string msg = "(" + clientID + ":OFF)";
                zmq::message_t out_msg(msg.size());
                memcpy(out_msg.data(), msg.data(), msg.size());

                publisher.send(out_msg, zmq::send_flags::none);
                cout << clientID << ": send status - deactivated" << endl;
            }
        }
    }
    return 0;
}