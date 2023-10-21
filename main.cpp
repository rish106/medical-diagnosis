#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>


using namespace std;

class graph_node {

private:
    string name;
    vector<int> children;
    vector<string> parents;
    int nvalues;
    vector<string> values;
    vector<float> cpt;

public:
    graph_node(string name, int n, vector<string> vals) {
        this->name = name;
        nvalues = n;
        values = vals;
    }
    string get_name() {
        return name;
    }
    vector<int> get_children() {
        return children;
    }
    vector<string> get_parents() {
        return parents;
    }
    vector<float> get_cpt() {
        return cpt;
    }
    int get_nvalues() {
        return nvalues;
    }
    vector<string> get_values() {
        return values;
    }
    void set_cpt(vector<float> new_cpt) {
        cpt.clear();
        cpt = new_cpt;
    }
    void set_parents(vector<string> parent_nodes) {
        parents.clear();
        parents = parent_nodes;
    }
    bool add_child(int new_child_index ) {
        for (int i = 0; i < children.size(); i++) {
            if (children[i] == new_child_index) {
                return false;
            }
        }
        children.push_back(new_child_index);
        return true;
    }
};


class network {

    list <graph_node> pres_graph;

public:

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
            if (count == n)
                return list_it;
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
        cout << "node not found\n";
        return list_it;
    }


};

network read_network() {
    network alarm;
    string line;
    ifstream myfile("alarm.bif");
    string temp;
    string name;
    vector<string> values;

    while (myfile.is_open() && !myfile.eof()) {
        stringstream ss;
        getline(myfile, line);
        ss.str(line);
        ss >> temp;
        if (temp.compare("variable") == 0) {
            ss >> name;
            getline(myfile, line);
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
            graph_node new_node(name, values.size(), values);
            int pos = alarm.add_node(new_node);
        }
        else if (temp.compare("probability") == 0) {
            ss >> temp;
            ss >> temp;
            list<graph_node>::iterator list_it;
            list<graph_node>::iterator list_it1;
            list_it = alarm.search_node(temp);
            int index = alarm.get_index(temp);
            ss >> temp;
            values.clear();
            while (temp.compare(")") != 0) {
                list_it1 = alarm.search_node(temp);
                list_it1->add_child(index);
                values.push_back(temp);
                ss >> temp;
            }
            list_it->set_parents(values);
            getline(myfile, line);
            stringstream ss2;
            ss2.str(line);
            ss2 >> temp;
            ss2 >> temp;
            vector<float> curr_cpt;
            string::size_type sz;
            while (temp.compare(";") != 0) {
                curr_cpt.push_back(atof(temp.c_str()));
                ss2 >> temp;
            }
            list_it->set_cpt(curr_cpt);
        }
    }

    return alarm;
}


int main() {
    network alarm;
    alarm = read_network();
    cout << "Perfect! Hurrah!\n";
}
