#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>

sf::TcpListener dispatcher;

void print(std::string textToPrint) {
	std::cout << textToPrint << std::endl;
}

struct Data {
public:
	sf::TcpSocket* socket;
	std::vector<sf::TcpSocket*> incoming;

	Data(sf::TcpSocket*  _socket, std::vector<sf::TcpSocket*> _incoming) {
		socket = _socket;
		incoming = _incoming;
	}
	
	void client() {
		print("I'm connected " + socket->getRemoteAddress().toString());

		while (true) {
			sf::Packet packet;
			socket->receive(packet);
			std::string packetData;
			packet >> packetData;
			print(packetData);
			std::cout << incoming.size() << std::endl;
			for (int i = 0; i < incoming.size(); i++) {
				incoming[i]->send(packet);
			}
		}
		print("Dissconected");
	}
};


int main()
{
	std::string serverMethod;
	std::vector<sf::TcpSocket*> incoming;

	do {
		print("Selecciona un metode:");
		print("\t1. Blocking + Threading");
		print("\t2. NonBlocking");
		print("\t3. Blocking + SocketSelector");
		std::cin >> serverMethod;
	} while (serverMethod != "1" && serverMethod != "2" && serverMethod != "3");

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
			incoming.push_back(socket);
			switch (std::stoi(serverMethod)) {
			case 1: 
			{
				Data* data = new Data(socket, incoming);
				sf::Thread thread( &Data::client, data);
				thread.launch();
				break;
			}
			case 2:
				break;
			case 3:
				break;
			}
		}
	}
}