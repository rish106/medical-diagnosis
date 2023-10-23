#include <iostream>

#include "network.hpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Bayesnet file and data file names required as arguments\n";
        exit(1);
    }
    string bayes_net_filename = argv[1];
    string data_filename = argv[2];
    string output_filename = "solved_alarm.bif";
    network alarm;
    alarm.read_network(bayes_net_filename);
    alarm.read_data(data_filename);
    alarm.randomise_missing_data();
    alarm.update_cpt();
    for (int i = 0; i < 1000; i++) {
        alarm.generate_missing_data();
        alarm.update_cpt();
    }
    alarm.print_cpt();
    alarm.write_to_file(output_filename);
    cout << "Perfect! Hurrah!\n";
}
