# Java Minimal Example

Create a `HelloWorld.java` file with the following.

```java

import com.java.helics.helics;

public class HelloWorld {

    public static void main(String[] args) {
        System.loadLibrary("JNIhelics");

        System.out.println("HELICS Version: " + helics.helicsGetVersion());
    }

}
```

Run the following to compile all Java classes. You will first have to create a `com/java/helics` folder relative to the source folder, and place all the swig generated java files in that folder.

```bash
javac com/java/helics/helics.java
javac HelloWorld.java
java -Djava.library.path="/Library/Java/Extensions:/Network/Library/Java/Extensions:/System/Library/Java/Extensions:/usr/lib/java:/path/to/GitRepos/HELICS/build-osx/swig/java/com/java/helics/:." HelloWorld
```

You should see the output that is similar to the following.

```text
HELICS Version: x.x.x (XX-XX-XX)
```

creating a jar file.

```bash
jar cfv helics.jar com/java/helics/*.java
```
