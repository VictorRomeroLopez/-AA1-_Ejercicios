#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <chrono>

sf::TcpListener dispatcher;
std::mutex _mutex;

void print(std::string textToPrint) {
	std::cout << textToPrint << std::endl;
}

#pragma region BLOCKING
bool SendPacketAllClients(sf::Packet packet, std::vector<sf::TcpSocket*>* incoming) {
	_mutex.lock();
	sf::Socket::Status status;
	for (int i = 0; i < incoming->size(); i++) {
		status = incoming->at(i)->send(packet);
		if (status != sf::Socket::Done) {
			incoming->at(i)->disconnect();
			incoming->erase(incoming->begin() + i);
			_mutex.unlock();
			return true;
		}
	}
	_mutex.unlock();
	return false;
}

void client(sf::TcpSocket* socket, std::vector<sf::TcpSocket*>* incoming) {
	print("I'm connected " + socket->getRemoteAddress().toString());

	sf::Packet _packet;
	_packet << "I'm connected " + socket->getRemoteAddress().toString();
	if (SendPacketAllClients(_packet, incoming)) return;

	while (true) {
		sf::Packet packet;
		sf::Socket::Status status = socket->receive(packet);
		if (status == sf::Socket::Disconnected)
		{
			print("Client " + socket->getRemoteAddress().toString() + " disconected!"); 
			sf::Packet _packet;
			_packet << "Client " + socket->getRemoteAddress().toString() + " disconected!";
			if(SendPacketAllClients(_packet, incoming)) return;
		}
		if (SendPacketAllClients(packet, incoming)) return;
	}
	print("Dissconected");
}

#pragma endregion

int main()
{
	using namespace std::chrono_literals;
	std::string serverMethod;
	std::vector<sf::TcpSocket*> *incoming = new std::vector<sf::TcpSocket*>();

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

	sf::TcpSocket* socket;
	sf::Packet packet;
	while (true) {

		socket = new sf::TcpSocket();
		status = dispatcher.accept(*socket);

		if (status == sf::Socket::NotReady) {
			socket = NULL;
			delete socket;
		}
		else if( status == sf::Socket::Done)
		{
			_mutex.lock();
			incoming->push_back(socket);
			_mutex.unlock();

			switch (std::stoi(serverMethod)) {
				case 1: 
				{
					std::thread threadstd(&client, socket, incoming);
					threadstd.detach();
					socket = NULL;
					break;
				}
				case 3:
					break;
			}
		}

		if (std::stoi(serverMethod) == 2 && incoming->size() > 0) {
			for (int i = 0; i < incoming->size(); i++) {
				incoming->at(i)->setBlocking(false);
				status = incoming->at(i)->receive(packet);
				if (status == sf::Socket::Disconnected)
				{
					print("Client " + incoming->at(i)->getRemoteAddress().toString() + " disconected!");
					sf::Packet _packet;
					_packet << "Client " + incoming->at(i)->getRemoteAddress().toString() + " disconected!";
					SendPacketAllClients(_packet, incoming);
				}
				else if (status == sf::Socket::Done) {
					SendPacketAllClients(packet, incoming);
				}
			}
			std::this_thread::sleep_for(1s);
		}
	}

	return 0;
}