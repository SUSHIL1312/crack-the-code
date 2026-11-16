/**
 * @file free_list_allocator.cpp
 * @brief Complete educational implementation of a Free-List Memory Allocator in C++.
 *        Demonstrates block headers, first-fit searching, block splitting, and coalescing.
 *
 * Compilation:
 *   clang++ -std=c++20 -Wall -Wextra -Wpedantic free_list_allocator.cpp -o free_list_alloc
 *   ./free_list_alloc
 */

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cassert>

class FreeListAllocator {
public:
    struct BlockHeader {
        size_t size;          // Usable payload size in bytes
        bool is_free;         // Allocation status flag
        BlockHeader* next;    // Pointer to next physical block in memory
    };

    explicit FreeListAllocator(size_t total_size) 
        : arena_size_(total_size) {
        // Allocate raw backing memory arena
        memory_ = new uint8_t[arena_size_];
        
        // Initialize single massive free block covering the entire arena
        head_ = reinterpret_cast<BlockHeader*>(memory_);
        head_->size = arena_size_ - sizeof(BlockHeader);
        head_->is_free = true;
        head_->next = nullptr;
    }

    ~FreeListAllocator() {
        delete[] memory_;
    }

    // Disable copy semantics
    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

    void* allocate(size_t size) {
        // Enforce 8-byte alignment
        size = align8(size);

        BlockHeader* curr = head_;
        while (curr != nullptr) {
            if (curr->is_free && curr->size >= size) {
                // Check if block can be split into two
                if (curr->size >= size + sizeof(BlockHeader) + 8) {
                    split_block(curr, size);
                }
                curr->is_free = false;
                // Return pointer immediately after the header
                return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(curr) + sizeof(BlockHeader));
            }
            curr = curr->next;
        }

        std::cerr << "FreeListAllocator: Out of memory for size " << size << "\n";
        return nullptr; // Out of memory
    }

    void deallocate(void* ptr) {
        if (!ptr) return;

        // Retrieve header located immediately before payload
        BlockHeader* header = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<uint8_t*>(ptr) - sizeof(BlockHeader)
        );
        header->is_free = true;

        // Coalesce adjacent free blocks to eliminate external fragmentation
        coalesce();
    }

    void dump_state() const {
        std::cout << "\n--- Memory Arena State ---\n";
        const BlockHeader* curr = head_;
        int block_idx = 0;
        size_t total_free = 0;
        size_t total_used = 0;

        while (curr != nullptr) {
            std::cout << "Block " << block_idx++ << ": "
                      << (curr->is_free ? "[FREE] " : "[USED] ")
                      << "Payload Size = " << curr->size << " bytes "
                      << "(Addr: " << curr << ")\n";
            if (curr->is_free) total_free += curr->size;
            else total_used += curr->size;
            curr = curr->next;
        }
        std::cout << "Summary: Used = " << total_used << " bytes, Free = " << total_free << " bytes\n";
        std::cout << "--------------------------\n";
    }

private:
    uint8_t* memory_;
    size_t arena_size_;
    BlockHeader* head_;

    static size_t align8(size_t n) {
        return (n + 7) & ~static_cast<size_t>(7);
    }

    void split_block(BlockHeader* block, size_t requested_size) {
        size_t remaining_size = block->size - requested_size - sizeof(BlockHeader);

        // Position new header immediately following the allocated payload
        BlockHeader* new_block = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<uint8_t*>(block) + sizeof(BlockHeader) + requested_size
        );
        new_block->size = remaining_size;
        new_block->is_free = true;
        new_block->next = block->next;

        block->size = requested_size;
        block->next = new_block;
    }

    void coalesce() {
        BlockHeader* curr = head_;
        while (curr != nullptr && curr->next != nullptr) {
            if (curr->is_free && curr->next->is_free) {
                // Merge adjacent free blocks
                curr->size += sizeof(BlockHeader) + curr->next->size;
                curr->next = curr->next->next;
                // Do not advance curr; check if newly expanded block can merge with next
            } else {
                curr = curr->next;
            }
        }
    }
};

int main() {
    std::cout << "Initializing 1024-byte FreeListAllocator...\n";
    FreeListAllocator allocator(1024);
    allocator.dump_state();

    std::cout << "\n1. Allocating ptrA (128 bytes), ptrB (256 bytes), ptrC (64 bytes)...\n";
    void* ptrA = allocator.allocate(128);
    void* ptrB = allocator.allocate(256);
    void* ptrC = allocator.allocate(64);
    allocator.dump_state();

    std::cout << "\n2. Freeing ptrB (middle block of 256 bytes)...\n";
    allocator.deallocate(ptrB);
    allocator.dump_state();

    std::cout << "\n3. Freeing ptrA (adjacent to free block ptrB). Notice automatic coalescing!\n";
    allocator.deallocate(ptrA);
    allocator.dump_state();

    std::cout << "\n4. Freeing ptrC (all blocks returned, arena coalesces into 1 block)...\n";
    allocator.deallocate(ptrC);
    allocator.dump_state();

    return 0;
}
