
#pragma once 

#include "../core.h"
#include <array>
#include <vector>
#include <unordered_set>
#include <set>


template <typename DataType, int MAX_OBJECTS = 5000>
struct ObjectPool
{

  std::array<DataType, MAX_OBJECTS> objects;

  std::array<int, MAX_OBJECTS> entity2ind;

  std::vector<int> active_inds;

  std::set<int> free_inds;

  ObjectPool()
  {
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
      free_inds.insert(i);
    }
    entity2ind.fill(-1);
  }

  int addObject(DataType& obj)
  {
    // static_assert(std::is_base_of<DataType, Type>::value);
    
    auto new_entity_ind = *free_inds.begin();
    assert(new_entity_ind < MAX_OBJECTS);
    free_inds.erase(free_inds.begin());

    objects.at(new_entity_ind) = obj;

    entity2ind.at(new_entity_ind) = active_inds.size();
    active_inds.push_back(new_entity_ind);
    return new_entity_ind;
  }
    int addObject(DataType&& obj)
  {
    // static_assert(std::is_base_of<DataType, Type>::value);
    
    auto new_entity_ind = *free_inds.begin();
    assert(new_entity_ind < MAX_OBJECTS);
    free_inds.erase(free_inds.begin());

    objects.at(new_entity_ind) = std::move(obj);

    entity2ind.at(new_entity_ind) = active_inds.size();
    active_inds.push_back(new_entity_ind);
    return new_entity_ind;
  }

//   int addObject(DataType &&obj)
//   {
//     auto new_entity_ind = *free_inds.begin();
//     free_inds.erase(free_inds.begin());

//     objects.at(new_entity_ind) = std::move(obj);

//     entity2ind.at(new_entity_ind) = active_inds.size();
//     active_inds.push_back(new_entity_ind);
//     return new_entity_ind;
//   }

  void remove(int entity_ind)
  {
    if(entity2ind.at(entity_ind) == -1){
        return;
    }
    free_inds.insert(entity_ind);
    // auto it = std::find(active_inds.begin(), active_inds.end(), entity2ind.at(entity_ind));
    // assert(it != active_inds.end());
    // active_inds.erase(it);
    auto vec_ind = entity2ind.at(entity_ind);
    active_inds.at(vec_ind) = active_inds.back();
    entity2ind.at(active_inds.back()) = vec_ind;
    active_inds.pop_back();
    entity2ind.at(entity_ind) = -1;
  }

  DataType &at(int entity_ind)
  {
    // assert(entity_ind < MAX_OBJECTS && entity2ind.at(entity_ind) != -1);
    return objects.at(entity_ind);
  }
};



template <typename Type, int MAX_ENTITIES>
struct GayVector2{
    int first_free_ind = 0;
    std::array<int ,MAX_ENTITIES> entity2ind_in_vec;
    std::vector<Type> data;
    std::vector<int> vec2entity_ind;

    public:
        GayVector2(){
            entity2ind_in_vec.fill(-1);
        }

        void removeEnt(int entity_ind);
        void remove(int vec_ind);
        void insert(Type datum);
        size_t size(){
            return data.size();
        }
        void clear(){
            data.clear();
            for(auto entity_ind : vec2entity_ind){
                entity2ind_in_vec.at(entity_ind) = -1;
            }
            vec2entity_ind.clear();
        }

        void findNewFreeInd(){
            while(entity2ind_in_vec.at(first_free_ind) != -1){
                first_free_ind++;
            }
        }
};

template <typename DataType, int MAX_ENTITIES>
void GayVector2<DataType, MAX_ENTITIES>::removeEnt(int entity_ind)
{
    auto vec_ind = entity2ind_in_vec.at(entity_ind);
    auto &last_entity = vec2entity_ind.back();

    data.at(vec_ind) = data.back();
    data.pop_back();

    vec2entity_ind.at(vec_ind) = vec2entity_ind.back();
    vec2entity_ind.pop_back();

    entity2ind_in_vec.at(last_entity) = vec_ind;
    entity2ind_in_vec.at(entity_ind) = -1;

    if(entity_ind < first_free_ind){first_free_ind = entity_ind;}
}

template <typename DataType, int MAX_ENTITIES>
void GayVector2<DataType, MAX_ENTITIES>::insert(DataType datum)
{
    auto entity_ind = first_free_ind;
    assert(entity_ind < N_MAX_NAVIGABLE_BOIDS);
    const auto new_vec_ind = data.size();

    data.push_back(datum);
    vec2entity_ind.push_back(entity_ind);
    entity2ind_in_vec.at(entity_ind) = new_vec_ind;

    findNewFreeInd();
}

template <typename DataType, int MAX_ENTITIES>
void GayVector2<DataType, MAX_ENTITIES>::remove(int vec_ind)
{
    auto entity_ind = vec2entity_ind.at(vec_ind);
    auto &last_entity = vec2entity_ind.back();

    data.at(vec_ind) = data.back();
    data.pop_back();

    vec2entity_ind.at(vec_ind) = vec2entity_ind.back();
    vec2entity_ind.pop_back();

    entity2ind_in_vec.at(last_entity) = vec_ind;
    entity2ind_in_vec.at(entity_ind) = -1;

    if(entity_ind < first_free_ind){first_free_ind = entity_ind;}
}

template <typename Type>
struct VectorMap{
    std::vector<int> entity2data_ind;
    std::vector<int> data2entity_ind;
    std::set<int> free_inds;
    std::vector<Type> data;
    int n_active = 0; 
    int n_max_entities = 0;

    public:

        VectorMap(int n_max_entities) 
        :
         n_max_entities(n_max_entities),
        entity2data_ind(n_max_entities, -1),
        data2entity_ind(n_max_entities, -1),
        data(n_max_entities)
        {
            for(int i = 0; i < n_max_entities; ++i){
                free_inds.insert(i);
            }
        } 

        void removeByDataInd(int data_ind){
            assert(n_active > 0);
            auto entity_ind = data2entity_ind.at(data_ind);
            free_inds.insert(entity_ind);
    
            assert(data_ind != -1);

            entity2data_ind[entity_ind] = -1;
            entity2data_ind.at(data2entity_ind.at(n_active-1)) = data_ind;

            data[data_ind] = data[n_active-1]; 
            data2entity_ind[n_active-1] = -1;

            n_active--;
        }

        void removeByEntityInd(int entity_ind){
            assert(free_inds.count(entity_ind) == 0);
            assert(n_active > 0);

            free_inds.insert(entity_ind);
            
            auto data_ind = entity2data_ind[entity_ind];
            assert(data_ind != -1);

            entity2data_ind[entity_ind] = -1;
            entity2data_ind.at(data2entity_ind.at(n_active-1)) = data_ind;

            data[data_ind] = data[n_active-1]; 
            data2entity_ind[n_active-1] = -1;

            n_active--;
        }
        void insert(Type datum){
            if(free_inds.empty()){
                return;
            }
            int new_ind = *free_inds.begin();
            free_inds.erase(free_inds.begin());
            
            data.at(n_active) = datum;
            data2entity_ind.at(n_active) = new_ind;
            entity2data_ind.at(new_ind) = n_active;
            

            n_active++;

        }
        size_t size(){
            return n_active;
        }
        Type& getEntity(int index){
            assert(index >= 0 && index <= n_max_entities);
            return data.at(entity2data_ind.at(index));
        }

        void clear(){
            for(int i = 0; i < n_max_entities; ++i){
                free_inds.insert(i);
                entity2data_ind[i] = -1;
                data2entity_ind[i] = -1;
            }
            n_active = 0;
        }
};


template <typename Type, int MAX_ENTITIES>
struct GayVector{
    std::array<int ,MAX_ENTITIES> entity2ind_in_vec;
    std::vector<Type> data;
    std::vector<int> vec2entity_ind;

    public:

        void removeEnt(int entity_ind);
        void remove(int vec_ind);
        void insert(Type datum, int entity_ind);
        size_t size(){
            return data.size();
        }
        void clear(){
            data.clear();
            for(auto entity_ind : vec2entity_ind){
                entity2ind_in_vec.at(entity_ind) = -1;
            }
            vec2entity_ind.clear();
        }
};


template <typename DataType, int MAX_ENTITIES>
void GayVector<DataType, MAX_ENTITIES>::removeEnt(int entity_ind)
{
    auto vec_ind = entity2ind_in_vec.at(entity_ind);
    auto &last_entity = vec2entity_ind.back();

    data.at(vec_ind) = data.back();
    data.pop_back();

    vec2entity_ind.at(vec_ind) = vec2entity_ind.back();
    vec2entity_ind.pop_back();

    entity2ind_in_vec.at(last_entity) = vec_ind;
    entity2ind_in_vec.at(entity_ind) = -1;
}

template <typename DataType, int MAX_ENTITIES>
void GayVector<DataType, MAX_ENTITIES>::remove(int vec_ind)
{
    auto entity_ind = vec2entity_ind.at(vec_ind);
    auto &last_entity = vec2entity_ind.back();

    data.at(vec_ind) = data.back();
    data.pop_back();

    vec2entity_ind.at(vec_ind) = vec2entity_ind.back();
    vec2entity_ind.pop_back();

    entity2ind_in_vec.at(last_entity) = vec_ind;
    entity2ind_in_vec.at(entity_ind) = -1;
}

template <typename DataType, int MAX_ENTITIES>
void GayVector<DataType, MAX_ENTITIES>::insert(DataType datum, int entity_ind)
{
    assert(entity_ind < N_MAX_NAVIGABLE_BOIDS);
    const auto new_vec_ind = data.size();

    data.push_back(datum);
    vec2entity_ind.push_back(entity_ind);
    entity2ind_in_vec.at(entity_ind) = new_vec_ind;
}


template <int MAX_ENTITIES>
struct GayVectorI{
    std::array<int ,MAX_ENTITIES> entity2ind_in_vec;
    std::vector<int> data;

    public:
        GayVectorI(){
            entity2ind_in_vec.fill(-1);
        }

        void removeEnt(int entity_ind);
        void remove(int vec_ind);
        void insert(int ind, int entity_ind);
        size_t size(){
            return data.size();
        }
        void clear(){
            for(auto entity_ind : data){
                entity2ind_in_vec.at(entity_ind) = -1;
            }
            data.clear();
        }

        bool containsEnt(int entity_ind){
            return entity2ind_in_vec.at(entity_ind)!=-1;
        }

        bool noDuplicates() const{
            std::unordered_set<int> set;
            for(auto d : data){
                if(set.count(d)==1){
                    return false;
                }
                set.insert(d);
            }
            set.clear();
            for(auto d : entity2ind_in_vec){
                if(d!=-1){
                    if( set.count(d)==1 ){
                        return false;
                    }
                    if(d >= data.size()){
                        return false;
                    }
                    set.insert(d);
                }
            }
            return true;
        }

};



template <int MAX_ENTITIES>
void GayVectorI<MAX_ENTITIES>::removeEnt(int entity_ind)
{
    auto vec_ind = entity2ind_in_vec.at(entity_ind);
    auto &last_entity = data.back();
    if(vec_ind == -1){return;}
    data.at(vec_ind) = data.back();
    data.pop_back();

    entity2ind_in_vec.at(last_entity) = vec_ind;
    entity2ind_in_vec.at(entity_ind) = -1;
}

template <int MAX_ENTITIES>
void GayVectorI<MAX_ENTITIES>::remove(int vec_ind)
{
    auto entity_ind = data.at(vec_ind);
    auto &last_entity = data.back();

    data.at(vec_ind) = data.back();
    data.pop_back();

    entity2ind_in_vec.at(last_entity) = vec_ind;
    entity2ind_in_vec.at(entity_ind) = -1;
}

template <int MAX_ENTITIES>
void GayVectorI<MAX_ENTITIES>::insert(int datum, int entity_ind)
{
    assert(entity_ind < N_MAX_NAVIGABLE_BOIDS);
    const auto new_vec_ind = data.size();

    data.push_back(datum);
    entity2ind_in_vec.at(entity_ind) = new_vec_ind;
}
