# Take-Home Assignment: Shared Memory Worker System with HTTP API and CUDA Audio Processing

## Objective

Build a **server application** that exposes an **HTTP API** (using [Oat++](https://oatpp.io/)) and internally communicates with a pool of **worker processes** using **shared memory IPC**. Workers must implement **CUDA-based mel spectrogram computation** for Whisper model preprocessing with **bidirectional streaming** support, meeting specified performance targets.

---

## Requirements

### 1. HTTP Server

* Use **Oat++** as the web framework.
* Expose a simple HTTP API:
  * `POST /process` → accepts a JSON body `{ "message": "<string>" }`
  * Returns `{ "result": "<string>" }` after the worker processes the message.
  * `POST /audio/stream` → accepts audio data (raw PCM or chunked format)
  * Returns mel spectrogram features compatible with Whisper tokenizer

### 2. Worker Processes

* Workers should be **separate processes** (not just threads).
* Server communicates with workers via **shared memory** (POSIX `shm_open` + `mmap` or equivalent).
* Use **synchronization primitives** (e.g., semaphores) to safely pass messages.
* Each worker should:
  * Wait for a task from shared memory.
  * Process the input string (example: reverse it, uppercase it, or add a prefix) **OR** process audio data.
  * Write the result back into shared memory.
  * Signal completion.

### 3. CUDA-Based Mel Spectrogram Processing

* Implement a **CUDA kernel** for mel spectrogram computation optimized for Whisper preprocessing:
  * Input: Raw audio (16kHz, mono PCM recommended)
  * Output: 80-channel mel spectrogram features (Whisper standard)
  * Use **STFT (Short-Time Fourier Transform)** followed by mel filterbank application
  * Must support Whisper's specific parameters:
    - Window size: 400 samples (25ms at 16kHz)
    - Hop length: 160 samples (10ms at 16kHz)
    - n_mels: 80
    - Frequency range: 0-8000 Hz

* **Bidirectional Streaming Architecture**:
  * **Stream 1 (Input)**: Audio chunks → CUDA processing pipeline
    - Support incremental audio input (streaming upload)
    - Maintain state across chunks (overlapping windows)
  * **Stream 2 (Output)**: Mel features → Client/Next stage
    - Stream computed mel spectrograms as they become available
    - Support partial results without waiting for entire audio file

* **CUDA Implementation Requirements**:
  * Use CUDA streams for concurrent processing
  * Implement efficient memory transfers (pinned memory, async copies)
  * Handle batch processing when multiple audio chunks arrive
  * Proper GPU memory management (avoid leaks, reuse buffers)

### 4. Task Dispatching

* The server must:
  * Place incoming requests into shared memory.
  * Assign them to **available workers**.
  * For audio tasks, route to CUDA-enabled workers.
  * Wait until the worker completes.
  * Return the result via the HTTP response.
  * Support streaming responses for audio processing tasks.

### 5. Concurrency

* The system must support **multiple HTTP requests in parallel**.
* Design a way to handle multiple workers (e.g., round-robin, simple queue).
* Ensure no race conditions in shared memory.
* **CUDA-specific**: Manage GPU access across workers (consider single GPU shared by workers or dedicated GPU per worker).

### 6. Performance Requirements

#### 6.1 Text Processing Performance

**Latency Targets** (measured from HTTP request received to response sent):
* **p50 (median)**: < 5ms
* **p95**: < 15ms
* **p99**: < 25ms

**Throughput Targets**:
* Minimum: **10,000 requests/second** with 4 workers
* Target: **20,000 requests/second** with 8 workers
* Message size: Up to 4KB text payload

**Test Conditions**:
* Concurrent clients: 100
* Test duration: 60 seconds minimum
* Mixed read/write workload

#### 6.2 Audio Processing Performance

**Latency Targets**:
* **First-Chunk Latency** (time to first mel spectrogram output): < 50ms
  - Measured from first audio chunk received to first mel frame available
  - Critical for real-time applications
* **Chunk Processing Latency**: < 20ms per 1-second audio chunk
  - p50: < 10ms
  - p95: < 20ms
  - p99: < 30ms
* **End-to-End Latency**: < 100ms for 10-second audio file
  - Includes HTTP overhead, IPC, GPU processing, response serialization

**Throughput Targets**:
* **Real-time factor**: Must achieve **> 10x real-time** processing speed
  - Process 10 seconds of audio in < 1 second
  - Target: 20-30x real-time on modern GPU (e.g., RTX 3090, A100)
* **Concurrent streams**: Support **≥ 8 concurrent audio streams** without degradation
* **Sustained throughput**: Process **≥ 480 seconds of audio per second** (8 streams × 60s each)

**CUDA-Specific Performance**:
* **GPU utilization**: Maintain **> 70%** GPU utilization during sustained load
* **Memory bandwidth**: Utilize **> 50%** of available GPU memory bandwidth
* **Kernel efficiency**: Individual kernels should achieve **> 40%** occupancy
* **Host-Device transfer overhead**: < 10% of total processing time

#### 6.3 Shared Memory IPC Performance

* **IPC overhead**: < 1ms per message (enqueue + dequeue)
* **Memory copy overhead**: Zero-copy where possible, < 0.5ms for 4KB payload
* **Semaphore wait time**: < 100μs under normal load (p95)
* **Queue saturation**: Handle bursts of 1000 requests without blocking

### 7. Simplicity

* Focus on correctness and clarity over complexity.
* Limit external dependencies to **Oat++**, **CUDA Toolkit**, POSIX APIs, and standard C++.
* Optional: cuFFT for STFT implementation (or custom CUDA kernel).

---

## Deliverables

1. **Source code** (C++17 or newer, CUDA 11.0+) with build instructions (CMake preferred).

2. **README.md** including:
   * How to build and run the server (including CUDA compilation).
   * How to start the worker processes.
   * Example curl command to test the API (text and audio endpoints).
   * **CUDA kernel explanation** (1 paragraph on optimization approach).

3. A **design explanation** (2–3 paragraphs) describing:
   * How shared memory communication is structured.
   * How synchronization between server and workers is handled.
   * **How bidirectional audio streaming is implemented** (buffering strategy, chunk coordination).
   * **GPU memory management strategy** across worker processes.

4. **Performance Report** including:
   * **Benchmarking methodology**: Tools used, test setup, hardware specs
   * **Latency measurements**: 
     - Text processing: p50, p95, p99 latencies
     - Audio processing: First-chunk, per-chunk, end-to-end latencies
     - Breakdown by component (HTTP, IPC, GPU processing)
   * **Throughput measurements**:
     - Requests/second for text processing
     - Real-time factor for audio processing
     - Maximum concurrent streams supported
   * **Resource utilization**:
     - CPU usage per worker
     - GPU utilization, memory usage, bandwidth
     - Shared memory efficiency
   * **Scalability analysis**: Performance vs. number of workers (1, 2, 4, 8)
   * **Bottleneck identification**: Where are the performance limits?
   * **Comparison tables**: Your results vs. target requirements

5. **Benchmarking scripts**:
   * Scripts to reproduce performance measurements
   * Load testing tools (e.g., using `wrk`, `ab`, or custom C++ client)
   * Audio generation scripts for testing
   * Automated performance validation

6. **Test audio file** or script to generate test data compatible with your implementation.

---

## Bonus Points

* Implement **streaming mode** for text processing (workers send partial results back).
* Implement **graceful shutdown** (server tells workers to exit cleanly, proper CUDA cleanup).
* Show awareness of performance:
  * Ring buffers instead of copying strings repeatedly
  * **CUDA optimization**: Kernel fusion, shared memory usage, warp-level optimizations
  * **Latency optimization**: First-chunk-to-first-output time minimization
  * **Adaptive batching**: Dynamically batch requests to maximize GPU throughput while meeting latency SLAs
* **Whisper integration**: Include the tokenization step after mel spectrogram (convert to tokens).
* **Multi-GPU support**: Distribute workers across multiple GPUs with load balancing.
* **Advanced benchmarking**: 
  - Flame graphs showing CPU/GPU bottlenecks
  - NSight profiling reports
  - Tail latency analysis under various load patterns
* **Performance optimizations**:
  - Implement kernel fusion (STFT + mel filterbank in single pass)
  - Use Tensor Cores if available
  - Implement prefetching and double-buffering
  - CUDA graph API for reduced launch overhead
* **Monitoring**: Real-time metrics endpoint (Prometheus-compatible)

---

## Evaluation Criteria

* **Correctness (25%)**: Does the system work as specified for both text and audio?
* **IPC Design (15%)**: Is shared memory + semaphores used correctly?
* **CUDA Implementation (25%)**: 
  - Mel spectrogram accuracy (compare with reference implementation)
  - Proper CUDA patterns (streams, async operations, memory management)
  - Bidirectional streaming correctness
* **Performance (20%)**: 
  - Meets or exceeds latency targets
  - Meets or exceeds throughput targets
  - Efficient resource utilization
  - Scalability demonstrated
* **Code Quality (10%)**: Is the code well-structured, readable, and maintainable?
* **Documentation (5%)**: Is it easy to build, run, and test?

---

## Performance Testing Guidelines

### Test Environment Specification

Candidates should report performance on hardware meeting these **minimum specifications**:

**CPU**: 
* 8+ cores (e.g., Intel i7-10700K, AMD Ryzen 7 5800X or better)
* 16GB+ RAM

**GPU**: 
* NVIDIA GPU with Compute Capability ≥ 7.5
* Examples: RTX 2080, RTX 3060, Tesla T4, A100
* 8GB+ VRAM

**OS**: Ubuntu 20.04/22.04 or similar Linux distribution

### Measurement Requirements

1. **Latency Measurement**:
   ```cpp
   // Use high-resolution timers
   auto start = std::chrono::high_resolution_clock::now();
   // ... operation ...
   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
   ```

2. **Percentile Calculation**:
   - Collect ≥ 10,000 samples for stable percentile estimates
   - Use proper percentile algorithms (not naive sorting for large datasets)
   - Report full distribution: min, p50, p90, p95, p99, p99.9, max

3. **Throughput Measurement**:
   - Measure under sustained load (≥ 60 seconds)
   - Report average throughput and standard deviation
   - Test with varying concurrency levels (1, 10, 50, 100, 200 clients)

4. **GPU Profiling**:
   ```bash
   # Use NVIDIA tools
   nvprof --print-gpu-trace ./worker
   nsys profile --stats=true ./worker
   ```

5. **System-wide metrics**:
   - Use `perf`, `htop`, `nvidia-smi` during load tests
   - Report CPU usage, memory usage, GPU utilization, GPU memory, power consumption

### Example Benchmark Output Format

```
=== Performance Benchmark Results ===

Hardware:
  CPU: AMD Ryzen 9 5950X (16 cores)
  GPU: NVIDIA RTX 3090 (24GB)
  RAM: 64GB DDR4-3600

Text Processing (4 workers, 100 concurrent clients, 60s test):
  Throughput: 18,453 req/s
  Latency:
    p50:   3.2ms ✓ (target: <5ms)
    p95:  12.1ms ✓ (target: <15ms)
    p99:  19.8ms ✓ (target: <25ms)
    max:  34.5ms

Audio Processing (8 workers, 10 concurrent streams):
  Real-time factor: 23.4x ✓ (target: >10x)
  First-chunk latency: 38ms ✓ (target: <50ms)
  Per-chunk latency:
    p50:   8.3ms ✓ (target: <10ms)
    p95:  16.2ms ✓ (target: <20ms)
    p99:  24.1ms ✓ (target: <30ms)
  
  GPU Metrics:
    Utilization: 78% ✓ (target: >70%)
    Memory BW: 58% ✓ (target: >50%)
    Kernel occupancy: 52% ✓ (target: >40%)

IPC Performance:
  Enqueue latency: 0.4ms ✓ (target: <1ms)
  Semaphore wait (p95): 45μs ✓ (target: <100μs)

Scalability:
  Workers    Text RPS    Audio RTF
  1          5,234       6.2x
  2          9,821      12.1x
  4         18,453      23.4x
  8         31,267      29.8x

✓ All performance targets met
```

---

## Reference Solution Outline (for evaluation)

### Architecture

```
          ┌───────────────────────────────────┐
          │         Oat++ HTTP Server         │
          │  POST /process {msg}              │
          │  POST /audio/stream {audio_chunk} │
          └────────────┬──────────────────────┘
                       │
                       ▼
            ┌───────────────────────┐     POSIX shm + sems
            │   Server/Dispatcher   │◄━━━━━━━━━━━━━━━━━━━━━━━┐
            │  - Text router        │                         │
            │  - Audio router       │                         │
            │  - Metrics collector  │                         │
            └────────┬──────────────┘                         │
                     │                                        │
                     ▼                                        │
   ┌──────────────────────────────────────────────┐           │
   │         SHARED MEMORY REGION                 │           │
   │ ┌───────────────┐  ┌────────────────────┐   │           │
   │ │ Request Ring  │  │  Response Slots    │   │           │
   │ │  (text+audio) │  │  [0..W-1]          │   │           │
   │ └───────────────┘  └────────────────────┘   │           │
   │ ┌────────────────────────────────────────┐  │           │
   │ │  Audio Stream Buffers (ring buffers)   │  │           │
   │ │  - Input chunks                        │  │           │
   │ │  - Output mel spectrograms             │  │           │
   │ └────────────────────────────────────────┘  │           │
   │ ┌────────────────────────────────────────┐  │           │
   │ │  Performance Counters                  │  │           │
   │ │  - Request timestamps                  │  │           │
   │ │  - Processing times                    │  │           │
   │ └────────────────────────────────────────┘  │           │
   └──────────────────────────────────────────────┘           │
                     ▲                                        │
                     │                                        │
      ┌──────────────┴────────────┐      ┌───────────────────┴──────────┐
      │   Worker #0 (CUDA)        │      │   Worker #N-1 (CUDA)         │
      │  ┌──────────────────────┐ │      │  ┌──────────────────────┐    │
      │  │ CUDA Mel Spectrogram │ │      │  │ CUDA Mel Spectrogram │    │
      │  │ - cuFFT / custom FFT │ │ ...  │  │ - cuFFT / custom FFT │    │
      │  │ - Mel filterbank     │ │      │  │ - Mel filterbank     │    │
      │  │ - Bidirectional      │ │      │  │ - Bidirectional      │    │
      │  │   streaming engine   │ │      │  │   streaming engine   │    │
      │  │ - Perf instrumentation│ │     │  │ - Perf instrumentation│   │
      │  └──────────────────────┘ │      │  └──────────────────────┘    │
      └───────────────────────────┘      └──────────────────────────────┘
```

### Shared Memory Layout (Extended)

```cpp
constexpr size_t RING_CAP        = 256;
constexpr size_t CHUNK_SIZE      = 4096;
constexpr size_t AUDIO_CHUNK_SIZE = 16384;  // ~1 sec at 16kHz
constexpr size_t MEL_FRAME_SIZE  = 80 * sizeof(float);

enum TaskType { TEXT_PROCESS, AUDIO_PROCESS };

struct ReqSlot {
  uint64_t  task_id;
  TaskType  type;
  uint32_t  len;
  uint64_t  enqueue_timestamp_ns;  // for latency tracking
  union {
    char  text_data[CHUNK_SIZE];
    struct {
      uint32_t sample_rate;
      uint32_t num_samples;
      int16_t  audio_data[AUDIO_CHUNK_SIZE];
    } audio;
  };
};

struct RespSlot {
  uint64_t  task_id;
  TaskType  type;
  uint32_t  len;
  bool      is_streaming;
  uint64_t  processing_time_ns;  // worker processing time
  uint64_t  gpu_time_ns;         // CUDA kernel time
  union {
    char  text_result[CHUNK_SIZE];
    float mel_features[80 * 100];
  };
};

struct SharedMem {
  std::atomic<size_t> head;
  std::atomic<size_t> tail;
  ReqSlot  req[RING_CAP];
  RespSlot resp_slots[MAX_WORKERS];
  
  // Streaming state per worker
  struct StreamState {
    std::atomic<bool> active;
    uint32_t sequence_num;
  } stream_state[MAX_WORKERS];
  
  // Performance metrics
  struct PerfCounters {
    std::atomic<uint64_t> total_requests;
    std::atomic<uint64_t> total_audio_samples;
    std::atomic<uint64_t> total_gpu_time_ns;
    std::atomic<uint64_t> queue_full_count;
  } perf_counters;
};
```

### CUDA Kernel Pseudocode with Performance Optimization

```cuda
// Optimized for low latency and high throughput
__global__ void compute_mel_spectrogram_fused(
    const float* audio,
    float* mel_output,
    int num_samples,
    int frame_offset,
    cudaEvent_t* timing_events  // for profiling
) {
    // Use shared memory for window coefficients
    __shared__ float window[400];
    __shared__ float mel_filters[80][201];  // Precomputed filters
    
    int frame_idx = blockIdx.x;
    int mel_bin = threadIdx.x;
    
    // Fused STFT + mel filterbank computation
    // Optimized for coalesced memory access
    // Use warp-level primitives for reduction
    
    // Compute STFT for this frame
    // Apply Hann window, FFT, compute magnitude
    // Apply mel filterbank in same kernel (fusion)
    // Write to mel_output[frame_idx * 80 + mel_bin]
}
```

### Sequence (Audio Processing with Performance Tracking)

1. **Server** accepts HTTP audio chunk, records `t0 = now()`, enqueues `{task_id, AUDIO, audio_data, t0}`.
2. **Worker** dequeues at `t1`, measures IPC latency = `t1 - t0`.
3. **Worker** transfers audio to GPU pinned memory at `t2`.
4. **Worker** launches CUDA kernel with timing events, completes at `t3`.
5. **Worker** streams partial results back, records `gpu_time = t3 - t2`.
6. **Server** receives response at `t4`, sends HTTP response at `t5`.
7. **Server** logs metrics: `total_latency = t5 - t0`, `gpu_time`, `ipc_time`.

---

## CUDA-Specific Notes for Evaluation

* Verify **correct mel spectrogram output** (compare with librosa or torchaudio reference).
* Check **CUDA error handling** (`cudaGetLastError`, synchronization).
* Ensure **no GPU memory leaks** (use `cuda-memcheck` or NSight).
* Validate **streaming correctness**: overlapping windows handled properly across chunks.
* Assess **performance against targets**: 
  - All latency targets met (p50, p95, p99)
  - Throughput targets achieved
  - GPU utilization > 70%
* Look for **CUDA best practices**: 
  - Coalesced memory access
  - Occupancy optimization
  - Use of shared memory
  - Stream concurrency
  - Proper event synchronization
* **Performance analysis depth**:
  - Bottleneck identification using profiling tools
  - Optimization rationale clearly documented
  - Trade-offs explained (e.g., latency vs. throughput)

---

## Additional Requirements

* Provide a **Dockerfile** or clear dependency list including CUDA Toolkit version.
* Include a **Python test client** that:
  - Sends audio and validates mel spectrogram output
  - Performs load testing with configurable concurrency
  - Calculates and reports latency percentiles
  - Validates against performance targets
* Document **GPU requirements** (minimum compute capability, memory needed).
* Provide **troubleshooting guide** for common performance issues.

---

## Performance Validation Script Example

```python
# perf_test.py - Provided by candidate
import requests
import numpy as np
import time
from concurrent.futures import ThreadPoolExecutor

def test_audio_latency(url, num_requests=1000):
    latencies = []
    for _ in range(num_requests):
        audio = generate_test_audio(16000)  # 1 second
        start = time.perf_counter_ns()
        response = requests.post(f"{url}/audio/stream", data=audio)
        end = time.perf_counter_ns()
        latencies.append((end - start) / 1e6)  # Convert to ms
    
    latencies.sort()
    print(f"p50: {latencies[len(latencies)//2]:.2f}ms")
    print(f"p95: {latencies[int(len(latencies)*0.95)]:.2f}ms")
    print(f"p99: {latencies[int(len(latencies)*0.99)]:.2f}ms")
    
    # Validate against targets
    assert latencies[len(latencies)//2] < 100, "p50 latency target missed"
    assert latencies[int(len(latencies)*0.95)] < 100, "p95 latency target missed"

if __name__ == "__main__":
    test_audio_latency("http://localhost:8000")