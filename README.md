# oatpp-cuda-whisper

This project is an Oat++ microservice designed to demonstrate speech-to-text functionality, potentially leveraging CUDA for GPU acceleration. It includes a CPU mock worker and a GPU worker, suggesting it can operate in different modes depending on the availability of CUDA.

## Prerequisites

*   Docker and Docker Compose
*   A C++ development environment (CMake, make, C++ compiler) if building locally outside of Docker.

## Building and Running with Docker Compose

The easiest way to get the application up and running is by using Docker Compose. This uses a secure **multi-stage Docker build** to compile the application and create a minimal runtime image.

1.  **Build and Run:**
    ```bash
    docker compose up --build -d
    ```
    The `-d` flag runs the services in the background.

    > **Note on CUDA Support:** The Dockerfile automatically detects if `nvcc` is available in the build environment. If not found (default), it builds the **CPU Mock version**. To enable GPU support, you would need to use a base image with CUDA development tools in the `builder` stage of the `Dockerfile`.

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

## Security Features

This project implements several security best practices to ensure robustness and safety:

*   **Non-Root Execution:** The application runs as a dedicated non-root user (`appuser`) inside the container to minimize the impact of potential container escapes.
*   **Multi-Stage Build:** The Docker image is built using a multi-stage process, ensuring that build tools and intermediate files are not present in the final runtime image, reducing the attack surface.
*   **Input Validation:** Strict validation is enforced on API inputs, including checks for `Content-Type` and message length limits to prevent Denial of Service (DoS) attacks.
*   **Secure Error Handling:** The application returns generic error messages to clients to prevent information leakage, while logging detailed exception information internally for debugging.
*   **Compiler Hardening:** The build configuration includes security-hardening flags (e.g., `-fstack-protector-strong`, `_FORTIFY_SOURCE=2`) to protect against common binary exploitation techniques.

## Running Tests

The project includes unit and integration tests covering the controller logic and API endpoints.

### Running Tests with Docker

To run tests in a Docker environment, you need to use the `builder` stage of the Docker image, which contains all the necessary build tools and the compiled test executable.

1.  **Build the `builder` image:**
    ```bash
    docker build --target builder -t oatpp-cuda-whisper-builder .
    ```
2.  **Run the tests:**
    ```bash
    docker run --rm oatpp-cuda-whisper-builder /app/build/my-tests
    ```
    This command creates a temporary container from the `builder` image, runs the `my-tests` executable, and then removes the container.

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

### Generating Coverage with Docker

Similar to running tests, generating coverage reports requires the `builder` image.

1.  **Ensure the `builder` image is built:**
    ```bash
    docker build --target builder -t oatpp-cuda-whisper-builder .
    ```
2.  **Run the coverage script within a temporary container:**
    ```bash
    docker run --rm -v $(pwd):/app oatpp-cuda-whisper-builder sh -c "cd /app && chmod +x run_tests_with_coverage.sh && ./run_tests_with_coverage.sh clean"
    ```
    This command will:
    *   Mount your local project directory into `/app` inside the container.
    *   Change into the mounted directory.
    *   Make the `run_tests_with_coverage.sh` script executable.
    *   Execute the script.
    The coverage report will be generated in your local `build_coverage/coverage_report` directory.

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
*   `Dockerfile`: Docker build definition (Multi-stage).
*   `docker-compose.yml`: Container orchestration config.
*   `CMakeLists.txt`: Build configuration.
