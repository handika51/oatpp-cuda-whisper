# oatpp-cuda-whisper

This project is an Oat++ microservice designed to demonstrate speech-to-text functionality, leveraging a shared memory worker architecture to potentially use CUDA for GPU acceleration. It includes a CPU mock worker and a GPU worker, allowing operation in different modes depending on the availability of CUDA.

## Prerequisites

*   Docker and Docker Compose
*   A C++ development environment (CMake, make, C++ compiler) if building locally outside of Docker.

## Architecture

The system uses a **Split-Process Architecture** where the HTTP server and the audio processing workers run as separate processes, communicating via POSIX **Shared Memory (IPC)** and Semaphores.

*   **Server Process:** Handles HTTP requests, validation, and dispatches tasks to the Request Ring Buffer in shared memory.
*   **Worker Processes:** Poll the shared memory for tasks (Text or Audio), process them (potentially using CUDA), and write results back to the Response Ring Buffer.

This design ensures that heavy CUDA initialization or crashes in a worker do not directly bring down the HTTP server.

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

## CUDA Kernel Implementation

The GPU worker (`src/worker/GpuWorker.cu`) implements a high-performance audio preprocessing pipeline optimized for Whisper.

*   **Pipeline:** Raw Audio -> Windowing -> FFT (R2C) -> Magnitude Squared -> Mel Filterbank -> Log10 -> Output.
*   **Optimization:**
    *   **Lazy Initialization:** Mel filterbank weights and Hann window are precomputed and uploaded to the GPU only once.
    *   **cuFFT:** Uses the optimized `cuFFT` library for batched Fast Fourier Transforms (R2C).
    *   **Custom Kernels:** 
        *   `applyWindowKernel`: Efficiently slices input audio into overlapping frames and applies the Hann window in parallel.
        *   `magnitudeAndMelKernel`: Fuses magnitude calculation, matrix multiplication (Mel filterbank application), and log scaling into a single kernel to minimize memory bandwidth usage.
    *   **Memory Management:** Reuses device buffers to avoid allocation overhead during streaming.

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

### Process Text Endpoint

*   **URL:** `/process`
*   **Method:** `POST`
*   **Content-Type:** `application/json`
*   **Body:**
    ```json
    {
      "message": "Text to be processed"
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
    "result": "dessecorp eb ot txeT" // Example processing (reverse string)
  }
}
```

### Audio Stream Endpoint

*   **URL:** `/audio/stream`
*   **Method:** `POST`
*   **Content-Type:** `application/octet-stream`
*   **Body:** Raw PCM 16-bit mono 16kHz audio data.

**Example Request:**
```bash
# Send binary audio data
curl -X POST --data-binary "@test_audio.raw" http://localhost:8000/audio/stream
```

**Example Response:**
```json
{
  "code": 200,
  "is_success": true,
  "message": "success",
  "result": {
    "sample_count": 16000,
    "features": [ ... 80-channel mel spectrogram data ... ]
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

*   `src/App.cpp`: Main application entry point (Server & Worker launcher).
*   `src/AppConfig.hpp`: Configuration component.
*   `src/AppComponent.hpp`: Dependency Injection container & wiring.
*   `src/controller/`: REST API Controllers.
    *   `MyController.hpp`: Main controller using `AudioService`.
*   `src/dto/`: Data Transfer Objects (DTOs).
    *   `BaseResponseDto.hpp`: Standard API response wrapper.
    *   `MessageDto.hpp`, `ProcessDto.hpp`, `ErrorDto.hpp`.
*   `src/service/`: Business Logic Layer.
    *   `AudioService.cpp`: Dispatches tasks to `WorkerManager`.
*   `src/worker/`: Infrastructure/Hardware Layer & IPC.
    *   `WorkerManager.hpp`: Manages worker processes and task futures.
    *   `WorkerMain.cpp`: Worker process entry point and logic.
    *   `IPC.hpp`: Shared memory and semaphore wrapper.
    *   `SharedMemoryStructs.hpp`: Definition of Ring Buffers and Task Slots.
    *   `CpuMock.cpp`: Mock implementation for development.
    *   `GpuWorker.cu`: CUDA implementation for production.
*   `src/validator/`: Input validation helpers.
*   `src/errorhandler/`: Centralized HTTP error handling.
*   `src/exception/`: Custom application exceptions.
*   `src/utils/`: Utilities (e.g., ExecutionTimer).
*   `test/`: Unit and Integration tests.
    *   `AudioServiceTest.cpp`: Tests service logic and worker IPC.
    *   `tests.cpp`: Test runner entry point.
*   `Dockerfile`: Docker build definition (Multi-stage).
*   `docker-compose.yml`: Container orchestration config.
*   `CMakeLists.txt`: Build configuration.