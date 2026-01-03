# oatpp-cuda-whisper

This project is an Oat++ microservice designed to demonstrate speech-to-text functionality, potentially leveraging CUDA for GPU acceleration. It includes a CPU mock worker and a GPU worker, suggesting it can operate in different modes depending on the availability of CUDA.

## Prerequisites

*   Docker and Docker Compose
*   A C++ development environment (CMake, make, C++ compiler) if building locally outside of Docker.

## Building and Running with Docker Compose

The easiest way to get the application up and running is by using Docker Compose. This will build the Docker image and start the service, exposing it on port `8000`.

1.  **Build and Run:**
    ```bash
    docker compose up --build -d
    ```
    The `-d` flag runs the services in the background.

2.  **Check Logs (Optional):**
    To view the application logs:
    ```bash
    docker compose logs -f
    ```

3.  **Stop the Services:**
    To stop and remove the containers, networks, and images created by `up`:
    ```bash
    docker compose down
    ```

## API Endpoints

The application exposes REST API endpoints.

### Hello World Endpoint

*   **URL:** `/hello`
*   **Method:** `GET`
*   **Description:** Returns a simple "Hello, World!" message.

**Example Request:**
```bash
curl http://localhost:8000/hello
```

**Example Response:**
```json
{
  "statusCode": 200,
  "message": "Hello, World!"
}
```

## Project Structure

*   `src/App.cpp`: Main application entry point, server setup.
*   `src/AppComponent.hpp`: Oat++ components configuration.
*   `src/controller/MyController.hpp`: Defines API endpoints, including `/hello`.
*   `src/dto/DTOs.hpp`: Data Transfer Objects definitions (e.g., `MessageDto`).
*   `src/worker/CpuMock.cpp`: Placeholder for CPU-based worker logic.
*   `src/worker/GpuWorker.cpp`: Placeholder for GPU-accelerated worker logic (CUDA).
*   `Dockerfile`: Defines the Docker image build process.
*   `docker-compose.yml`: Defines how to run the application using Docker Compose.
*   `CMakeLists.txt`: CMake build configuration.
