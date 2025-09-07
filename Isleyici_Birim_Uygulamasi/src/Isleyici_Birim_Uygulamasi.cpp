#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT_8090 = 8090;
const int BUFFER_SIZE = 1024;
const char* REQUEST = "GET_TARGET";


struct SensorData {
    int x;
    int y;
    double angle_deg;
};

bool parseResponse(const std::string& response, SensorData& s1, SensorData& s2)
{
    std::istringstream iss(response);
    std::string line;
    int count = 0;

    while (std::getline(iss, line))
    {
        if (line.find("Sensor-1") != std::string::npos)
        {
            std::sscanf(line.c_str(), "Sensor-1: (%d,%d), Angle: %lf", &s1.x, &s1.y, &s1.angle_deg);
            count++;
        }
        else if (line.find("Sensor-2") != std::string::npos) {
            std::sscanf(line.c_str(), "Sensor-2: (%d,%d), Angle: %lf", &s2.x, &s2.y, &s2.angle_deg);
            count++;
        }
		else
		{
			//No Action
		}
    }

    return count == 2;
}

std::pair<int, int> estimateTarget(const SensorData& s1, const SensorData& s2)
{
    double theta1 = s1.angle_deg * M_PI / 180.0;
    double theta2 = s2.angle_deg * M_PI / 180.0;

    double dx1 = std::cos(theta1);
    double dy1 = std::sin(theta1);
    double dx2 = std::cos(theta2);
    double dy2 = std::sin(theta2);

    double det = dx1 * dy2 - dy1 * dx2;

    if (std::fabs(det) < 1e-8)
    {
        // Doğrular paralel veya çok yakın, kesişim güvenilmez
        return std::pair<int, int>(-1, -1);
    }
	else
	{
		//No Action
	}

    double x_diff = s2.x - s1.x;
    double y_diff = s2.y - s1.y;

    double t1 = (x_diff * dy2 - y_diff * dx2) / det;

    double x_target = s1.x + t1 * dx1;
    double y_target = s1.y + t1 * dy1;

    return std::pair<int, int>(static_cast<int>(std::round(x_target)),
                               static_cast<int>(std::round(y_target)));
}



void calculate_target()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return;
    }
	else
	{
		//No Action
	}

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT_8090);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return;
    }
	else
	{
		//No Action
	}

    send(sock_fd, REQUEST, std::strlen(REQUEST), 0);

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    close(sock_fd);

    if (bytes <= 0)
    {
        std::cerr << "Sunucudan veri alınamadı.\n";
        return;
    }
	else
	{
		//No Action
	}

    buffer[bytes] = '\0';
    std::string response(buffer);

    SensorData s1, s2;
    if (!parseResponse(response, s1, s2))
    {
        std::cerr << "Yanıt ayrıştırılamadı:\n" << response << "\n";
        return;
    }
	else
	{
		//No Action
	}

    std::pair<int, int> target = estimateTarget(s1, s2);

    std::cout << "Tahmini hedef konumu: (" << target.first << ", " << target.second << ")\n";
}

int main()
{
    std::string command;
    std::cout << "Komut girin (örn: calculate_target): ";

    while (std::getline(std::cin, command))
    {
        if (command == "calculate_target")
        {
            calculate_target();
        }
        else
        {
            std::cout << "Bilinmeyen komut: " << command << "\n";
        }


        std::cout << "\nKomut girin (örn: calculate_target): ";
    }

    return 0;
}
