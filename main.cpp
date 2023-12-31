#include <chrono>
#include <string>
#include <vector>
#include <iostream>

#include "network.hpp"

using namespace std;

const int MAX_EM_ITERATIONS = 3000;
int DURATION_MILLISECONDS = 115 * 1000;

int main(int argc, char** argv) {
    auto start_time = chrono::high_resolution_clock::now();
    if (argc != 3) {
        cerr << "Bayesnet file and data names required as arguments\n";
        exit(1);
    }
    string bayesnet_filename = argv[1];
    string data_filename = argv[2];
    string output_filename = "solved_alarm.bif";
    network* alarm = new network();
    alarm->read_network(bayesnet_filename, output_filename);
    alarm->read_data(data_filename);
    alarm->randomise_missing_data();
    alarm->learn_and_update_cpt();
    auto single_end_time = chrono::high_resolution_clock::now();
    int SINGLE_ITERATION_TIME_MILLSECONDS = chrono::duration_cast<chrono::milliseconds>(single_end_time - start_time).count();
    DURATION_MILLISECONDS -= 2 * SINGLE_ITERATION_TIME_MILLSECONDS;
    int em_iterations = 0;
    auto current_time = chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    bool has_converged = false;
    for (int i = 0; i < MAX_EM_ITERATIONS && !has_converged && (elapsed_milliseconds < DURATION_MILLISECONDS); i++) {
        em_iterations++;
        alarm->generate_missing_data();
        has_converged = alarm->learn_and_update_cpt();
        current_time = chrono::high_resolution_clock::now();
        elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    }
    alarm->write_cpt_to_file(output_filename);
    // alarm->print_cpt_list();
    current_time = chrono::high_resolution_clock::now();
    elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    // cout << "EM Iterations: " << em_iterations << '\n';
    // cout << "Finished in " << elapsed_milliseconds << " milliseconds\n";
}
