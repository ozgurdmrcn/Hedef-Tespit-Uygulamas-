// Bu sınıf, Sensör uygulamasının ürettiği veriyi taşımak için kullanılır.
public class SensorData {
    public final Point position;
    public final double angle;

    public SensorData(Point position, double angle)
    {
        this.position = position;
        this.angle = angle;
    }
}
