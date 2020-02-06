#include <SFML\Network.hpp>
#include <iostream>
#include <vector>

void print(std::string textToPrint) {
	std::cout << textToPrint << std::endl;
}

int main()
{
	sf::TcpListener dispatcher;
	sf::Socket::Status status = dispatcher.listen(50000);

	if (status != sf::Socket::Done) {
		print("Can't link to port 50000");
	}
	else {
		print("Succesfully linked to port 50000");
	}

	std::vector<sf::TcpSocket*> incoming;
	while (true) {
		incoming.push_back(new sf::TcpSocket);
		if (dispatcher.accept(*incoming.at(incoming.size() - 1)) != sf::Socket::Done) {
			print("Error al aceptar la conexion");
		}
		else {
			print("Gucci linked to " + incoming.at(incoming.size() - 1)->getRemoteAddress().toString());
		}
	}
}