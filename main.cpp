#include <chrono>
#include <string>
#include <vector>
#include <iostream>

#include "network.hpp"

using namespace std;

int SINGLE_ITERATION_TIME_MILLSECONDS;
int DURATION_MILLISECONDS = 10 * 1000;

int main(int argc, char** argv) {
    auto start_time = chrono::high_resolution_clock::now();
    if (argc != 3) {
        cerr << "Bayesnet file and data file names required as arguments\n";
        exit(1);
    }
    string bayes_net_filename = argv[1];
    string data_filename = argv[2];
    string output_filename = "solved_alarm.bif";
    network* alarm = new network();
    alarm->read_network(bayes_net_filename, output_filename);
    alarm->read_data(data_filename);
    auto single_start_time = chrono::high_resolution_clock::now();
    alarm->randomise_missing_data();
    // alarm->generate_missing_data();
    alarm->learn_and_update_cpt();
    auto single_end_time = chrono::high_resolution_clock::now();
    SINGLE_ITERATION_TIME_MILLSECONDS = chrono::duration_cast<chrono::milliseconds>(single_end_time - single_start_time).count();
    DURATION_MILLISECONDS -= 10 * SINGLE_ITERATION_TIME_MILLSECONDS;
    int em_iterations = 0;
    auto current_time = chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    bool has_converged = true;
    for (int i = 0; i < 10 && has_converged; i++) {
        em_iterations++;
        alarm->generate_missing_data();
        has_converged = alarm->learn_and_update_cpt();
        // current_time = chrono::high_resolution_clock::now();
        // elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    }
    alarm->write_cpt_to_file(output_filename);
    // alarm->print_cpt_list();
    delete alarm;
    current_time = chrono::high_resolution_clock::now();
    elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    cout << "EM Iterations : " << em_iterations << '\n';
    cout << "Finished in " << elapsed_milliseconds << " milliseconds\n";
}
