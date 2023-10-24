#include <chrono>
#include <iostream>

#include "network.hpp"

using namespace std;

int BUFFER_MILLISECONDS = 0;
int DURATION_MILLISECONDS = 30 * 1000;

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Bayesnet file and data file names required as arguments\n";
        exit(1);
    }
    string bayes_net_filename = argv[1];
    string data_filename = argv[2];
    string output_filename = "solved_alarm.bif";
    network alarm;
    auto start_time = chrono::high_resolution_clock::now();
    alarm.read_network(bayes_net_filename);
    alarm.read_data(data_filename);
    auto buffer_start_time = chrono::high_resolution_clock::now();
    alarm.randomise_missing_data();
    alarm.learn_and_update_cpt();
    auto buffer_end_time = chrono::high_resolution_clock::now();
    BUFFER_MILLISECONDS = chrono::duration_cast<chrono::milliseconds>(buffer_end_time - buffer_start_time).count();
    DURATION_MILLISECONDS -= BUFFER_MILLISECONDS;
    auto current_time = chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    while (elapsed_milliseconds < DURATION_MILLISECONDS) {
        alarm.generate_missing_data();
        alarm.learn_and_update_cpt();
        current_time = chrono::high_resolution_clock::now();
        elapsed_milliseconds = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    }
    // alarm.print_cpt();
    alarm.write_to_file(output_filename);
    network gold_alarm;
    gold_alarm.read_network("gold_alarm.bif");
    list<graph_node>::iterator alarm_iter, gold_iter;
    alarm_iter = alarm.pres_graph.begin();
    gold_iter = gold_alarm.pres_graph.begin();
    int big_difference = 0, total_cpt_values = 0;
    while (alarm_iter != alarm.pres_graph.end() && gold_iter != gold_alarm.pres_graph.end()) {
        vector<double> alarm_cpt, gold_cpt;
        alarm_cpt = alarm_iter->get_cpt();
        gold_cpt = gold_iter->get_cpt();
        total_cpt_values += alarm_cpt.size();
        for (int i = 0; i < alarm_cpt.size(); i++) {
            if (abs(alarm_cpt[i] - gold_cpt[i]) / gold_cpt[i] > 0.5) {
                big_difference++;
            }
            // cout << abs(alarm_cpt[i] - gold_cpt[i]) << " \n"[i == alarm_cpt.size() - 1];
        }
        alarm_iter++;
        gold_iter++;
    }
    cout << "CPT Values with difference greater than 50% : " << big_difference << '\n';
    cout << "Total CPT values : " << total_cpt_values << '\n';
}
