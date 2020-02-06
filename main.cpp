#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <unordered_set>
#include <cassert>
#include <cmath>

// params
int gen_max = 100;
int popsize = 100;
int landscape_max = 10; // must always be even!

// random number generator
std::mt19937_64 rng;
std::uniform_int_distribution<int> position_distribution_int(0, landscape_max-1);
std::uniform_real_distribution<double> dispersal_distribution_float(0.0, static_cast<double>(landscape_max/2));

// agent class
class agent
{
public:
    double p_dispersal = dispersal_distribution_float(rng);
    int position = position_distribution_int(rng);
    double intake = 0.0001;

    void do_disperse();
    void do_get_fitness();
};

std::vector<agent> population(popsize);

// gridcell class
class gridcell
{
public:
    int colonisers = 0;
};

std::vector<gridcell> landscape(landscape_max);

void agent::do_disperse(){

    int old_pos = position;

    double temp_pos = static_cast<double>(position) + p_dispersal;

    temp_pos = fmod(temp_pos, static_cast<double>(landscape_max));

    position = static_cast<int>(temp_pos);

    assert(position <= (landscape_max-1) &&
           "do_disperse: agent walked over landscape\n");
}

void count_colonisers(){
    for(int land = 0; land < landscape_max; land++){
        // reset initial value
        landscape[land].colonisers = 0;

        for (int ind3 = 0;ind3 < popsize;ind3++) {
            if(static_cast<int>(population[ind3].position == land)){
                landscape[land].colonisers += 1;
            }
        }

        std::cout << "colonisers = " << landscape[land].colonisers << "\n";
    }
}

void agent::do_get_fitness(){
    int neighbours = landscape[position].colonisers;
    intake += ((neighbours == 0) ? (1.0) : (1.0/(static_cast<double>(neighbours))));
}

void do_reproduce(){
    // make fitness vec
    std::vector<double> fitness_vec;
    for (int a = 0; a < popsize; a++)
    {
//        assert(population[a].intake >= 0.f && "agent energy is 0!");

        fitness_vec.push_back(static_cast<double> (population[a].intake));
    }

    // temp pop vec
    std::vector<agent> pop2(popsize);
    // assign parents
    for (int a = 0; a < popsize; a++){
        std::discrete_distribution<> weighted_lottery(fitness_vec.begin(), fitness_vec.end());

        int parent_id = weighted_lottery(rng);
        // inherit dispersal
        pop2[a].p_dispersal = population[parent_id].p_dispersal;
        // inherit parent position
        pop2[a].position = population[parent_id].position;
        // reset energy
        pop2[a].intake = 0.0001;

        // mutate dispersal parameter
        {
            std::bernoulli_distribution mut_event(0.001);
            if(mut_event(rng)){
                std::cauchy_distribution<double> m_shift(0.0, 0.1);
                pop2[a].p_dispersal += static_cast<int>(m_shift(rng));

                if(pop2[a].p_dispersal > landscape_max/2){
                    pop2[a].p_dispersal = landscape_max/2;
                }
                if(pop2[a].p_dispersal < 0){
                    pop2[a].p_dispersal = 0;
                }
            }
        }
    }

    population = pop2;
}

void write_data(const int& gen_p)
{
    // open or append
    std::ofstream agentofs;
    agentofs.open("aggremove_dataAgents.csv", std::ofstream::out | std::ofstream::app);

    // col header
    if (gen_p == 0) { agentofs << "gen,id,p_disp,pos,intake\n"; }

    // print for each ind
//    if ((gen_p == 0 || gen_p % 20 == 0) && time_p % 5 == 0)
//    {
        for (int ind2 = 0; ind2 < popsize; ind2++)
        {
            agentofs
                << gen_p << ","
                << ind2 << ","
                << population[ind2].p_dispersal << ","
                << population[ind2].position << ","
                << population[ind2].intake << "\n";
        }
//    }
    // close
    agentofs.close();
}

int main()
{
    // run through generations
    for(int gen = 0; gen < gen_max; gen++)
    {
        std::cout << "gen = " << gen << "\n";

        // run through individuals
        // disperse agents
        for(int ind = 0; ind < popsize; ind++)
        {
            population[ind].do_disperse();

        }
        count_colonisers();

        write_data(gen);
        do_reproduce();
    }


    std::cout << "sim run\n";
    return 0;
}
