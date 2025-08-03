#pragma once

#include <vector>
#include <unordered_map>

template <class DataType, class IdType>
struct ContiguousColony
{

    void clear()
    {
        data.clear();
        data_ind2id.clear();
        id2data_ind.clear();
    }

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

    DataType &get(IdType id)
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

    std::size_t size() const
    {
        return data.size();
    }

    bool contains(IdType id) const
    {
        return id2data_ind.contains(id);
    }

public:
    std::vector<DataType> data;
    std::vector<IdType> data_ind2id;

private:
    std::unordered_map<IdType, std::size_t> id2data_ind;
};

template <typename DataType>
class DynamicObjectPool2
{
public:
    int insert(DataType data)
    {
        int id;
        if (!m_free_list.empty())
        {
            id = m_free_list.back();
            m_free_list.pop_back();
            if (m_free_list.empty())
            {
                m_next_id = m_data.size()+1;
            }
            m_data.insert(id, data);
            return id;
        }
        
        id = m_next_id;
        m_next_id++;
        m_data.insert(id, data);

        return id;
    }

    int reserveIndexForInsertion()
    {
        if (m_free_list.empty())
        {
            //! m_data is full so we add at the end
            int id = m_next_id;
            assert(!m_data.contains(id));
            m_next_id++;
            return id;
        }

        int id = m_free_list.back();
        m_free_list.pop_back();
        if (m_free_list.empty()) //! if we empty the free list then next id points at m_data end
        {
            m_next_id = m_data.size()+1;
        }

        assert(!m_data.contains(id));
        return id;
    }

    void insertAt(int index, DataType datum)
    {
        assert(!m_data.contains(index));
        m_data.insert(index, datum);
    }

    std::vector<DataType> &data()
    {
        return m_data.data;
    }
    std::vector<int> &getIds()
    {
        return m_data.data_ind2id;
    }

    DataType &at(int index)
    {
        return m_data.get(index);
    }

    void remove(int id)
    {
        m_data.erase(id);
        m_free_list.push_back(id);
    }

private:
    int m_next_id = 0;
    ContiguousColony<DataType, int> m_data;
    std::vector<int> m_free_list;
};
