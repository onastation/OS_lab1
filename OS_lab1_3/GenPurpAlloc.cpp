#include "GenPurpAlloc.h"
#include <cassert>
#include <iostream>

Allocator::Allocator(size_t total_size)
{
    assert(total_size % block_alignment == 0 && "Size must be 4 byte aligned");
    m_totalSize = total_size;
    m_start_ptr = malloc(m_totalSize);
    m_end_ptr = static_cast<void*>(static_cast<char*>(m_start_ptr) + total_size);

    auto* header = (Header*)m_start_ptr;
    header->is_available = true;
    header->size = (total_size - header_size);
    header->prev_size = 0;

    m_used = header_size;
}

void* Allocator::mem_alloc(size_t size)
{
    assert(size > 0 && "Size must be bigger than 0");
    size = align(size);
    if (size > m_totalSize - m_used) return nullptr;

    auto* header = find(size);
    if (header == nullptr) return nullptr;

    split_block(header, size);
    return header + 1;
}

size_t Allocator::align(const size_t size) const {
    size_t align = block_alignment - size % block_alignment;
    align = align == block_alignment ? 0 : align;
    return size + align;
}

Allocator::Header* Allocator::find(size_t size)
{
    auto* header = static_cast<Header*>(m_start_ptr);
    while (!header->is_available || header->size < size)
    {
        header = header->next();
        if (header == m_end_ptr) return nullptr;
    }
    return header;
}

void Allocator::split_block(Allocator::Header* header, size_t chunk)
{
    size_t block_size = header->size;
    header->size = chunk;
    header->is_available = false;
    if (block_size - chunk >= header_size)
    {
        auto* next = header->next();
        next->prev_size = chunk;
        next->size = block_size - chunk - header_size;
        next->is_available = true;
        m_used += chunk + header_size;

        auto* followed = next->next();
        if (followed != m_end_ptr)
        {
            followed->prev_size = next->size;
        }
    }
    else
    {
        header->size = block_size;
        m_used += block_size;
    }
}

void* Allocator::mem_realloc(void* addr, size_t size)
{
    assert(validate_address(addr) && "Error. Invalid reallocation address.");
    if (size <= 0)
    {
        mem_free(addr);
        return nullptr;
    }

    auto* header = static_cast<Header*>(addr) - 1;
    size_t oldSize = header->size;
    size_t prevSize = is_prev_free(header) ? header->prev_size + header_size : 0;
    size_t nextSize = is_next_free(header) ? header->next()->size + header_size : 0;

    if (prevSize + nextSize + header->size >= size)
    {
        if (prevSize != 0)
        {
            header = header->prev();
            header->size += oldSize + header_size;
            memcpy(header + 1, addr, oldSize);
            m_used -= header_size;
        }
        if (nextSize != 0)
        {
            header->size += nextSize;
            m_used -= header_size;
        }
        split_block(header, size);
        return header + 1;
    }

    return move(addr, oldSize, size);
}

void* Allocator::move(void* addr, size_t size, size_t target)
{
    mem_free(addr);
    auto* new_addr = mem_alloc(target);
    if (new_addr != nullptr) memcpy(new_addr, addr, size);
    return new_addr;
}

void Allocator::mem_free(void* addr)
{
    assert(validate_address(addr) && "Error. Invalid freeing address.");
    auto* header = static_cast<Header*>(addr) - 1;
    header->is_available = true;
    m_used -= header->size;
    defragment(header);
}

bool Allocator::validate_address(void* addr)
{
    auto* header = static_cast<Header*>(m_start_ptr);
    while (header != m_end_ptr)
    {
        if (header + 1 == addr) return true;
        header = header->next();
    }
    return false;
}

void Allocator::defragment(Allocator::Header* header)
{
    if (is_prev_free(header))
    {
        auto* prev = header->prev();
        prev->size += header->size + header_size;
        m_used -= header_size;
        header = prev;
    }
    if (is_next_free(header))
    {
        header->size += header_size + header->next()->size;
        m_used -= header_size;
        auto* next = header->next();
        if (next != m_end_ptr) next->prev_size = header->size;
    }
}

void Allocator::mem_free()
{
    auto* header = static_cast<Header*>(m_start_ptr);
    header->is_available = true;
    header->size = (m_totalSize - header_size);
    m_used = header_size;
}

bool Allocator::is_prev_free(Header* header)
{
    auto* prev = header->prev();
    return header != m_start_ptr && prev->is_available;
}

bool Allocator::is_next_free(Header* header)
{
    auto* next = header->next();
    return next != m_end_ptr && next->is_available;
}

Allocator::~Allocator()
{
    free(m_start_ptr);
    m_start_ptr = nullptr;
    m_end_ptr = nullptr;
}

void Allocator::mem_dump()
{
    std::cout << "Total size: " << m_totalSize << std::endl;
    std::cout << "Used: " << m_used << std::endl;
    std::cout << "Header size: " << header_size << std::endl;
    auto* header = static_cast<Header*>(m_start_ptr);
    while (header != m_end_ptr)
    {
        auto isAvailable = header->is_available ? "+" : "-";
        std::cout << isAvailable << " " << header << " " << header->size << std::endl;
        header = header->next();
    }
    std::cout << std::endl;
}