import com.java.helics.helics;

public class HelloWorld {

    public static void main(String[] args) {
        System.loadLibrary("helicsJava");

        System.out.println("HELICS Version: " + helics.helicsGetVersion());
    }

}
