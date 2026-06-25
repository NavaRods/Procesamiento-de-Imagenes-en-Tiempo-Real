# Procesamiento de Imagénes en Tiempo Real (Concurrencia & Paralelismo)

Este proyecto es un sistema de procesamiento de imágenes y video en tiempo real diseñado para evaluar y comparar el impacto del **Procesamiento Secuencial (Single-Core)** frente al **Procesamiento Paralelo (Multi-Core con OpenMP)**. 

El sistema cuenta con un backend de alto rendimiento desarrollado en **C++** y una interfaz web interactiva en **HTML5/JavaScript** comunicados mediante **WebSockets** de baja latencia.

## Arquitectura del Sistema

El proyecto implementa un modelo **Productor-Consumidor** desacoplado mediante hilos:
* **Hilo Productor:** Captura frames mediante OpenCV, aplica transformaciones matriciales píxel por píxel y los comprime en JPEG.
* **Hilo Transmisor / Servidor:** Gestiona las conexiones WebSocket entrantes y despacha los datos binarios a los clientes.
* **Frontend:** Decodifica los arrays binarios a Blobs de imagen en tiempo real y calcula los FPS reales del cliente.

## Tecnologías Utilizadas

### Backend (C++)
* **OpenCV (v4.x+):** Captura de video y manipulación de matrices de imágenes.
* **OpenMP:** Paralelización del bucle de procesamiento de píxeles a nivel de hardware.
* **Crow:** Framework micro-web en C++ para ruteo de WebSockets.
* **Asio:** Gestión de operaciones de I/O asíncronas para redes.
* **CMake:** Sistema de automatización de compilación cross-platform.

### Frontend
* **HTML5 & CSS3:** Interfaz responsiva con temática oscura (*Dashboard Moderno*).
* **JavaScript Nativo:** Cliente WebSocket con gestión eficiente de buffers de memoria (`Blob` y `ObjectURL`).

---

## Requisitos Previos

Antes de compilar, asegúrate de tener instaladas las siguientes dependencias en tu entorno (ejemplo para Windows/MinGW o Linux):

* Compilador con soporte para **C++17** y **OpenMP** (GCC, Clang o MSVC).
* **CMake** (Versión 3.10 o superior).
* Librería de **OpenCV** correctamente vinculada en las variables de entorno.
* Librería **Asio** (Header-only).
* El archivo de cabecera `crow_all.h` incorporado en el directorio del proyecto.

---

## 🔧 Compilación e Instalación

# 1. Clonar el repositorio:
```bash
git clone https://github.com/NavaRods/Procesamiento-de-Imagenes-en-Tiempo-Real.git
cd Procesamiento-de-Im-genes-en-Tiempo-Real
