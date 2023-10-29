#include <iomanip>
#include <array>
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
const float LIDSTONE_CONSTANT = 0.1;
const float MINIMUM_PROBABILITY = 0.0001;
const float CHANGE_THRESHOLD = 0.0001;

class network {

private:

    vector<unordered_map<string, int>> possible_values;
    vector<array<int, 3>> missing_data_lines;
    vector<vector<int>> current_data;
    unordered_map<string, int> variable_name_index;

public:

    vector<graph_node*> pres_graph;

    int add_node(graph_node* node) {
        pres_graph.push_back(node);
        return 0;
    }

    int net_size() {
        return pres_graph.size();
    }

    int get_index(string val_name) {
        return variable_name_index[val_name];
    }

    graph_node* get_nth_node(int n) {
        return pres_graph[n];
    }

    graph_node* search_node(string val_name) {
        return pres_graph[variable_name_index[val_name]];
    }

    void read_network(string bayes_net_filename, string output_filename) {
        string line;
        ifstream fin(bayes_net_filename);
        ofstream fout(output_filename);
        string temp;
        string name;
        vector<string> values;
        bool reached_probability = false;

        while (fin.is_open() && !fin.eof()) {
            stringstream ss;
            getline(fin, line);
            ss.str(line);
            ss >> temp;
            if (temp.compare("variable") == 0) {
                fout << line << '\n';
                ss >> name;
                unordered_map<string, int> node_values;
                getline(fin, line);
                fout << line << '\n';
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
                graph_node* new_node = new graph_node(name);
                variable_name_index[name] = net_size();
                int pos = add_node(new_node);
            } else if (temp.compare("probability") == 0) {
                ss >> temp;
                ss >> temp;
                reached_probability = true;
                graph_node* current_node = search_node(temp);
                graph_node* parent_node;
                int index = get_index(temp);
                ss >> temp;
                values.clear();
                while (temp.compare(")") != 0) {
                    parent_node = search_node(temp);
                    parent_node->add_child(index);
                    values.push_back(temp);
                    ss >> temp;
                }
                current_node->set_parents(values);
                getline(fin, line);
                stringstream ss2;
                ss2.str(line);
                ss2 >> temp;
                ss2 >> temp;
                vector<float> curr_cpt;
                while (temp.compare(";") != 0) {
                    curr_cpt.push_back(atof(temp.c_str()));
                    ss2 >> temp;
                }
                current_node->set_original_cpt(curr_cpt);
            } else if (!reached_probability) {
                fout << line << '\n';
            }
        }
        fin.close();
        fout.close();
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
            int missing_data_index = -1;
            for (int i = 0; ss >> val && i < net_size(); i++) {
                if (val == MISSING_DATA) {
                    missing_data_index = i;
                    enumerated_values.push_back(-1);
                } else {
                    enumerated_values.push_back(possible_values[i][val]);
                }
            }
            if (missing_data_index != -1) {
                vector<string> parent_names = pres_graph[missing_data_index]->get_parents();
                int evidence_index = 0;
                for (string &parent_name : parent_names) {
                    int parent_index = variable_name_index[parent_name];
                    evidence_index = (evidence_index * possible_values[parent_index].size()) + enumerated_values[parent_index];
                }
                missing_data_lines.push_back({line_number, missing_data_index, evidence_index});
            }
            line_number++;
            current_data.push_back(enumerated_values);
        }
        fin.close();
    }

    bool learn_and_update_cpt() {
        bool has_not_converged = false;
        for (int node_index = 0; node_index < pres_graph.size(); node_index++) {
            vector<float> original_cpt = pres_graph[node_index]->get_original_cpt();
            bool has_unknown_probability = false;
            for (int x : original_cpt) {
                if (x == -1) {
                    has_unknown_probability = true;
                    break;
                }
            }
            if (!has_unknown_probability) {
                continue;
            }
            vector<int> parent_indices;
            int max_size = possible_values[node_index].size();
            vector<string> parent_names = pres_graph[node_index]->get_parents();
            for (string parent : parent_names) {
                int parent_index = get_index(parent);
                parent_indices.push_back(parent_index);
                max_size *= possible_values[parent_index].size();
            }
            vector<int> cpt_occurences(max_size, 0), evidence_occurences(max_size / possible_values[node_index].size(), 0);
            for (vector<int> &data_sample : current_data) {
                int cpt_index = data_sample[node_index];
                for (int parent_index : parent_indices) {
                    cpt_index = (cpt_index * possible_values[parent_index].size()) + data_sample[parent_index];
                }
                cpt_occurences[cpt_index]++;
                evidence_occurences[cpt_index % evidence_occurences.size()]++;
            }
            vector<float> new_cpt(max_size);
            vector<float> old_cpt = pres_graph[node_index]->get_cpt();
            float numerator, denominator;
            for (int i = 0; i < new_cpt.size(); i++) {
                if (original_cpt[i] != -1) {
                    new_cpt[i] = original_cpt[i];
                    continue;
                }
                numerator = (float)cpt_occurences[i] + LIDSTONE_CONSTANT;
                denominator = (float)evidence_occurences[i % evidence_occurences.size()] + LIDSTONE_CONSTANT * (float)possible_values[node_index].size();
                new_cpt[i] = max(numerator / denominator, MINIMUM_PROBABILITY);
                has_not_converged = has_not_converged || (abs(new_cpt[i] - old_cpt[i]) >= CHANGE_THRESHOLD);
            }
            pres_graph[node_index]->set_cpt(new_cpt);
        }
        return !has_not_converged;
    }

    void randomise_missing_data() {
        srand(time(0));
        for (auto [line_number, var_index, evidence_index] : missing_data_lines) {
            int upper_cap_value = possible_values[var_index].size();
            current_data[line_number][var_index] = rand() % upper_cap_value;
        }
    }

    void generate_missing_data() {
        srand(time(0));
        for (auto [line_number, var_index, evidence_index] : missing_data_lines) {
            vector<float> cpt = pres_graph[var_index]->get_cpt();
            int evidence_size = cpt.size() / possible_values[var_index].size();
            vector<float> query_probabilites;
            float total_probability = 0.0;
            for (int i = evidence_index; i < cpt.size(); i += evidence_size) {
                query_probabilites.push_back(cpt[i]);
                total_probability += cpt[i];
            }
            float max_probability = 0;
            int random_item = 0;
            for (int i = 0; i < query_probabilites.size(); i++) {
                if (query_probabilites[i] > max_probability) {
                    max_probability = query_probabilites[i];
                    random_item = i;
                }
            }
            current_data[line_number][var_index] = random_item;
        }
    }

    void write_cpt_to_file(string output_filename) {
        ofstream fout(output_filename, ios::app);
        for (int node_index = 0; node_index < pres_graph.size(); node_index++) {
            vector<string> parent_names = pres_graph[node_index]->get_parents();
            vector<float> cpt = pres_graph[node_index]->get_cpt();
            fout << "probability (  " << pres_graph[node_index]->get_name();
            for (string &parent_name : parent_names) {
                fout << "  " << parent_name;
            }
            fout << " ) { //" << parent_names.size() + 1 << " variable(s) and ";
            fout << cpt.size() << " values\n" << "\ttable " << fixed << setprecision(4);
            for (int i = 0; i < cpt.size(); i++) {
                fout << cpt[i] << ' ';
            }
            fout << ";\n}\n";
        }
        fout.close();
    }

    void print_cpt_list() {
        for (int i = 0; i < pres_graph.size(); i++) {
            vector<float> cpt = pres_graph[i]->get_cpt();
            cout << fixed << setprecision(4);
            for (int i = 0; i < cpt.size(); i++) {
                cout << cpt[i] << " \n"[i == cpt.size() - 1];
            }
        }
    }

    ~network() {
        for (int i = pres_graph.size(); i >= 0; i--) {
            delete pres_graph[i];
        }
    }

};
