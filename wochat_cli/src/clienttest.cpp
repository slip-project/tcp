#include "udp.hpp"
#include "tcp.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#define SERVER_PORT 19999

using std::cin;
using std::endl;
using std::cout;

void printMenu(bool loginStatus) {
	cout << "" << endl;
	cout << "-------------------------------------------------------" << endl;
	cout << "Welcome to Wochat(Command Line Version) !" << endl;
	cout << "You can use the Following Commands:" << endl;
	cout << "-------------------------------------------------------" << endl;
	if (loginStatus) {
		cout << "LOGOUT	-	LOGOUT the WoChat." << endl;
		cout << "LIST	-	LIST all Online users." << endl;
		cout << "SEND	-	SEND a message to the current user." << endl;
		cout << "PULL	-	PULL your message from the server." << endl;
	} else {
		cout << "LOGIN	-	LOGIN the WoChat." << endl;
	}
	cout << "-------------------------------------------------------" << endl;
	cout << endl;
}

void printChoices(bool loginStatus , std::string& username , std::string& instructions) {
	if (loginStatus) {
		cout << "Enter your choice:" << endl;
		cout << username << " >> ";
		cin >> instructions;
	} else {
		cout << "Please Login first." << endl;
		cout << "Your Username >> ";
		cin >> userName;
	}
}


int main(int argc, char const *argv[]) {
	// Select Boost up UDP ports
	if (argc < 2) return 0;



	unsigned short listen_port = atoi(argv[1]);
	unsigned short send_port = 18888;
	std::string server_ip = slip::get_local_ip();
	
	bool isLogin = false;
	bool transact_not_finished = true;
	std::string instruction;
	std::string currentUser = "";
	std::string userName;
	slip::Udp udp;



	printMenu(isLogin);
	printChoices(isLogin, username , instruction);

		/**
		 * Start a port to listen udp from server
		 */
	
	// udp.add_listener(listen_port, [&transact_not_finished](std::string source_ip, unsigned short source_port, std::string message)->void{
	// 	if (message == "LGIN_SUCCESS") {
	// 		isLogin = true;
	// 		currentUser = userName;
	// 		cout << "Login Successfully! Enjoy :)" << endl;
	// 	} else if (message == "LGOU_SUCCESS") {
	// 		isLogin = false;
	// 		currentUser = "";
	// 		userName = "";
	// 		cout << "Logout Successfully! See you next time :)" << endl;
	// 	} else if (message != "END_MSG_BODY") {
	// 		cout << message << endl;
	// 	} else {
	// 		transact_not_finished = false;
	// 	}
	// });
    
    // while (transact_not_finished) {}
    // cout << endl;
	
 //    printMenu(isLogin);
	// printChoices(isLogin , username , instruction);
	while (1) {
		cin >> instruction;
		if (instruction == "LOGIN" && !isLogin) {
			cin >> userName;
			std::stringstream stream;
	        stream << "LGIN " << userName << " " << listen_port << endl;
	        std::string message = stream.str();
			udp.send(server_ip , SERVER_PORT , send_port , message);
			cout << "Login Successfully! Enjoy :)" << endl;
			
			isLogin = true;
			currentUser = userName;

		} else if (instruction == "LOGOUT" && isLogin) {
			std::stringstream stream;
	        stream << "LGOU " << userName << " " << endl;
	        std::string message = stream.str();
			udp.send(server_ip , SERVER_PORT , send_port , message);
			isLogin = false;
			currentUser = "";
			userName = "";
		}
	}

	// while (cin >> instruction) {
// 
	// }





	return 0;

}