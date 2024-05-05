#include "core.h"

#include "Utils/GayVector.h"

struct Selection{

    GayVectorI<N_MAX_NAVIGABLE_BOIDS> selected_agents;


    const std::vector<int>& getSelectedInds() const{
        return selected_agents.data;
    }

    bool empty()const{
        return selected_agents.data.empty();
    }

    void clear(){
        selected_agents.clear();
    }

};
