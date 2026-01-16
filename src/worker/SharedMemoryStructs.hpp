#ifndef SharedMemoryStructs_hpp
#define SharedMemoryStructs_hpp

#include <cstdint>
#include <atomic>
#include <cstddef>

namespace app { namespace worker {

constexpr size_t RING_CAP        = 256;
constexpr size_t TEXT_CHUNK_SIZE = 4096;
// Whisper usually takes 16kHz audio.
// 1 second buffer = 16000 samples.
// float = 4 bytes. 16000 * 4 = 64KB.
// This is reasonable for SHM.
constexpr size_t AUDIO_CHUNK_SIZE = 16000; 
constexpr size_t MAX_WORKERS     = 8;
constexpr char SHM_NAME[]        = "/oatpp_whisper_shm";
constexpr char SEM_REQ_NAME[]    = "/oatpp_whisper_sem_req";
constexpr char SEM_RESP_NAME[]   = "/oatpp_whisper_sem_resp";

enum TaskType : uint32_t {
    TASK_TEXT_PROCESS = 0,
    TASK_AUDIO_PROCESS = 1,
    TASK_SHUTDOWN = 99
};

struct ReqSlot {
    uint64_t  task_id;
    TaskType  type;
    uint32_t  len;
    uint64_t  enqueue_timestamp_ns;  // for latency tracking
    union {
        char  text_data[TEXT_CHUNK_SIZE];
        struct {
            uint32_t sample_rate;
            uint32_t num_samples;
            float    audio_data[AUDIO_CHUNK_SIZE];
        } audio;
    };
};

struct RespSlot {
    uint64_t  task_id;
    TaskType  type;
    uint32_t  len;
    uint32_t  status_code; // 0 = success
    uint64_t  processing_time_ns;  // worker processing time
    union {
        char  text_result[TEXT_CHUNK_SIZE];
        // Mel spectrogram: 80 mels * frames.
        // If we process 1 sec of audio (16000 samples)
        // Hop length 160 => 100 frames.
        // 80 * 100 = 8000 floats.
        float mel_features[80 * 100];
    };
};

struct SharedMem {
    std::atomic<size_t> req_head;
    std::atomic<size_t> req_tail; // Unused in my logic currently? Or used for logic?
    
    // We used explicit indices in IPC.cpp
    std::atomic<size_t> req_write_idx; // Where producer puts next item
    std::atomic<size_t> req_read_idx;  // Where consumer takes next item
    
    std::atomic<size_t> resp_write_idx;
    std::atomic<size_t> resp_read_idx;

    ReqSlot  req_ring[RING_CAP];
    RespSlot resp_ring[RING_CAP];
};

}}

#endif