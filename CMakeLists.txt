cmake_minimum_required(VERSION 3.10)
project(ProyectoConcurrente)

# Forzar C++17 para compatibilidad con Crow
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar paquetes
find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)
find_package(asio REQUIRED)

# Añadir el ejecutable
add_executable(app main.cpp)

# Vincular librerías
target_link_libraries(app PRIVATE 
    ${OpenCV_LIBS} 
    OpenMP::OpenMP_CXX 
    asio                # Referencia al target de Asio
    ws2_32              # Libreria de sockets de Windows (Winsock2)
    mswsock             # Libreria de sockets de Windows (MSWSOCK)
)