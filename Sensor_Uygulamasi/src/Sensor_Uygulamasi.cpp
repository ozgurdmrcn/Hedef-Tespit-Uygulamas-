#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <cmath>

const int MAX_COORDINATE = 1000;
const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT_8080 = 8080;
const int SERVER_PORT_8090 = 8090;
const int BUFFER_SIZE = 1024;
const char* REQUEST_POSITION = "GET_POSITION";
const char* REQUEST_TARGET = "GET_TARGET";

class Sensor {
public:
    Sensor()
    {
        srand(time(NULL) + rand());
        updatePosition();
    }

    void updatePosition()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_x = rand() % MAX_COORDINATE;
        m_y = rand() % MAX_COORDINATE;
    }

    std::pair<int, int> getPosition() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return std::pair<int, int>(m_x, m_y);
    }

private:
    int m_x = 0;
    int m_y = 0;
    mutable std::mutex m_mutex;
};

std::pair<int, int> getTargetFromServer8080()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("8080 socket");
        return std::pair<int, int>(-1, -1);
    }
	else
	{
		//No Action
	}

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT_8080);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("8080 connect");
        close(sock_fd);
        return std::pair<int, int>(-1, -1);
    }
	else
	{
		//No Action
	}

    send(sock_fd, REQUEST_POSITION, std::strlen(REQUEST_POSITION), 0);

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    close(sock_fd);

    int x = 0, y = 0;
    if (bytes > 0 && std::sscanf(buffer, "%d,%d", &x, &y) == 2)
    {
        return std::pair<int, int>(x, y);
    }
	else
	{
		//No Action
	}

    return std::pair<int, int>(-1, -1);
}

double calculateAngle(std::pair<int, int> pos, std::pair<int, int> target)
{
    double dx = static_cast<double>(target.first - pos.first);
    double dy = static_cast<double>(target.second - pos.second);
    double angle = std::atan2(dy, dx) * 180.0 / M_PI;
    return angle < 0 ? angle + 360.0 : angle;
}

void sensor_updater_thread(Sensor& sensor)
{
    while (true)
    {
        sensor.updatePosition();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void tcp_server_thread(Sensor& s1, Sensor& s2)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("8090 socket");
        return;
    }
	else
	{
		//No Action
	}

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT_8090);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("8090 bind");
        close(server_fd);
        return;
    }
	else
	{
		//No Action
	}

    if (listen(server_fd, 5) < 0)
    {
        perror("8090 listen");
        close(server_fd);
        return;
    }
	else
	{
		//No Action
	}

    std::cout << "8090 sunucusu dinleniyor...\n";

    while (true)
    {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            perror("8090 accept");
            continue;
        }
		else
		{
			//No Action
		}

        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            close(client_fd);
            continue;
        }

        buffer[bytes] = '\0';
        if (std::strncmp(buffer, REQUEST_TARGET, std::strlen(REQUEST_TARGET)) == 0)
        {
            std::pair<int, int> target = getTargetFromServer8080();
            if (target.first == -1)
            {
                const char* err = "Target alınamadı\n";
                send(client_fd, err, std::strlen(err), 0);
                close(client_fd);
                continue;
            }
    		else
    		{
    			//No Action
    		}

            std::cout << "Hedef koordinat alındı: (" << target.first << ", " << target.second << ")\n";

            std::pair<int, int> p1 = s1.getPosition();
            std::pair<int, int> p2 = s2.getPosition();
            double a1 = calculateAngle(p1, target);
            double a2 = calculateAngle(p2, target);

            std::cout << "Sensor-1: (" << p1.first << ", " << p1.second << "), Açı: " << a1 << "°\n";
            std::cout << "Sensor-2: (" << p2.first << ", " << p2.second << "), Açı: " << a2 << "°\n";

            char response[BUFFER_SIZE];
            std::snprintf(response, sizeof(response),
                "Target: (%d,%d)\nSensor-1: (%d,%d), Angle: %.2f\nSensor-2: (%d,%d), Angle: %.2f\n",
                target.first, target.second,
                p1.first, p1.second, a1,
                p2.first, p2.second, a2);

            send(client_fd, response, std::strlen(response), 0);
        }
		else
		{
			//No Action
		}

        close(client_fd);
    }
}

int main()
{
    Sensor sensor1;
    Sensor sensor2;

    std::thread updater1(sensor_updater_thread, std::ref(sensor1));
    std::thread updater2(sensor_updater_thread, std::ref(sensor2));
    std::thread server(tcp_server_thread, std::ref(sensor1), std::ref(sensor2));

    updater1.detach();
    updater2.detach();
    server.join();

    return 0;
}
