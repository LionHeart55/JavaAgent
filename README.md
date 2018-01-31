# Agent

## Introduction
This c++ project is an example of how to implement JVMTI library to attach to a JVM and gather data on exception in runtime.

1) tap into the load classes capabilities, get filename, read it ans save class file to /tmp/
2) tap into exception capabilities
   - Get Exception message
   - Get Exception class
   - Get stack frames and for each frame
      - Get Class.Method.
      - Get line Number.
      - Get Variable names and values.

## Prerequisites

Need to include the JVMTI header library located under:
goto project-->properties choose C/C++ General--> Paths and symbols, then choose includes ==> GNU C++
then add these directories within your JDK folder for example:
/Library/Java/JavaVirtualMachines/jdk1.8.0_121.jdk/Contents/Home/include
/Library/Java/JavaVirtualMachines/jdk1.8.0_121.jdk/Contents/Home/include/darwin

To build in eclipse choose project right click and click 'Build All'
if this does not work you will need to create an new project in your IDE and import the sources.

## Usage

java -agentpath:Debug/Agent  -jar javajam.jar

OR

sudo chmod +x run.sh
./run.sh

## Sample output

Exception Reference Type: Local Reference
Exception Message: Throwing a test exception: IllegalThreadStateException
Stacktrace: Frame count=2 num of frames: 2

Number of records filled: 2
Exception Stack Trace

Stack Trace Depth: 2
at method Throwing( 47 ) in class LMain/Main;
Variables: 1
Type: Integer, Name: value, Value: 8
at method main( 19 ) in class LMain/Main;
Variables: 1
Type: Integer, Name: value, Value: 8
