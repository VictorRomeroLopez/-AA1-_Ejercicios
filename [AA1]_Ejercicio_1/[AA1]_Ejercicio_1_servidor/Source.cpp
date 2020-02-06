#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>

void print(std::string textToPrint) {
	std::cout << textToPrint << std::endl;
}

void client(sf::TcpSocket* socket) {

	print("I'm connected " + socket->getRemoteAddress().toString());
	while (true) {

	}
}

int main()
{
	std::string serverMethod;

	do {
		print("Selecciona un metode:");
		print("\t1. Blocking + Threading");
		print("\t2. NonBlocking");
		print("\t3. Blocking + SocketSelector");
		std::cin >> serverMethod;
	} while (serverMethod != "1" && serverMethod != "2" && serverMethod != "3");

	sf::TcpListener dispatcher;
	std::vector<sf::TcpSocket*> incoming;

	dispatcher.setBlocking(!(std::stoi(serverMethod) == 2));
	int port = 50000;

	sf::Socket::Status status = dispatcher.listen(port);

	while (status != sf::Socket::Done) {
		print("Can't link to port " + std::to_string(port));
		port++;
		status = dispatcher.listen(port);
	}

	print("Succesfully linked to port " + std::to_string(port));

	while (true) {

		sf::TcpSocket* socket = new sf::TcpSocket();
		if (dispatcher.accept(*socket) != sf::Socket::Done) {
			print("Error al aceptar la conexion");
			socket = NULL;
			delete socket;
		}
		else 
		{
			switch (std::stoi(serverMethod)) {
			case 1: {
				sf::Thread thread(&client, socket);
				thread.launch();
				}
				break;
			case 2:
				break;
			case 3:
				break;
			}
			incoming.push_back(socket);
		}
	}
}