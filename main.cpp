#define CROW_MAIN
#define ASIO_STANDALONE
#include "crow_all.h"
#include <opencv2/opencv.hpp>
#include <omp.h>
#include <mutex>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;
using namespace cv;

// --- VARIABLES GLOBALES (Recurso Compartido) ---
mutex frame_mutex;
vector<uchar> compressed_buffer;
bool is_parallel = true;
double current_fps = 0;

// --- FUNCIÓN DE PROCESAMIENTO (FILTROS) ---
void aplicar_filtros(Mat& frame) {
    int rows = frame.rows;
    int cols = frame.cols;

    // Paralelización de datos con OpenMP
    #pragma omp parallel for collapse(2) if(is_parallel)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Vec3b& pixel = frame.at<Vec3b>(i, j);
            
            // Ejemplo: Filtro de inversión de color (Negativo)
            // Esto demuestra el procesamiento intensivo por píxel
            pixel[0] = 255 - pixel[0]; // B
            pixel[1] = 255 - pixel[1]; // G
            pixel[2] = 255 - pixel[2]; // R
        }
    }
}

// --- HILO PRODUCTOR: CAPTURA Y PROCESAMIENTO ---
void capture_loop() {
    VideoCapture cap(0, CAP_DSHOW); // Abre la cámara por defecto
    if (!cap.isOpened()) {
        cerr << "Error: No se pudo abrir la cámara." << endl;
        return;
    }

    Mat frame;
    while (true) {
        auto start = chrono::high_resolution_clock::now();

        cap >> frame;
        if (frame.empty()) break;

        // 1. Procesamiento Paralelo
        aplicar_filtros(frame);

        // 2. Compresión JPEG (Optimiza el ancho de banda del WebSocket)
        vector<uchar> local_buf;
        vector<int> params = {IMWRITE_JPEG_QUALITY, 70}; // Calidad balanceada
        imencode(".jpg", frame, local_buf, params);

        // 3. Sección Crítica: Actualizar buffer global de forma segura
        {
            lock_guard<mutex> lock(frame_mutex);
            compressed_buffer = move(local_buf);
        }

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> diff = end - start;
        current_fps = 1.0 / diff.count();

        // Control de frame rate para no saturar
        this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void timer_loop(crow::websocket::connection& conn) {
    // Usamos un puntero a la conexión para mayor seguridad
    auto conn_ptr = &conn;

    std::thread([conn_ptr]() {
        // En versiones nuevas de Crow, no hay un is_alive() directo fiable
        // Así que usamos un bucle que se detendrá cuando Crow cierre la conexión
        try {
            while (true) {
                std::string frame_to_send;
                {
                    std::lock_guard<std::mutex> lock(frame_mutex);
                    if (!compressed_buffer.empty()) {
                        frame_to_send = std::string(compressed_buffer.begin(), compressed_buffer.end());
                    }
                }

                if (!frame_to_send.empty()) {
                    // Enviar binario. Si la conexión murió, esto lanzará una excepción y saldremos del hilo.
                    conn_ptr->send_binary(frame_to_send);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(40)); // ~25 FPS estables
            }
        } catch (...) {
            // Si hay error al enviar (conexión cerrada), el hilo termina aquí
            std::cout << "Hilo de transmisión finalizado para un cliente." << std::endl;
        }
    }).detach();
}

// --- MAIN: SERVIDOR Y CONCURRENCIA ---
int main() {
    cout << "Optimizando para: " << omp_get_max_threads() << " hilos de CPU." << endl;

    // Iniciar hilo de captura
    thread worker(capture_loop);
    worker.detach();

    crow::SimpleApp app;

    // Ruta del WebSocket
    CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([&](crow::websocket::connection& conn) {
            cout << "Cliente conectado" << endl;
            // Creamos un temporizador para enviar frames a esta conexión
            // cada 33ms (aprox 30 FPS)
            timer_loop(conn); 
        })
        .onmessage([&](crow::websocket::connection& conn, const string& data, bool is_binary) {
            if (data == "toggle") is_parallel = !is_parallel;
        });

    // Hilo de transmisión (Broadcaster)
    thread broadcaster([&]() {
        while (true) {
            this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
            
            string frame_to_send;
            {
                lock_guard<mutex> lock(frame_mutex);
                if (!compressed_buffer.empty()) {
                    frame_to_send = string(compressed_buffer.begin(), compressed_buffer.end());
                }
            }

            if (!frame_to_send.empty()) {
                // Enviar frames a todos los clientes conectados (simplificado)
                // En una app real, manejaríamos una lista de conexiones
            }
        }
    });
    broadcaster.detach();

    // Iniciar servidor en el puerto 8080
    app.port(8080).multithreaded().run();

    return 0;
}