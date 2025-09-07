import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Random;

public class SensorUygulamasi {
    private static final int SIMULASYON_PORT = 8080;
    private static final String SIMULASYON_IP = "127.0.0.1";
    private static final int ISLEYICI_DINLEME_PORTU = 9090;
    private static final int MAX_COORDINATE = 1000;
    private static final Random random = new Random();

    public static void main(String[] args) throws IOException {
        System.out.println(">>> [SENSÖR SERVİSİ]: Port " + ISLEYICI_DINLEME_PORTU + " üzerinde dinleniyor...");
        try (ServerSocket listener = new ServerSocket(ISLEYICI_DINLEME_PORTU)) {
            while (true)
            {
                new PUHandler(listener.accept()).start();
            }
        }
    }

    private static class PUHandler extends Thread {
        private final Socket puSocket;
        PUHandler(Socket socket) { this.puSocket = socket; }

        @Override
        public void run() {
            try (
                BufferedReader in = new BufferedReader(new InputStreamReader(puSocket.getInputStream()));
                PrintWriter out = new PrintWriter(puSocket.getOutputStream(), true)
            ) {
                if ("GET_TARGET".equals(in.readLine())) 
                {
                    QuerySimulationTask task1 = new QuerySimulationTask(new Point(random.nextInt(MAX_COORDINATE), random.nextInt(MAX_COORDINATE)));
                    QuerySimulationTask task2 = new QuerySimulationTask(new Point(random.nextInt(MAX_COORDINATE), random.nextInt(MAX_COORDINATE)));
                    
                    Thread t1 = new Thread(task1);
                    Thread t2 = new Thread(task2);
                    t1.start(); t2.start();
                    t1.join(); t2.join();

                    SensorData data1 = task1.getResult();
                    SensorData data2 = task2.getResult();
                    
                    String response = String.format("%.8f,%.8f,%.8f:%.8f,%.8f,%.8f",
                        data1.position.x, data1.position.y, data1.angle,
                        data2.position.x, data2.position.y, data2.angle);
                    out.println(response);
                }
            } catch (Exception e) { e.printStackTrace(); }
        }
    }

    private static class QuerySimulationTask implements Runnable {
        private final Point sensorPosition;
        private SensorData result;
        QuerySimulationTask(Point position) { this.sensorPosition = position; }
        public SensorData getResult() { return result; }

        @Override
        public void run() 
        {
            try (Socket s = new Socket(SIMULASYON_IP, SIMULASYON_PORT);
                 PrintWriter out = new PrintWriter(s.getOutputStream(), true);
                 BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()))) {
                out.println("GET_POSITION");
                String[] parts = in.readLine().split(",");
                Point target = new Point(Double.parseDouble(parts[0]), Double.parseDouble(parts[1]));
                double angle = Math.toDegrees(Math.atan2(target.y - sensorPosition.y, target.x - sensorPosition.x));
                this.result = new SensorData(sensorPosition, angle);
            } catch (IOException e) {
                this.result = new SensorData(sensorPosition, -999.0);
            }
        }
    }
}
