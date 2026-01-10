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

    > **Note on CUDA Support:** By default, the Docker configuration builds the CPU Mock version (`ENABLE_CUDA=OFF` in `docker-compose.yml`). To enable GPU support (CUDA), ensure you have the **NVIDIA Container Toolkit** installed and configured, then update the `command` in `docker-compose.yml` to use `cmake -DENABLE_CUDA=ON ..`.

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

The application exposes REST API endpoints. All successful responses follow a standardized JSON wrapper format.

### Standard Response Format
```json
{
  "code": 200,
  "is_success": true,
  "message": "success",
  "duration": "23.45ms",
  "result": { ... } 
}
```

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
  "code": 200,
  "is_success": true,
  "message": "success",
  "duration": "0.015ms",
  "result": {
    "status_code": 200,
    "message": "Hello, World!"
  }
}
```

### Process Audio Endpoint

*   **URL:** `/process`
*   **Method:** `POST`
*   **Content-Type:** `application/json`
*   **Body:**
    ```json
    {
      "message": "Audio data simulation"
    }
    ```

**Example Response:**
```json
{
  "code": 200,
  "is_success": true,
  "message": "success",
  "duration": "500.20ms",
  "result": {
    "transcript": "Audio data simulation"
  }
}
```

## Running Tests

The project includes unit and integration tests covering the controller logic and API endpoints.

### Using Docker (Recommended)

Ensure the application is running:
```bash
docker compose up --build -d
```

Then execute the tests inside the container:
```bash
docker compose exec app ./build/my-tests
```

### Local Build

1.  **Build the Tests:**
    ```bash
    mkdir -p build && cd build
    cmake ..
    make my-tests
    ```

2.  **Run the Tests:**
    ```bash
    ./my-tests
    ```

## Code Coverage

To generate code coverage reports (using `gcov` and `lcov`), you can use the provided helper script.

### Using Docker (Recommended)

> **Important:** If you've modified `CMakeLists.txt` or `Dockerfile` (e.g., to enable coverage or install `lcov`), ensure you rebuild your Docker image first: `docker compose up --build -d`.

1.  **Ensure the container is running with the latest build:**
    ```bash
    docker compose up --build -d
    ```

2.  **Run the coverage script inside the container:**
    ```bash
    docker compose exec app sh run_tests_with_coverage.sh clean
    ```

3.  **Copy the report to host (optional):**
    ```bash
    docker cp $(docker compose ps -q app):/app/build_coverage/coverage_report ./coverage_report
    ```
    This will copy the HTML report to a new `coverage_report` directory in your project's root on the host. Open `coverage_report/index.html` in your browser to view the results.

### Local Environment

Requirements: `cmake`, `gcov`, `lcov`.

```bash
chmod +x run_tests_with_coverage.sh
./run_tests_with_coverage.sh clean
```
The report will be generated in `build_coverage/coverage_report/index.html`.

## Project Structure

The project follows a modular Clean Architecture approach:

*   `src/App.cpp`: Main application entry point.
*   `src/AppConfig.hpp`: Configuration component.
*   `src/AppComponent.hpp`: Dependency Injection container & wiring.
*   `src/controller/`: REST API Controllers.
    *   `MyController.hpp`: Main controller using `AudioService`.
*   `src/dto/`: Data Transfer Objects (DTOs).
    *   `BaseResponseDto.hpp`: Standard API response wrapper.
    *   `MessageDto.hpp`, `ProcessDto.hpp`, `ErrorDto.hpp`.
*   `src/service/`: Business Logic Layer.
    *   `AudioService.cpp`: Handles audio processing logic.
*   `src/worker/`: Infrastructure/Hardware Layer.
    *   `CpuMock.cpp`: Mock implementation for development.
    *   `GpuWorker.cpp`: CUDA implementation for production.
*   `src/validator/`: Input validation helpers.
*   `src/errorhandler/`: Centralized HTTP error handling.
*   `src/exception/`: Custom application exceptions.
*   `src/utils/`: Utilities (e.g., ExecutionTimer).
*   `test/`: Unit and Integration tests.
    *   `MyControllerTest.cpp`: Integration tests for API endpoints.
    *   `tests.cpp`: Test runner entry point.
    *   `TestAppComponent.hpp`: Test-specific DI container.
*   `Dockerfile`: Docker build definition.
*   `docker-compose.yml`: Container orchestration config.
*   `CMakeLists.txt`: Build configuration.
