#include "../../include/udp.hpp"
#include "../../include/tcp.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#define SERVER_PORT 19999

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
}

int main(int argc, char const *argv[]) {
	// Select Boost up UDP ports
	if (argc < 1) return 0;

	bool isLogin = false;

	printMenu(isLogin);
	cout << "Your Choice >> ";



	return 0;

}