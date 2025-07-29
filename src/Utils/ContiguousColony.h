#pragma once

#include <vector>
#include <unordered_map>

template <class DataType, class IdType>
struct ContiguousColony
{

    void reserve(std::size_t new_size)
    {
        data.reserve(new_size);
        data_ind2id.reserve(new_size);
    }

    void insert(IdType id, DataType datum)
    {
        data.push_back(datum);
        data_ind2id.push_back(id);

        assert(id2data_ind.count(id) == 0);
        id2data_ind[id] = data.size() - 1;
    }

    DataType& get(IdType id)
    {
        return data.at(id2data_ind.at(id));
    }

    void erase(IdType id)
    {
        assert(id2data_ind.count(id) != 0);
        std::size_t data_ind = id2data_ind.at(id);

        IdType swapped_id = data_ind2id.at(data.size() - 1);
        id2data_ind.at(swapped_id) = data_ind; //! swapped points to erased

        data.at(data_ind) = data.back();               //! swap
        data.pop_back();                               //! and pop
        data_ind2id.at(data_ind) = data_ind2id.back(); //! swap
        data_ind2id.pop_back();                        //! and pop

        id2data_ind.erase(id);
    }

    bool isEmpty() const
    {
        return data.empty();
    }

    bool contains(IdType id)const
    {
        return id2data_ind.contains(id);
    }

public:
    std::vector<DataType> data;
    std::vector<IdType> data_ind2id;

private:
    std::unordered_map<IdType, std::size_t> id2data_ind;
};
