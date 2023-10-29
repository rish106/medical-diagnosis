#include <string>
#include <vector>

using namespace std;

class graph_node {

private:
    string name;
    vector<int> children;
    vector<string> parents;
    vector<float> cpt, original_cpt;

public:
    graph_node(string name) {
        this->name = name;
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
    vector<float> get_original_cpt() {
        return original_cpt;
    }
    void set_cpt(vector<float> new_cpt) {
        cpt.clear();
        cpt = new_cpt;
    }
    void set_original_cpt(vector<float> new_cpt) {
        original_cpt.clear();
        cpt.clear();
        original_cpt = new_cpt;
        cpt = new_cpt;
    }
    void set_parents(vector<string> parent_nodes) {
        parents.clear();
        parents = parent_nodes;
    }
    bool add_child(int new_child_index) {
        for (int i = 0; i < children.size(); i++) {
            if (children[i] == new_child_index) {
                return false;
            }
        }
        children.push_back(new_child_index);
        return true;
    }
};
