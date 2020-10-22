#ifndef TEST_ALLOCATOR_H
#define TEST_ALLOCATOR_H

class Allocator
{
public:
    explicit Allocator(size_t total_size);
    void* mem_alloc(size_t size);
    void* mem_realloc(void* addr, size_t size);
    void mem_free(void* addr);
    void mem_free();
    void mem_dump();
    ~Allocator();
private:
    struct Header
    {
    public:
        unsigned short size;
        unsigned short prev_size;
        bool is_available;
        inline Header* next()
        {
            return (Header*)((char*)(this + 1) + size);
        }
        inline Header* prev()
        {
            return (Header*)((char*)this - prev_size) - 1;
        }
    private:
        char aligment[3];
    };

    const size_t header_size = sizeof(Header);
    const size_t block_alignment = 4;
    void* m_start_ptr = nullptr;
    void* m_end_ptr = nullptr;
    size_t m_totalSize;
    size_t m_used;
    inline size_t align(size_t size) const;
    Header* find(size_t size);
    void split_block(Header* header, size_t chunk);
    void defragment(Header* header);
    void* move(void* addr, size_t size, size_t target);
    bool validate_address(void* header);
    bool is_prev_free(Header* header);
    bool is_next_free(Header* header);
};

#endif //TEST_ALLOCATOR_H