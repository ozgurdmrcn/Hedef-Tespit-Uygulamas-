#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>


const int PORT = 8080;
const int MAX_COORDINATE = 1000;
const std::string COMMAND = "GET_POSITION";
const int UPDATE_INTERVAL_SECONDS = 5;


class Target {
private:
	int m_x, m_y;
	mutable std::mutex m_mutex;
public:
	Target()
	{
		srand(time(NULL));
		updatePosition();
	}
	void updatePosition(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_x = rand() % MAX_COORDINATE;
		m_y = rand() % MAX_COORDINATE;
	}
	std::pair<int, int> getPosition(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return {m_x, m_y};
	}
};


class TcpServer {
private:
	void setup(void);
	void acceptConnections(void);
	static void handleClient(int clientSocket, Target& target);
	int m_port;
	int m_server_socket;
	Target& m_target;
public:
	TcpServer(int port, Target& target)
	: m_port(port), m_server_socket(-1), m_target(target) {}
	~TcpServer()
	{
		if (m_server_socket != -1)
		{
			close(m_server_socket);
		}
		else
		{
			//No Action
		}
	}
	void run(void)
	{
		setup();
		acceptConnections();
	}
};

void TcpServer::setup(void)
{
	m_server_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (m_server_socket == -1)
	{
		throw std::runtime_error("Soket olusturulamadi.");
	}
	else
	{
		//No Action
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(m_port);
	if (bind(m_server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		close(m_server_socket);
		throw std::runtime_error("Soket bind edilemedi.");
	}
	else
	{
		//No Action
	}

	if (listen(m_server_socket, 10) == -1)
	{
		close(m_server_socket);
		throw std::runtime_error("Hata: Soket dinlenemedi.");
	}
	else
	{
		//No Action
	}
	std::cout << "Sunucu baslatildi, " << m_port << " portu dinleniyor." << std::endl;
}

void TcpServer::acceptConnections(void)
{
	while (true)
	{
		int clientSocket = accept(m_server_socket, nullptr, nullptr);
		if (clientSocket == -1)
		{
			std::cerr << "Baglanti kabul edilemedi." << std::endl;
			continue;
		}
		else
		{
			//No Action
		}
		std::thread clientThread(handleClient, clientSocket, std::ref(m_target));
		clientThread.detach();
	}
}

void TcpServer::handleClient(int clientSocket, Target& target)
{
	std::cout << "Yeni bir sensor baglandi." << std::endl;
	char buffer[1024] = {0};
	int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
	if (bytesRead > 0)
	{
		std::string received_command(buffer, bytesRead);
		size_t end_pos = received_command.find_first_of("\r\n");
		if (end_pos != std::string::npos)
		{
			received_command = received_command.substr(0, end_pos);
		}

		if (received_command == COMMAND)
		{
			std::pair<int,int> position = target.getPosition(); //auto pos = target.getPosition();
			std::string response = std::to_string(position.first) + "," + std::to_string(position.second);
			write(clientSocket, response.c_str(), response.length());
			std::cout << "Hedef konumu (" << response << ") sensore gonderildi." << std::endl;
		}
		else
		{
			//No Action
		}
	}
	else
	{
		//No Action
	}
	close(clientSocket);
	std::cout << "Sensor baglantisi kapatildi." << std::endl;
}


void updaterTask(Target& target)
{
	std::cout << "Hedef güncelleme thread'i başlatıldı." << std::endl;
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(UPDATE_INTERVAL_SECONDS));
		target.updatePosition();
		std::pair<int,int> position = target.getPosition();//auto pos = target.getPosition();
		std::cout << "--------------------------------------------------\n";
		std::cout << "Hedef yeni konuma geçti -> ("
				<< position.first << ", " << position.second << ")\n";
		std::cout << "--------------------------------------------------\n";
	}
}


int main()
{
	try {
		Target sharedTarget;

		std::thread updaterThread(updaterTask, std::ref(sharedTarget));
		updaterThread.detach();

		TcpServer server(PORT, sharedTarget);
		server.run();

	} catch (const std::runtime_error& e) {
		std::cerr << "[KRITIK HATA] " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
