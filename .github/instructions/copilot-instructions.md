---
applyTo: '**'
---
I am building a PostgreSQL extension using CMake and the ai-sdk-cpp library. 

I want to have a function based cpp files. 

- the folder `src` will have the execution cpp files `include` folder will have the header files that are required for the extension.

- The folder `third-party` will have all the external libraries. and the headers for the third party libraries such as `ai-sdk-cpp` will be in `third-party/include` folder and they can be included directly as we have linked them. 

- The main entry point of the extension is `pg_ask.cpp` which  will have the function definition for th extension. 

- The folde `pg` will have the postgres control file and the sql file for the extension. 

- The complete CMakeLists.txt will contain the details to build link and produce the extension shared object file.

- The `docker` folder will have the docker file which is being used to build the extension and test it in local. 

- The `docker-compose.yaml` file will have the running of the docker file and the pgadmin to have them easily run and test in local. 

