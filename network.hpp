#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

#include "node.hpp"

using namespace std;

const string MISSING_DATA = "\"?\"";

class network {

    list<graph_node> pres_graph;

public:

    vector<unordered_map<string, int>> possible_values;
    vector<pair<int, int>> missing_data_lines;
    vector<vector<int>> raw_data, curr_data;

    int add_node(graph_node node) {
        pres_graph.push_back(node);
        return 0;
    }

    int net_size() {
        return pres_graph.size();
    }

    int get_index(string val_name) {
        list<graph_node>::iterator list_it;
        int count = 0;
        for (list_it = pres_graph.begin(); list_it != pres_graph.end(); list_it++) {
            if (list_it->get_name().compare(val_name) == 0) {
                return count;
            }
            count++;
        }
        return -1;
    }

    list<graph_node>::iterator get_nth_node(int n) {
        list<graph_node>::iterator list_it;
        int count = 0;
        for (list_it = pres_graph.begin(); list_it != pres_graph.end(); list_it++) {
            if (count == n) {
                return list_it;
            }
            count++;
        }
        return list_it;
    }

    list<graph_node>::iterator search_node(string val_name) {
        list<graph_node>::iterator list_it;
        for (list_it = pres_graph.begin(); list_it != pres_graph.end(); list_it++) {
            if (list_it->get_name().compare(val_name) == 0) {
                return list_it;
            }
        }
        cerr << "node not found\n";
        return list_it;
    }

    void read_network(string bayes_net_filename) {
        string line;
        ifstream fin(bayes_net_filename);
        string temp;
        string name;
        vector<string> values;

        while (fin.is_open() && !fin.eof()) {
            stringstream ss;
            getline(fin, line);
            ss.str(line);
            ss >> temp;
            if (temp.compare("variable") == 0) {
                ss >> name;
                unordered_map<string, int> node_values;
                getline(fin, line);
                stringstream ss2;
                ss2.str(line);
                for (int i = 0; i < 4; i++) {
                    ss2 >> temp;
                }
                values.clear();
                while (temp.compare("};") != 0) {
                    values.push_back(temp);
                    ss2 >> temp;
                }
                for (int i = 0; i < values.size(); i++) {
                    node_values[values[i]] = i;
                }
                possible_values.push_back(node_values);
                graph_node new_node(name, values.size(), values);
                int pos = add_node(new_node);
            }
            else if (temp.compare("probability") == 0) {
                ss >> temp;
                ss >> temp;
                list<graph_node>::iterator list_it, list_it_parent;
                list_it = search_node(temp);
                int index = get_index(temp);
                ss >> temp;
                values.clear();
                while (temp.compare(")") != 0) {
                    list_it_parent = search_node(temp);
                    list_it_parent->add_child(index);
                    values.push_back(temp);
                    ss >> temp;
                }
                list_it->set_parents(values);
                getline(fin, line);
                stringstream ss2;
                ss2.str(line);
                ss2 >> temp;
                ss2 >> temp;
                vector<double> curr_cpt;
                while (temp.compare(";") != 0) {
                    curr_cpt.push_back(atof(temp.c_str()));
                    ss2 >> temp;
                }
                list_it->set_original_cpt(curr_cpt);
            }
        }
    }

    void read_data(string data_filename) {
        ifstream fin(data_filename);
        if (!fin.is_open()) {
            cerr << "Data file could not be opened\n";
            exit(1);
        }
        string line;
        int line_number = 0;
        while (getline(fin, line)) {
            vector<int> enumerated_values;
            stringstream ss;
            ss.str(line);
            string val;
            for (int i = 0; ss >> val && i < net_size(); i++) {
                if (val == MISSING_DATA) {
                    missing_data_lines.push_back({line_number, i});
                    enumerated_values.push_back(-1);
                } else {
                    enumerated_values.push_back(possible_values[i][val]);
                }
            }
            line_number++;
            raw_data.push_back(enumerated_values);
        }
        curr_data = raw_data;
        fin.close();
    }

    void update_cpt() {
        int i = 0;
        for (list<graph_node>::iterator list_it = pres_graph.begin(); list_it != pres_graph.end(); list_it++, i++) {
            vector<double> original_cpt = list_it->get_original_cpt();
            bool found_minus_one = false;
            for (int x : original_cpt) {
                if (x == -1) {
                    found_minus_one = true;
                    break;
                }
            }
            if (!found_minus_one) {
                continue;
            }
            vector<int> cpt_indices;
            int max_size = possible_values[i].size();
            for (string parent : list_it->get_parents()) {
                int parent_index = get_index(parent);
                cpt_indices.push_back(parent_index);
                max_size *= possible_values[parent_index].size();
            }
            vector<int> cpt_occurences(max_size, 0), evidence_occurences(max_size / possible_values[i].size(), 0);
            for (vector<int> &data_sample : curr_data) {
                int cpt_index = data_sample[i];
                for (int var_index : cpt_indices) {
                    cpt_index = (cpt_index * possible_values[var_index].size()) + data_sample[var_index];
                }
                cpt_occurences[cpt_index]++;
                evidence_occurences[cpt_index % evidence_occurences.size()]++;
            }
            bool has_zero_occurence = false;
            for (int i = 0; i < evidence_occurences.size(); i++) {
                if (evidence_occurences[i] == 0) {
                    has_zero_occurence = true;
                    break;
                }
            }
            // update cpt
            vector<double> probabilities(max_size);
            double numerator, denominator;
            for (int j = 0; j < probabilities.size(); j++) {
                if (original_cpt[j] != -1) {
                    probabilities[j] = original_cpt[j];
                    continue;
                }
                numerator = cpt_occurences[j] + has_zero_occurence;
                denominator = evidence_occurences[j % evidence_occurences.size()] + (possible_values[i].size() * has_zero_occurence);
                probabilities[j] = numerator / denominator;
            }
            list_it->set_cpt(probabilities);
        }
    }

    void randomise_missing_data() {
        for (auto [line_number, var_index] : missing_data_lines) {
            srand(time(0));
            int upper_cap_value = possible_values[var_index].size();
            curr_data[line_number][var_index] = rand() % upper_cap_value;
        }
    }

    void generate_missing_data() {
        for (auto [line_number, var_index] : missing_data_lines) {
            list<graph_node>::iterator query_node = get_nth_node(var_index);
            vector<double> cpt = query_node->get_cpt();
            vector<int> cpt_indices;
            int evidence_size = cpt.size() / possible_values[var_index].size();
            for (string parent : query_node->get_parents()) {
                int parent_index = get_index(parent);
                cpt_indices.push_back(parent_index);
            }
            int cpt_index = 0;
            for (int parent_index : cpt_indices) {
                cpt_index = (cpt_index * possible_values[parent_index].size()) + curr_data[line_number][parent_index];
            }
            double max_probability = 0;
            int max_item = 0;
            for (int i = cpt_index; i < cpt.size(); i += evidence_size) {
                if (cpt[i] > max_probability) {
                    max_probability = cpt[i];
                    max_item = i / evidence_size;
                }
            }
            curr_data[line_number][var_index] = max_item;
        }
    }

    void write_to_file(string output_filename) {
        ofstream fout("solved_alarm.bif");
        fout.close();
    }

    void print_cpt() {
        for (list<graph_node>::iterator list_it = pres_graph.begin(); list_it != pres_graph.end(); list_it++) {
            vector<double> cpt = list_it->get_cpt();
            for (int i = 0; i < cpt.size(); i++) {
                cout << cpt[i] << " \n"[i == cpt.size() - 1];
            }
        }
    }

};
