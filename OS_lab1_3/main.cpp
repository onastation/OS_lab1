#include <iostream>
#include "GenPurpAlloc.h"

void testingAllocation()
{
    Allocator allocator = Allocator(1024);
    allocator.mem_dump();
    allocator.mem_alloc(512);
    allocator.mem_alloc(256);
    allocator.mem_alloc(128);
    allocator.mem_alloc(64);
    allocator.mem_alloc(32);
    allocator.mem_alloc(16);
    allocator.mem_alloc(8);
    allocator.mem_dump();
}

void testingFreeing()
{
    Allocator allocator = Allocator(1024);

    auto* addr1 = allocator.mem_alloc(512);
    auto* addr2 = allocator.mem_alloc(256);
    auto* addr3 = allocator.mem_alloc(128);
    allocator.mem_dump();

    allocator.mem_free(addr1);
    allocator.mem_dump();

    allocator.mem_free(addr2);
    allocator.mem_dump();

    allocator.mem_free(addr3);
    allocator.mem_dump();

    allocator.mem_alloc(500);
    allocator.mem_alloc(500);
    allocator.mem_dump();

    allocator.mem_free();
    allocator.mem_dump();
}

void testingReallocation() {
    Allocator allocator = Allocator(1024);

    auto* loc = allocator.mem_alloc(200);
    allocator.mem_alloc(200);
    allocator.mem_dump();

    allocator.mem_realloc(loc, 20);
    allocator.mem_dump();

    allocator.mem_realloc(loc, 200);
    allocator.mem_dump();

    allocator.mem_realloc(loc, 500);
    allocator.mem_dump();
}

void testingFragmentation()
{
    Allocator allocator = Allocator(1024);
    void* location1 = allocator.mem_alloc(900);
    void* location2 = allocator.mem_alloc(40);
    void* location3 = allocator.mem_alloc(20);
    allocator.mem_dump();
    allocator.mem_free(location3);
    allocator.mem_realloc(location1, 920);
    allocator.mem_dump();
}

int main() {
    try
    {
        //testingAllocation();
        //testingFreeing();
        //testingReallocation();
        //testingFragmentation();
    }
    catch (std::exception ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}