#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atltypes.h>

using namespace sf;

const std::string SERVER_IP = "127.0.0.1";
const u_short SERVER_PORT = 50000;
const u_short SERVER_WAIT_TIME = 5;
const u_short MAX_PLAYERS = 5;
const std::string FOLDER_RESOURCES = "../../resources";

std::vector<std::string> aMensajes;

struct Player {
	TcpSocket* tcpSocket;
	std::string nick;

	Player(TcpSocket* _tcpSocket, std::string _nick):tcpSocket(_tcpSocket), nick(_nick) {}
};

void print(std::string text) {
	std::cout << text << std::endl;
}

void PeerListener(Player* player) {

	while (true) {
		sf::Packet packet;
		player->tcpSocket->receive(packet);
		std::string mensaje;
		packet >> mensaje;
		aMensajes.push_back(player->nick + mensaje);

		if (aMensajes.size() > 25)
		{
			aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
		}
	}
}

int main()
{
	Vector2i screenDimensions(800, 600);
	Font font;
	RenderWindow window;
	TcpListener listener;
	TcpSocket socket;
	int playersCount;
	std::vector<Player> peers;
	u_short portServer;
	Packet pack;
	Socket::Status serverStatus;
	sf::String mensaje;
	std::string nickname;

#pragma region CONNECTION TO SERVER

	serverStatus = socket.connect(SERVER_IP, SERVER_PORT, sf::seconds(SERVER_WAIT_TIME));

	if (serverStatus != Socket::Status::Done) {
		print("There was a problem trying to connect to the server");
		return 0;
	}

	std::cout << "Type your nickname:" << std::endl;
	std::cin >> nickname;

	pack << nickname;
	socket.send(pack);

	portServer = socket.getLocalPort();

	serverStatus = socket.receive(pack);
	if (serverStatus != Socket::Status::Done) {
		print("Esta reventando en el receive");
	}

	socket.disconnect();

	pack >> playersCount;

	for (short i = 0; i < playersCount; i++)
	{
		std::string ip_peer;
		short port_peer;
		std::string nick_peer;
		pack >> ip_peer >> port_peer >> nick_peer;
		TcpSocket* sockAux = new TcpSocket();
		sockAux->connect(ip_peer, port_peer);
		peers.push_back(Player(sockAux, nick_peer));

		Packet nickPack;
		nickPack << nickname;
		sockAux->send(nickPack);
	}

	print("Waiting for other players...");
	std::cout << MAX_PLAYERS - playersCount - 1 << std::endl;

	if (listener.listen(portServer) != Socket::Status::Done)
	{
		print("error at listener");
	}
	
	for (int i = 0; i < MAX_PLAYERS - playersCount - 1; i++)
	{
		TcpSocket* socketToAccept = new TcpSocket();
		listener.accept(*socketToAccept);
		std::string nickpeer;
		socketToAccept->receive(pack);
		pack >> nickpeer;
		print((nickpeer + "entrered"));
		peers.push_back(Player(socketToAccept, nickpeer));
	}

	listener.close();
#pragma endregion
	
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat "+ nickname);
	if (!font.loadFromFile(FOLDER_RESOURCES + "/courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = ">";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	for (int i = 0; i < peers.size(); i++) {
		std::thread thread(&PeerListener, &peers[i]);
		thread.detach();
	}
	
	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type) {
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape) {
					window.close();
				}
				else if (evento.key.code == sf::Keyboard::Return)
				{
					sf::Packet packet;
					std::string packetToSend = mensaje;
					packet << packetToSend;
					for (int i = 0; i < peers.size(); i++) {
						peers[i].tcpSocket->send(packet);
					}

					aMensajes.push_back(mensaje);

					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}

					mensaje = ">";
				}
				break;

			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}

		window.draw(separator);

		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}

		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);

		window.display();
		window.clear();
	}
	return 0;
}