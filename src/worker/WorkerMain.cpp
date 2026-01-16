#include "WorkerMain.hpp"
#include "IPC.hpp"
#include "Bridge.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <cstring>
#include <vector>

namespace app { namespace worker {

void processText(const ReqSlot& req, RespSlot& resp) {
    // Example: Reverse the string
    size_t len = strnlen(req.text_data, TEXT_CHUNK_SIZE);
    len = std::min(len, (size_t)TEXT_CHUNK_SIZE - 1);
    
    std::string s(req.text_data, len);
    std::reverse(s.begin(), s.end());
    
    strncpy(resp.text_result, s.c_str(), TEXT_CHUNK_SIZE);
    resp.len = s.length();
}

void processAudio(const ReqSlot& req, RespSlot& resp) {
    // Convert raw array to vector for the existing interface
    std::vector<float> input(req.audio.audio_data, req.audio.audio_data + req.audio.num_samples);
    std::vector<float> output;
    
    AudioWorker worker;
    worker.computeMelSpectrogram(input, output);
    
    // Copy back
    // Capacity of mel_features is 80 * 100 = 8000 floats
    size_t maxFloats = 80 * 100;
    size_t copyLen = std::min(output.size(), maxFloats); 
    
    for(size_t i=0; i<copyLen; ++i) {
        resp.mel_features[i] = output[i];
    }
    resp.len = copyLen;
}

void runWorker() {
    IPC ipc;
    try {
        ipc.initWorker();
    } catch(const std::exception& e) {
        std::cerr << "Worker failed to init IPC: " << e.what() << std::endl;
        return;
    }

    std::cout << "Worker process started. Waiting for tasks..." << std::endl;

    ReqSlot req;
    while (true) {
        if (ipc.waitForRequest(req)) {
            if (req.type == TASK_SHUTDOWN) {
                std::cout << "Worker received shutdown signal." << std::endl;
                break;
            }

            RespSlot resp;
            resp.task_id = req.task_id;
            resp.type = req.type;
            resp.status_code = 0;
            
            auto start = std::chrono::high_resolution_clock::now();

            if (req.type == TASK_TEXT_PROCESS) {
                processText(req, resp);
            } else if (req.type == TASK_AUDIO_PROCESS) {
                processAudio(req, resp);
            } else {
                resp.status_code = 400; // Unknown task
            }

            auto end = std::chrono::high_resolution_clock::now();
            resp.processing_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

            ipc.submitResponse(resp);
        }
    }
    
    ipc.cleanup();
}

}}
