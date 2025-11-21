TestLang++ (Java) – Backend API Testing DSL

Introduction

This document provides a description on the design and implementation of TestLang++, a customized Domain Specific Language (DSL) that has been developed specifically to test HTTP APIs. The goal of the project is to take a .test file written in TestLang++, parse the file, check for syntax errors and produce an automatically generated JUnit 5 test class (GeneratedTests.java) that can be executed. The automatically generated class will utilize java.net.http.HttpClient (Java 11+) to run the provided tests against a running backend. Thereby creating the complete automated testing framework from a simple and high-level language. The compiler/parser is implemented  Flex (lexer.l), and Bison (parser.y) and the generated tests are made to run against the provided Spring Boot backend.

Project Organization

The project is organized into three main components.
1. The Parser (lexer.l, parser.y, Makefile): These files specify the lexical rules (tokens), grammar (productions), and build for the TestLang++ DSL. They will generate a file named GeneratedTests.java, using the .test file provided when built and run.
2. The TestLang++ DSL (example.test): This is the higher-level test script. It specifies configuration and variables, and contains a number of test blocks which consist of HTTP requests (GET/POST) and expect assertions.
3. The Generated Code (GeneratedTests.java): The generated runnable class produced by the parser (JUnit 5 class).
