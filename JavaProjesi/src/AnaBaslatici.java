import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class AnaBaslatici {
    public static void main(String[] args) 
    {
        System.out.println(">>> Ana Başlatıcı çalıştırıldı. Tüm uygulamalar başlatılıyor...");
        
        String javaBin = System.getProperty("java.home") + File.separator + "bin" + File.separator + "java";
        List<Process> backgroundProcesses = new ArrayList<>();

        
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("[BAŞLATICI]: Kapanış hook'u tetiklendi. Arka plan uygulamaları kapatılıyor...");
            for (Process process : backgroundProcesses) 
            
                if (process.isAlive()) 
                {
                    process.destroy();
                }
            }
        }));

        try {
            backgroundProcesses.add(startProcess("HedefSunucusu"));
            Thread.sleep(500);
            backgroundProcesses.add(startProcess("SensorUygulamasi"));
            Thread.sleep(500);
            
            System.out.println("[BAŞLATICI]: IsleyiciBirimGUI başlatılıyor...");
            Process guiProcess = startProcess("IsleyiciBirimGUI");
            
            int exitCode = guiProcess.waitFor();
            System.out.println("[BAŞLATICI]: İşleyici Birim GUI kapatıldı (Çıkış Kodu: " + exitCode + ").");

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        } finally {
            // Ana program bittiğinde, hook zaten çağrılmış olmalı ama garanti olsun.
            System.exit(0);
        }
    }

    private static Process startProcess(String mainClass) throws IOException 
    {
        String javaBin = System.getProperty("java.home") + File.separator + "bin" + File.separator + "java";
        ProcessBuilder builder = new ProcessBuilder(javaBin, "-cp", "bin", mainClass);
        builder.inheritIO();
        System.out.println("[BAŞLATICI]: " + mainClass + " başlatılıyor...");
        return builder.start();
    }
}
