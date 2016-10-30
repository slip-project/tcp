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
		cout << "logout	-	LOGOUT the WoChat." << endl;
		cout << "list	-	LIST all Online users." << endl;
		cout << "send	-	SEND a message to the current user." << endl;
		cout << "pull	-	PULL your message from the server." << endl;
	} else {
		cout << "login	-	LOGIN the WoChat." << endl;
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
		instructions = "login";
		cout << "Please Login first." << endl;
		cout << "Your Username >> ";
		cin >> username;
	}
}


int main(int argc, char const *argv[]) {
	// Select Boost up UDP ports
	if (argc < 2) return 0;



	unsigned short listen_port = atoi(argv[1]);
	unsigned short send_port = 18888;
	std::string server_ip = slip::get_local_ip();
	
	bool isLogin = false;
	// bool transact_not_finished = true;
	std::string instruction;
	std::string currentUser = "";
	std::string userName;
	slip::Udp udp;


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
		printMenu(isLogin);
		printChoices(isLogin, userName , instruction);
		if (instruction == "login" && !isLogin) {
			std::stringstream stream;
	        stream << "LGIN " << userName << " " << listen_port << endl;
	        std::string message = stream.str();
			udp.send(server_ip , SERVER_PORT , send_port , message);
			cout << "Login Successfully! Enjoy :)" << endl;
			
			isLogin = true;
			currentUser = userName;

		} else if (instruction == "logout" && isLogin) {
			std::stringstream stream;
	        stream << "LGOU " << currentUser << " " << endl;
	        std::string message = stream.str();
			udp.send(server_ip , SERVER_PORT , send_port , message);

			cout << currentUser << " Logout Successfully! See you next time :)" << endl;

			isLogin = false;
			currentUser = "";
			userName = "";

		} else if (instruction == "send" && isLogin) {
			std::string tempname , tempcontent;
			std::stringstream stream;

			cout << "Please Input Your Friend\'s Username:" << endl;
			cout << currentUser << " >> ";
			cin >> tempname;
			cout << "Please Input the Message and press enter:" << endl;
			cout << currentUser << " >> ";
			std::getline(cin , tempcontent);

			stream << "SEND " << userName << " " << tempname << " " << tempcontent;
			std::string message = stream.str();

			udp.send(server_ip , SERVER_PORT , send_port , message);

			cout << "BRAVOO! Your Message Has Successfully SENT to Your Buddy!" << endl;

		}
	}

	// while (cin >> instruction) {
// 
	// }





	return 0;

}