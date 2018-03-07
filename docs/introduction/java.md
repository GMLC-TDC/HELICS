
# Java Minimal Example

Create a `HelloWorld.java` file with the following.

```java

public class HelloWorld {

    public static void main(String[] args) {
		System.loadLibrary("JNIhelics");

		System.out.println("HELICS Version: " + helics.helicsGetVersion());
	}

}
```

Run the following to compile all Java classes.

```bash
javac helics.java
javac HelloWorld.java
java -Djava.library.path="/Library/Java/Extensions:/Network/Library/Java/Extensions:/System/Library/Java/Extensions:/usr/lib/java:/path/to/GitRepos/HELICS-src/build-osx/swig/java/:." HelloWorld
```

You should see the output that is similar to the following.

```
HELICS Version: 1.0.0-beta.1 02-26-18
```

creating a jar file.

```bash
jar cfv helics.jar <java files>
```
