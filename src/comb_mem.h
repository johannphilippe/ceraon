#ifndef COMB_MEM_H
#define COMB_MEM_H


#include<iostream>
#include<vector>
#include<list>


struct vec
{
    vec(size_t size)
        : _capacity(size)
        , used(0)
    {
        _data = new std::byte[size];
    }

    std::byte* data() {return _data;}
    size_t capacity() {return _capacity;}

    size_t _capacity, used;
    std::byte *_data;
};

struct mem_zone : public vec //public std::vector<std::byte>
{
    mem_zone(size_t nbytes)
        : vec(nbytes)
    {}

    std::byte *reserve_mem(size_t nbytes, bool *result)
    {
        std::cout << "asking nbytes : " << nbytes 
            << " while capacity is : " << (this->capacity() - used) << std::endl;
        std::byte *ptr = nullptr;
        if( (this->capacity() - this->used) >= nbytes)
        {
            std::cout << "capacity is ok " << std::endl;
            ptr = this->data() + this->used; 
            std::cout << "ptr : " << ptr << std::endl;
            this->used += nbytes;
            *result = true;;
            return ptr;
        }
        *result = false;
        return ptr;
    }

};

struct mem_alloc
{
    mem_alloc(size_t initial = 16384)
        : mem(1, initial)
    {
    }

    std::byte *mem_reserve(size_t nbytes)
    {
        std::byte *ptr;
        bool res = false;

        for(auto & it : mem)
        {
            ptr = it.reserve_mem(nbytes, &res);
            if(res)
                return ptr;
        }
        size_t new_capacity = mem.back().capacity() * 2;
        while(new_capacity < nbytes)
            new_capacity *= 2;
        std::cout << "new capacity = " << new_capacity << " & " << nbytes << std::endl;
        mem.push_back(mem_zone(new_capacity));
        ptr = mem.back().reserve_mem(nbytes, &res);
        if(res)
            return ptr;
        else 
            throw std::runtime_error("Could not allocate");
        return nullptr;
    }

    template<typename Flt = double>
    void alloc_channels(size_t bloc_size, size_t nchannels, Flt **dptr)
    {

        std::cout << "alloc channels " << nchannels << " & " << bloc_size << std::endl;
        size_t nbytes = (bloc_size * nchannels) * sizeof(Flt);
        std::byte *ptr = this->mem_reserve(nbytes);
        std::cout << ptr << std::endl;
        if(ptr == nullptr)
            throw std::runtime_error("Allocation error");

        std::cout << "allocated " << std::endl;
        for(size_t i = 0; i < nchannels; ++i)
        {
            size_t index_in_zone = i * (bloc_size);
            std::cout << "index in zone :" << index_in_zone << std::endl;
            dptr[i] = (Flt *) ptr + index_in_zone;
            std::cout << dptr[i] << std::endl;
        }
        std::cout << " pushed pointer to location" << std::endl;
    }

    size_t total_used()
    {
        size_t cnt = 0;
        for(auto & it : mem)
            cnt += it.used;
        return cnt;
    }

    std::list<mem_zone> mem;
};

extern mem_alloc *main_mem;

#endif