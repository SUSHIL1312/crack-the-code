#include "allocator.hpp"
#include <iostream>
#include <cstring>
#include <cassert>

constexpr size_t ALIGNMENT = 16;
constexpr size_t MIN_BLOCK_SIZE = sizeof(BlockHeader) + sizeof(BlockFooter) + ALIGNMENT;

static size_t align_up(size_t n, size_t alignment) {
    return (n + alignment - 1) & ~(alignment - 1);
}

static BlockFooter* get_footer(BlockHeader* header) {
    uint8_t* block_end = reinterpret_cast<uint8_t*>(header) + header->size;
    return reinterpret_cast<BlockFooter*>(block_end - sizeof(BlockFooter));
}

static BlockHeader* get_next_physical(BlockHeader* header, uint8_t* heap_end) {
    uint8_t* next = reinterpret_cast<uint8_t*>(header) + header->size;
    if (next >= heap_end) return nullptr;
    return reinterpret_cast<BlockHeader*>(next);
}

static BlockHeader* get_prev_physical(BlockHeader* header, uint8_t* heap_start) {
    uint8_t* curr = reinterpret_cast<uint8_t*>(header);
    if (curr <= heap_start) return nullptr;
    BlockFooter* prev_footer = reinterpret_cast<BlockFooter*>(curr - sizeof(BlockFooter));
    return reinterpret_cast<BlockHeader*>(curr - prev_footer->size);
}

MemoryAllocator::MemoryAllocator(size_t heap_size_bytes)
    : heap_storage_(align_up(heap_size_bytes, ALIGNMENT)),
      heap_start_(nullptr),
      heap_end_(nullptr),
      free_list_head_(nullptr) {
    init_heap();
}

MemoryAllocator::~MemoryAllocator() = default;

void MemoryAllocator::init_heap() {
    heap_start_ = heap_storage_.data();
    heap_end_ = heap_start_ + heap_storage_.size();

    // Initialize one large free block covering the entire heap
    BlockHeader* initial_block = reinterpret_cast<BlockHeader*>(heap_start_);
    initial_block->size = static_cast<uint32_t>(heap_storage_.size());
    initial_block->is_allocated = 0;
    initial_block->next_free = nullptr;
    initial_block->prev_free = nullptr;

    BlockFooter* footer = get_footer(initial_block);
    footer->size = initial_block->size;
    footer->is_allocated = 0;

    free_list_head_ = initial_block;
}

void MemoryAllocator::insert_free_block(BlockHeader* block) {
    block->is_allocated = 0;
    BlockFooter* footer = get_footer(block);
    footer->is_allocated = 0;

    block->next_free = free_list_head_;
    block->prev_free = nullptr;
    if (free_list_head_) {
        free_list_head_->prev_free = block;
    }
    free_list_head_ = block;
}

void MemoryAllocator::remove_free_block(BlockHeader* block) {
    if (block->prev_free) {
        block->prev_free->next_free = block->next_free;
    } else {
        free_list_head_ = block->next_free;
    }
    if (block->next_free) {
        block->next_free->prev_free = block->prev_free;
    }
    block->next_free = nullptr;
    block->prev_free = nullptr;
}

BlockHeader* MemoryAllocator::find_fit(size_t total_size, AllocationPolicy policy) {
    BlockHeader* best = nullptr;
    BlockHeader* curr = free_list_head_;

    while (curr) {
        if (curr->size >= total_size) {
            if (policy == AllocationPolicy::FIRST_FIT) {
                return curr;
            }
            if (!best || curr->size < best->size) {
                best = curr;
            }
        }
        curr = curr->next_free;
    }
    return best;
}

void MemoryAllocator::split_block(BlockHeader* block, size_t needed_size) {
    size_t remaining_size = block->size - needed_size;

    if (remaining_size >= MIN_BLOCK_SIZE) {
        // Split block into needed_size and remaining_size
        block->size = static_cast<uint32_t>(needed_size);
        BlockFooter* block_footer = get_footer(block);
        block_footer->size = block->size;
        block_footer->is_allocated = 1;

        // Create remaining free block
        uint8_t* rem_ptr = reinterpret_cast<uint8_t*>(block) + needed_size;
        BlockHeader* rem_block = reinterpret_cast<BlockHeader*>(rem_ptr);
        rem_block->size = static_cast<uint32_t>(remaining_size);
        rem_block->is_allocated = 0;
        rem_block->next_free = nullptr;
        rem_block->prev_free = nullptr;

        BlockFooter* rem_footer = get_footer(rem_block);
        rem_footer->size = rem_block->size;
        rem_footer->is_allocated = 0;

        insert_free_block(rem_block);
    } else {
        // Remaining space is too small to form a block; allocate whole chunk
        BlockFooter* footer = get_footer(block);
        footer->is_allocated = 1;
    }
    block->is_allocated = 1;
}

void* MemoryAllocator::allocate(size_t size, AllocationPolicy policy) {
    if (size == 0) return nullptr;

    // Total required: header + aligned payload + footer
    size_t total_needed = align_up(sizeof(BlockHeader) + size + sizeof(BlockFooter), ALIGNMENT);
    if (total_needed < MIN_BLOCK_SIZE) total_needed = MIN_BLOCK_SIZE;

    BlockHeader* block = find_fit(total_needed, policy);
    if (!block) {
        std::cerr << "[Allocator] Out of Memory! Cannot satisfy request for " << size << " bytes.\n";
        return nullptr;
    }

    remove_free_block(block);
    split_block(block, total_needed);

    // Payload address is immediately following BlockHeader
    return reinterpret_cast<uint8_t*>(block) + sizeof(BlockHeader);
}

BlockHeader* MemoryAllocator::coalesce(BlockHeader* block) {
    BlockHeader* prev = get_prev_physical(block, heap_start_);
    BlockHeader* next = get_next_physical(block, heap_end_);

    bool prev_free = (prev && prev->is_allocated == 0);
    bool next_free = (next && next->is_allocated == 0);

    if (prev_free && next_free) {
        // Case 4: Merge both sides
        remove_free_block(prev);
        remove_free_block(next);

        prev->size += block->size + next->size;
        BlockFooter* footer = get_footer(prev);
        footer->size = prev->size;
        insert_free_block(prev);
        return prev;
    } else if (prev_free) {
        // Case 3: Merge left
        remove_free_block(prev);
        prev->size += block->size;
        BlockFooter* footer = get_footer(prev);
        footer->size = prev->size;
        insert_free_block(prev);
        return prev;
    } else if (next_free) {
        // Case 2: Merge right
        remove_free_block(next);
        block->size += next->size;
        BlockFooter* footer = get_footer(block);
        footer->size = block->size;
        insert_free_block(block);
        return block;
    } else {
        // Case 1: Neither neighbor free
        insert_free_block(block);
        return block;
    }
}

void MemoryAllocator::deallocate(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = reinterpret_cast<BlockHeader*>(
        reinterpret_cast<uint8_t*>(ptr) - sizeof(BlockHeader));

    if (block->is_allocated == 0) {
        std::cerr << "[Allocator] Error: Double free detected!\n";
        return;
    }

    block->is_allocated = 0;
    BlockFooter* footer = get_footer(block);
    footer->is_allocated = 0;

    coalesce(block);
}

size_t MemoryAllocator::get_total_free_bytes() const {
    size_t total = 0;
    BlockHeader* curr = free_list_head_;
    while (curr) {
        total += curr->size;
        curr = curr->next_free;
    }
    return total;
}

size_t MemoryAllocator::get_largest_free_block() const {
    size_t largest = 0;
    BlockHeader* curr = free_list_head_;
    while (curr) {
        if (curr->size > largest) largest = curr->size;
        curr = curr->next_free;
    }
    return largest;
}

void MemoryAllocator::print_heap_state() const {
    std::cout << "Heap State (" << heap_storage_.size() << " bytes total):\n";
    std::cout << "  Free blocks count: ";
    size_t count = 0;
    BlockHeader* curr = free_list_head_;
    while (curr) {
        count++;
        curr = curr->next_free;
    }
    std::cout << count << ", Total Free: " << get_total_free_bytes()
              << " bytes, Largest Free Block: " << get_largest_free_block() << " bytes\n";
}
