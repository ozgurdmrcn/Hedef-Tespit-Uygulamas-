import javax.swing.*;
import java.awt.*;
import java.awt.geom.Line2D;
import java.io.*;
import java.net.Socket;
import java.util.concurrent.ExecutionException;

public class IsleyiciBirimGUI {
    // --- Ayarlar ---
    private static final int SENSOR_APP_PORT = 9090;
    private static final String SENSOR_APP_IP = "127.0.0.1";
    private static final String COMMAND = "GET_TARGET";
    private static final int SIMULATION_AREA_SIZE = 1000; // Simülasyon alanının boyutu

    // --- GUI Bileşenleri ---
    private JFrame frame;
    private DrawingPanel drawingPanel;
    private JLabel statusLabel;
    private Point sensor1, sensor2, calculatedTarget;

    public static void main(String[] args) 
    {
        SwingUtilities.invokeLater(() -> new IsleyiciBirimGUI().createAndShowGui());
    }

    private void createAndShowGui() 
    {
        frame = new JFrame("İşleyici Birim GUI");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(650, 700);

        drawingPanel = new DrawingPanel();
        drawingPanel.setBorder(BorderFactory.createEtchedBorder());
        
        statusLabel = new JLabel("Başlamak için butona tıklayın.", SwingConstants.CENTER);
        statusLabel.setFont(new Font("Arial", Font.PLAIN, 16));

        JButton calculateButton = new JButton("Veri Al & Hesapla");
        calculateButton.setFont(new Font("Arial", Font.BOLD, 16));
        calculateButton.addActionListener(e -> runCalculation());

        JPanel bottomPanel = new JPanel(new BorderLayout());
        bottomPanel.add(calculateButton, BorderLayout.CENTER);
        bottomPanel.add(statusLabel, BorderLayout.SOUTH);

        frame.getContentPane().add(drawingPanel, BorderLayout.CENTER);
        frame.getContentPane().add(bottomPanel, BorderLayout.SOUTH);

        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }

    private void runCalculation()
    {
        statusLabel.setText("Sensörlere bağlanılıyor...");
        new SwingWorker<Point, Void>() 
        {
            @Override
            protected Point doInBackground() throws Exception {
                try (Socket s = new Socket(SENSOR_APP_IP, SENSOR_APP_PORT);
                     PrintWriter out = new PrintWriter(s.getOutputStream(), true);
                     BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream()))) {
                    out.println(COMMAND);
                    String response = in.readLine();
                    if (response == null) throw new IOException("Sensörden boş cevap.");
                    
                    String[] sensorsData = response.split(":");
                    String[] s1Data = sensorsData[0].split(",");
                    String[] s2Data = sensorsData[1].split(",");

                    sensor1 = new Point(Double.parseDouble(s1Data[0]), Double.parseDouble(s1Data[1]));
                    double angle1 = Double.parseDouble(s1Data[2]);
                    sensor2 = new Point(Double.parseDouble(s2Data[0]), Double.parseDouble(s2Data[1]));
                    double angle2 = Double.parseDouble(s2Data[2]);
                    
                    return findTargetLocation(sensor1, angle1, sensor2, angle2);
                }
            }
            @Override
            protected void done() {
                try {
                    calculatedTarget = get();
                    if (calculatedTarget != null) {
                        statusLabel.setText(String.format("Hesaplanan Hedef: (%.2f, %.2f)", calculatedTarget.x, calculatedTarget.y));
                        drawingPanel.repaint();
                    } else { statusLabel.setText("Hedef hesaplanamadı."); }
                } catch (Exception e) { statusLabel.setText("Hata: " + e.getCause().getMessage()); }
            }
        }.execute();
    }
    
    private Point findTargetLocation(Point s1, double a1_deg, Point s2, double a2_deg) {
        double m1 = Math.tan(Math.toRadians(a1_deg));
        double m2 = Math.tan(Math.toRadians(a2_deg));
        if (Math.abs(m1 - m2) < 1e-9) return null;
        double x = (m1 * s1.x - m2 * s2.x + s2.y - s1.y) / (m1 - m2);
        double y = m1 * (x - s1.x) + s1.y;
        return new Point(x, y);
    }

    private class DrawingPanel extends JPanel {
        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2d = (Graphics2D) g;
            g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            int panelWidth = getWidth();
            int panelHeight = getHeight();

            double scaleX = (double) panelWidth / SIMULATION_AREA_SIZE;
            double scaleY = (double) panelHeight / SIMULATION_AREA_SIZE;

            // --- YENİ: Izgarayı (Grid) Çiz ---
            g2d.setColor(new Color(230, 230, 230)); // Açık gri renk
            g2d.setStroke(new BasicStroke(1));
            // Dikey çizgiler
            for (int i = 0; i <= SIMULATION_AREA_SIZE; i += 100) {
                int x = (int) (i * scaleX);
                g2d.drawLine(x, 0, x, panelHeight);
            }
            // Yatay çizgiler
            for (int i = 0; i <= SIMULATION_AREA_SIZE; i += 100) {
                int y = (int) (i * scaleY);
                g2d.drawLine(0, y, panelWidth, y);
            }

            // --- YENİ: Koordinat Eksenlerini ve Etiketlerini Çiz ---
            g2d.setColor(Color.DARK_GRAY);
            g2d.setStroke(new BasicStroke(2));
            // Y Ekseni (sol kenar)
            g2d.drawLine(0, 0, 0, panelHeight);
            // X Ekseni (alt kenar)
            g2d.drawLine(0, panelHeight, panelWidth, panelHeight);

            g2d.setFont(new Font("Arial", Font.PLAIN, 12));
            // Eksen etiketleri
            g2d.drawString("0", 5, panelHeight - 5);
            g2d.drawString("1000", panelWidth - 30, panelHeight - 5);
            g2d.drawString("1000", 5, 15);
            
            // Eğer çizilecek veri varsa, ızgaranın üzerine çiz
            if (sensor1 != null && sensor2 != null && calculatedTarget != null) {
                int s1_x = (int)(sensor1.x * scaleX), s1_y = (int)(panelHeight - (sensor1.y * scaleY));
                int s2_x = (int)(sensor2.x * scaleX), s2_y = (int)(panelHeight - (sensor2.y * scaleY));
                int h_x = (int)(calculatedTarget.x * scaleX), h_y = (int)(panelHeight - (calculatedTarget.y * scaleY));

                g2d.setColor(Color.GRAY);
                g2d.setStroke(new BasicStroke(1, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 0, new float[]{9}, 0));
                g2d.drawLine(s1_x, s1_y, h_x, h_y);
                g2d.drawLine(s2_x, s2_y, h_x, h_y);

                g2d.setColor(Color.BLUE);
                g2d.fillOval(s1_x - 5, s1_y - 5, 10, 10);
                g2d.drawString("S1", s1_x + 8, s1_y + 5);
                g2d.fillOval(s2_x - 5, s2_y - 5, 10, 10);
                g2d.drawString("S2", s2_x + 8, s2_y + 5);

                g2d.setColor(Color.RED);
                g2d.fillOval(h_x - 6, h_y - 6, 12, 12);
                g2d.drawString("HEDEF", h_x + 10, h_y + 6);
            }
        }
    }
}
