#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

enum class AllocationPolicy {
    FIRST_FIT,
    BEST_FIT
};

struct BlockHeader {
    uint32_t size;     // Total block size including header and footer
    uint32_t is_allocated; // 1 = allocated, 0 = free
    BlockHeader* next_free;
    BlockHeader* prev_free;
};

struct BlockFooter {
    uint32_t size;
    uint32_t is_allocated;
};

class MemoryAllocator {
public:
    explicit MemoryAllocator(size_t heap_size_bytes = 64 * 1024);
    ~MemoryAllocator();

    // Disable copy/move
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    void* allocate(size_t size, AllocationPolicy policy = AllocationPolicy::FIRST_FIT);
    void deallocate(void* ptr);

    size_t get_total_free_bytes() const;
    size_t get_largest_free_block() const;
    void print_heap_state() const;

private:
    void init_heap();
    BlockHeader* find_fit(size_t total_size, AllocationPolicy policy);
    void split_block(BlockHeader* block, size_t total_size);
    BlockHeader* coalesce(BlockHeader* block);

    void insert_free_block(BlockHeader* block);
    void remove_free_block(BlockHeader* block);

    std::vector<uint8_t> heap_storage_;
    uint8_t* heap_start_;
    uint8_t* heap_end_;
    BlockHeader* free_list_head_;
};
