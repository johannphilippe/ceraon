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
        _data = new std::byte[size]();
    }

    std::byte* data() {return _data;}
    size_t capacity() {return _capacity;}

    size_t _capacity, used;
    std::byte *_data;
};

struct mem_zone : public vec 
{
    mem_zone(size_t nbytes)
        : vec::vec(nbytes)
    {}

    std::byte *reserve_mem(size_t nbytes, bool *result)
    {
        std::byte *ptr = nullptr;
        if( (this->capacity() - this->used) >= nbytes)
        {
            ptr = this->data() + this->used; 
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
        mem.push_back(mem_zone(new_capacity));
        ptr = mem.back().reserve_mem(nbytes, &res);
        if(res)
            return ptr;
        else 
            throw std::runtime_error("Could not allocate");
        return nullptr;
    }

    template<typename Flt = double> 
    Flt *alloc_buffer(size_t bloc_size) 
    {
        return new Flt[bloc_size]();
        /*
        size_t nbytes = (bloc_size * sizeof(Flt));
        std::byte *ptr = this->mem_reserve(nbytes);
        if(ptr == nullptr)
            throw std::runtime_error("Allocation error");
        return (Flt*)ptr;
        */
    }

    template<typename Flt = double>
    void alloc_channels(size_t bloc_size, size_t nchannels, Flt **dptr)
    {
        for(size_t i = 0; i < nchannels; ++i)
            dptr[i] = new Flt[bloc_size]();
        /*
        size_t nbytes = (bloc_size * nchannels) * sizeof(Flt);
        std::byte *ptr = this->mem_reserve(nbytes);
        if(ptr == nullptr)
            throw std::runtime_error("Allocation error");

        for(size_t i = 0; i < nchannels; ++i)
        {
            size_t index_in_zone = i * (bloc_size);
            dptr[i] = (Flt *) ptr + index_in_zone;
        }*/
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