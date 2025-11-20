TestLang++ (Java) – Backend API Testing DSL

Assignment Report / README.md

Programming Paradigms SE2052

Student ID: IT23632028





Introduction

This document provides a description on the design and implementation of TestLang++, a customized Domain Specific Language (DSL) that has been developed specifically to test HTTP APIs. The goal of the project is to take a .test file written in TestLang++, parse the file, check for syntax errors and produce an automatically generated JUnit 5 test class (GeneratedTests.java) that can be executed. The automatically generated class will utilize java.net.http.HttpClient (Java 11+) to run the provided tests against a running backend. Thereby creating the complete automated testing framework from a simple and high-level language. The compiler/parser is implemented  Flex (lexer.l), and Bison (parser.y) and the generated tests are made to run against the provided Spring Boot backend.

Project Organization

The project is organized into three main components.
1. The Parser (lexer.l, parser.y, Makefile): These files specify the lexical rules (tokens), grammar (productions), and build for the TestLang++ DSL. They will generate a file named GeneratedTests.java, using the .test file provided when built and run.
2. The TestLang++ DSL (example.test): This is the higher-level test script. It specifies configuration and variables, and contains a number of test blocks which consist of HTTP requests (GET/POST) and expect assertions.
3. The Generated Code (GeneratedTests.java): The generated runnable class produced by the parser (JUnit 5 class).

Executing the project

In this section we will provide an overview on how to execute the entire pipeline for testing from starting up the backend to running the generated tests. 

1. Run the parser
At the first step, we parse our example.test DSL script, producing a JUnit test file. This is done through the parser's project folder (e.g., in VS Code), by utilizing a Makefile and mingw32-make.

Instructions:

From your VS Code project directory - where you have your Makefile, lexer.l, parser.y, and example.test files - open a terminal.

Build the parser: execute mingw32-make to compile the lexer and parser to parser.exe.

![run_parser](./images/mingw32-run.png)

This will create lex.yy.c, parser.tab.c, parser.tab.h and parser.exe.

Run the parser: Run the following line. This takes the contents of your example.test and pipes it to our compiled parser.exe, which processes it, sending standard output as a GeneratedTests.java file.

![code gen](./images/codewasgenerated.png)

GeneratedTests.java Output:

The parser is able to accurately convert the high-level DSL into verbose idiomatic Java code. The let variables (like $id) are substituted correctly and the default/request headers are included as specified.

![code generate java](./images/generatedcode.png)                                       ![DSL ](./images/exampletest.png)






2. Then open the spring boot project using Intelij Idea. Paste the generated code inside of test->java folder. Then run the backenend first. 




![Backend](./images/runbackend.png)




Instructions to run the test file. (GeneratedTests.java file)


  1. Verify that the Spring Boot backend continues to run (from Step 2).

  2. In the IntelliJ project explorer, right-click GeneratedTests.java.

  3. In the context menu, select Run 'GeneratedTests'.

  4. IntelliJ will compile it and run the JUnit tests. The "Run" tool window will open at the bottom of the screen, showing you the test results (e.g., "Tests passed: 2 of 2").




Test Results

When the GeneratedTests.java file was run as a test file in the Spring Boot backend, all of the defined test cases ran successfully. This included the test methods: test_Login, test_GetUser, test_UpdateUser and test_DeleteUser. This demonstrates that the parser accurately translates the TestLang++ DSL into valid and functional Java JUnit tests.

![test](./images/testcaseresult.png)




Invalid Inputs and Error Messages 


1. Error 1: Invalid Identifier

Identifiers (for variables and test names) in the TestLang++ specification must begin with a letter.

Invalid Input->

![msg](./images/errorone.png)                                                                                     

Output->

![msg](./images/erroroneoutput.png)
      

2. Error 2: Invalid Type for body

The body of a POST or PUT request must be of type STRING.

Invalid Input->

![msg](./images/errortwo.png)

Output->

![msg](./images/errortwooutput.png)


3. Error 3: Invalid Type for status

The expect status assertion must be followed by a NUMBER (integer).

Invalid Input->

![msg](./images/errorthree.png)

Output->

![msg](./images/errorthreeoutput.png)


4. Error 4: Missing Semicolon

All request statements (whats) such as GET must end with a semicolon.

Invalid Input->

![msg](./images/errorfour.png)

Output->

![msg](./images/errorfouroutput.png)


Conclusion

This project establishes a complete end-to-end pipeline for a custom DSL. The TestLang++ language allows users to define API tests cleanly and expressively before conventionally translating them to a fully functional JUnit 5 test class via a reliable Flex/Bison parser. The parser provides variable substitution management, header management, and diagnostics for syntax errors, satisfying all the stated core requirements of the assignment.
