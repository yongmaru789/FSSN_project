#include <iostream>
using namespace std;
#include <string>
#include <vector>
#include <thread>
#include <ctime>
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <zmq.hpp>

vector<string> split(const string& s, char c) {
    vector<string> out;
    string tmp;
    for (char ch : s) {
        if (ch == c) {
            out.push_back(tmp);
            tmp = "";
        }
        else tmp += ch;
    }
    out.push_back(tmp);
    return out;
}

string get_local_ip() {

    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);

    connect(sock, (sockaddr*)&addr, sizeof(addr));

    sockaddr_in name;
    int namelen = sizeof(name);
    getsockname(sock, (sockaddr*)&name, &namelen);

    char buffer[32];
    inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));

    closesocket(sock);
    WSACleanup();

    return string(buffer);
}

string search_nameserver(string ip_mask, string local_ip_addr, int port_nameserver) {
    zmq::context_t context(1);
    zmq::socket_t sub(context, zmq::socket_type::sub);

    sub.set(zmq::sockopt::subscribe, "NAMESERVER");

    for (int last = 1; last < 255; last++) {
        string target = "tcp://" + ip_mask + "." + to_string(last) + ":" + to_string(port_nameserver);
        sub.connect(target.c_str());
    }

    sub.set(zmq::sockopt::rcvtimeo, 2000);

    zmq::message_t msg;

    if (!sub.recv(msg, zmq::recv_flags::none)) {
        return "";
    }

    string s((char*)msg.data(), msg.size());
    vector<string> parts = split(s, ':');

    if (parts.size() >= 2 && parts[0] == "NAMESERVER") {
        return parts[1];
    }
    return "";
}

void beacon_nameserver(string local_ip_addr, int port_nameserver) {
    zmq::context_t context(1);
    zmq::socket_t pub(context, zmq::socket_type::pub);

    string bind_addr = "tcp://" + local_ip_addr + ":" + to_string(port_nameserver);
    pub.bind(bind_addr.c_str());

    cout << "local p2p name server bind to " << bind_addr << "." << endl;

    while (true) {
        Sleep(1000);
        string msg = "NAMESERVER:" + local_ip_addr;

        pub.send(zmq::buffer(msg), zmq::send_flags::none);
    }
}


void user_manager_nameserver(string local_ip_addr, int port_subscribe) {
    vector<vector<string>> user_db;

    zmq::context_t context(1);
    zmq::socket_t rep(context, zmq::socket_type::rep);

    string bind_addr = "tcp://" + local_ip_addr + ":" + to_string(port_subscribe);
    rep.bind(bind_addr.c_str());

    cout << "local p2p db server activated at " << bind_addr << "." << endl;

    while (true) {

        zmq::message_t msg;
        rep.recv(msg, zmq::recv_flags::none);

        string req((char*)msg.data(), msg.size());
        vector<string> parts = split(req, ':');

        user_db.push_back(parts);

        cout << "user registration '" << parts[1] << "' from '" << parts[0] << "'." << endl;

        string ok = "ok";
        rep.send(zmq::buffer(ok), zmq::send_flags::none);
    }
}

void relay_server_nameserver(string local_ip_addr, int port_chat_publisher, int port_chat_collector) {

    zmq::context_t context(1);
    zmq::socket_t pub(context, zmq::socket_type::pub);
    zmq::socket_t pull(context, zmq::socket_type::pull);

    string pub_addr = "tcp://" + local_ip_addr + ":" + to_string(port_chat_publisher);
    string pull_addr = "tcp://" + local_ip_addr + ":" + to_string(port_chat_collector);

    pub.bind(pub_addr.c_str());
    pull.bind(pull_addr.c_str());

    cout << "local p2p relay server activated at "
        << pub_addr << " & " << port_chat_collector << "." << endl;

    while (true) {
        zmq::message_t msg;
        pull.recv(msg, zmq::recv_flags::none);

        string s((char*)msg.data(), msg.size());

        cout << "p2p-relay:<==> " << s << endl;

        string relay_msg = "RELAY:" + s;
        pub.send(zmq::buffer(relay_msg), zmq::send_flags::none);
    }
}

void main_p2p(int argc, char* argv[]) {

    string ip_addr_p2p_server = "";
    int port_nameserver = 9001;
    int port_chat_publisher = 9002;
    int port_chat_collector = 9003;
    int port_subscribe = 9004;

    string user_name = argv[1];
    string ip_addr = get_local_ip();
    string ip_mask = ip_addr.substr(0, ip_addr.rfind('.'));

    cout << "searching for p2p server." << endl;

    string found = search_nameserver(ip_mask, ip_addr, port_nameserver);

    if (found == "") {

        ip_addr_p2p_server = ip_addr;

        cout << "p2p server is not found, and p2p server mode is activated." << endl;

        thread t1(beacon_nameserver, ip_addr, port_nameserver);
        t1.detach();
        cout << "p2p beacon server is activated." << endl;

        thread t2(user_manager_nameserver, ip_addr, port_subscribe);
        t2.detach();
        cout << "p2p subsciber database server is activated." << endl;

        thread t3(relay_server_nameserver, ip_addr, port_chat_publisher, port_chat_collector);
        t3.detach();
        cout << "p2p message relay server is activated." << endl;

    }
    else {
        ip_addr_p2p_server = found;

        cout << "p2p server found at " << found << ", and p2p client mode is activated." << endl;
    }

    cout << "starting user registration procedure." << endl;

    zmq::context_t db_context(1);
    zmq::socket_t db_client(db_context, zmq::socket_type::req);

    string reg_addr = "tcp://" + ip_addr_p2p_server + ":" + to_string(port_subscribe);
    db_client.connect(reg_addr.c_str());

    string regmsg = ip_addr + ":" + user_name;
    db_client.send(zmq::buffer(regmsg), zmq::send_flags::none);

    zmq::message_t rep;
    db_client.recv(rep, zmq::recv_flags::none);

    cout << "user registration to p2p server completed." << endl;

    cout << "starting message transfer procedure." << endl;

    zmq::context_t relay_context(1);
    zmq::socket_t p2p_rx(relay_context, zmq::socket_type::sub);
    zmq::socket_t p2p_tx(relay_context, zmq::socket_type::push);

    p2p_rx.set(zmq::sockopt::subscribe, "RELAY");

    string pub_conn = "tcp://" + ip_addr_p2p_server + ":" + to_string(port_chat_publisher);
    string col_conn = "tcp://" + ip_addr_p2p_server + ":" + to_string(port_chat_collector);

    p2p_rx.connect(pub_conn.c_str());
    p2p_tx.connect(col_conn.c_str());

    cout << "starting autonomous message transmit and receive scenario." << endl;

    srand((unsigned)time(NULL));

    while (true) {

        zmq::pollitem_t items[] = { { (void*)p2p_rx.handle(), 0, ZMQ_POLLIN, 0 } };

        zmq::poll(items, 1, std::chrono::milliseconds(100));

        if (items[0].revents & ZMQ_POLLIN) {

            zmq::message_t msg;
            p2p_rx.recv(msg, zmq::recv_flags::none); 

            string s((char*)msg.data(), msg.size());
            vector<string> parts = split(s, ':');

            cout << "p2p-recv::<<== " << parts[1] << ":" << parts[2] << endl;

        }
        else {
            int r = rand() % 100 + 1;

            if (r < 10) {
                Sleep(3000);
                string msg = "(" + user_name + "," + ip_addr + ":ON)";
                p2p_tx.send(zmq::buffer(msg), zmq::send_flags::none);
                cout << "p2p-send::==>> " << msg << endl;
            }
            else if (r > 90) {
                Sleep(3000);
                string msg = "(" + user_name + "," + ip_addr + ":OFF)";
                p2p_tx.send(zmq::buffer(msg), zmq::send_flags::none);
                cout << "p2p-send::==>> " << msg << endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "usage is 'python dechat.py _user-name_'." << endl;
        return 0;
    }

    cout << "starting p2p chatting program." << endl; 
    main_p2p(argc, argv);
    return 0;
}