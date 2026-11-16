#pragma once

#include <cstdint>
#include <cstddef>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>

constexpr size_t IPC_CAPACITY = 64;

struct IPCMessage {
    uint64_t seq;
    uint32_t payload_len;
    char data[64];
};

struct SharedMemoryChannel {
    size_t head; // Producer index
    size_t tail; // Consumer index
    IPCMessage buffer[IPC_CAPACITY];
};

class IPCProducerConsumer {
public:
    IPCProducerConsumer(const std::string& prefix = "ctc_ipc")
        : sem_empty_name_("/" + prefix + "_empty"),
          sem_full_name_("/" + prefix + "_full"),
          sem_mutex_name_("/" + prefix + "_mutex"),
          sem_empty_(SEM_FAILED),
          sem_full_(SEM_FAILED),
          sem_mutex_(SEM_FAILED),
          shm_channel_(nullptr) {

        // Clean up any stale semaphores from previous runs
        sem_unlink(sem_empty_name_.c_str());
        sem_unlink(sem_full_name_.c_str());
        sem_unlink(sem_mutex_name_.c_str());

        // Create named semaphores
        sem_empty_ = sem_open(sem_empty_name_.c_str(), O_CREAT | O_EXCL, 0644, IPC_CAPACITY);
        sem_full_ = sem_open(sem_full_name_.c_str(), O_CREAT | O_EXCL, 0644, 0);
        sem_mutex_ = sem_open(sem_mutex_name_.c_str(), O_CREAT | O_EXCL, 0644, 1);

        if (sem_empty_ == SEM_FAILED || sem_full_ == SEM_FAILED || sem_mutex_ == SEM_FAILED) {
            perror("sem_open");
            throw std::runtime_error("Failed to initialize POSIX semaphores");
        }

        // Map anonymous shared memory (inheritable across fork)
        void* ptr = mmap(nullptr, sizeof(SharedMemoryChannel),
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANON, -1, 0);

        if (ptr == MAP_FAILED) {
            perror("mmap");
            throw std::runtime_error("Failed to map shared memory");
        }

        shm_channel_ = static_cast<SharedMemoryChannel*>(ptr);
        shm_channel_->head = 0;
        shm_channel_->tail = 0;
    }

    ~IPCProducerConsumer() {
        if (shm_channel_) {
            munmap(shm_channel_, sizeof(SharedMemoryChannel));
        }
        if (sem_empty_ != SEM_FAILED) sem_close(sem_empty_);
        if (sem_full_ != SEM_FAILED) sem_close(sem_full_);
        if (sem_mutex_ != SEM_FAILED) sem_close(sem_mutex_);

        sem_unlink(sem_empty_name_.c_str());
        sem_unlink(sem_full_name_.c_str());
        sem_unlink(sem_mutex_name_.c_str());
    }

    void produce(const IPCMessage& msg) {
        sem_wait(sem_empty_);
        sem_wait(sem_mutex_);

        shm_channel_->buffer[shm_channel_->head] = msg;
        shm_channel_->head = (shm_channel_->head + 1) % IPC_CAPACITY;

        sem_post(sem_mutex_);
        sem_post(sem_full_);
    }

    IPCMessage consume() {
        sem_wait(sem_full_);
        sem_wait(sem_mutex_);

        IPCMessage msg = shm_channel_->buffer[shm_channel_->tail];
        shm_channel_->tail = (shm_channel_->tail + 1) % IPC_CAPACITY;

        sem_post(sem_mutex_);
        sem_post(sem_empty_);

        return msg;
    }

private:
    std::string sem_empty_name_;
    std::string sem_full_name_;
    std::string sem_mutex_name_;

    sem_t* sem_empty_;
    sem_t* sem_full_;
    sem_t* sem_mutex_;

    SharedMemoryChannel* shm_channel_;
};
