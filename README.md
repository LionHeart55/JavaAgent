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