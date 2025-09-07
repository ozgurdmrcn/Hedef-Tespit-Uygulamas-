import java.io.IOException;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;
import java.util.Random;

public class HedefSunucusu {
    private static final int PORT = 8080;
    private static final int MAX_COORDINATE = 1000;
    private static final int UPDATE_INTERVAL_SECONDS = 5;

    private static volatile int targetX;
    private static volatile int targetY;
    private static volatile long lastUpdateTime;
    private static final Random random = new Random();

    public static void main(String[] args) throws IOException {
        updateTargetPosition();
        System.out.printf("[HEDEF SUNUCU]: Sunucu Port %d üzerinde başlatıldı. İlk hedef: (%d, %d)\n", PORT, targetX, targetY);

        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true)
            {
                new ClientHandler(serverSocket.accept()).start();
            }
        }
    }

    private static synchronized void updateTargetPosition()
    {
        targetX = random.nextInt(MAX_COORDINATE);
        targetY = random.nextInt(MAX_COORDINATE);
        lastUpdateTime = new Date().getTime();
    }

    private static class ClientHandler extends Thread {
        private final Socket clientSocket;
        public ClientHandler(Socket socket) { this.clientSocket = socket; }

        @Override
        public void run() {
            try (PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)) {
                long currentTime = new Date().getTime();
                if ((currentTime - lastUpdateTime) / 1000 >= UPDATE_INTERVAL_SECONDS) 
                {
                    updateTargetPosition();
                    System.out.printf("[HEDEF SUNUCU]: Hedef yeni konuma geçti: (%d, %d)\n", targetX, targetY);
                }
                String position = targetX + "," + targetY;
                out.println(position);
            } catch (IOException e) {
                System.out.println("Hata: " + e.getMessage());
            }
        }
    }
}
