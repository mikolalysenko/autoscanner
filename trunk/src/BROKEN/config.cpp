//TODO: Document this

#include "config.h"

#define WRITE for (int i__0 = 0; i__0 < level; i__0++) fout << "    "; fout

#include <fstream>
#include <iostream>
using namespace std;

namespace cfg {

config config::global;

void config::save(const string& fname) {
    ofstream fout(fname.c_str());
    save(fout);
}

void config::save(ofstream& fout, int level) {
    WRITE << "[" << name << "]" << endl;
    level++;
    for (data_map::iterator iter = data.begin(); iter != data.end(); iter++) {
        stringstream out_buf(iter->second);
        string dat;
        stringstream out;
        int i = 0;
        while (getline(out_buf, dat)) {
            if (i++ != 0) out << " ";
            out << dat;
        }
        WRITE << iter->first << " = " << out.rdbuf() << endl;
    }

    WRITE << endl;
    
    for (child_map::iterator iter = children.begin(); iter != children.end(); iter++)
        iter->second.save(fout, level);

    level--;
    WRITE << "[/" << name << "]" << endl;    
}

void config::load(const std::string& fname) {
    ifstream fin(fname.substr(0, fname.find_last_not_of(' ') + 1).c_str());
    load(fin);
}

void config::load(ifstream& fin) {
    assert(fin.good());
    fin >> name;
    assert(name[0] == '[' && name[name.size() - 1] == ']');
    name = name.substr(1, name.size() - 2);

    while (true) {
        assert(fin.good());
        string buf;
        getline(fin, buf);
        stringstream line(buf);
        
        string word; line >> word;
        if ( word.size() == 0) continue;
        
        if (word[0] == '[' && word[1] == '/') break;
        
        if (word[0] == '[') {
            string child_name = word.substr(1, word.size() - 2);
            config& child = children[child_name];
            while(fin.peek() != '[') fin.unget();
            child.load(fin);
            continue;
        }

        string key = word, value;
        line >> value;
        assert(value == "=");
        line.ignore(); getline(line, value);

        data[key] = value;        
    }
}


std::string read(const std::string& data, const Type2Type<std::string>&) 
    { return data; }

vec3 read(const std::string& data, const Type2Type<vec3>&) {
    vec3 val;
    stringstream buf(data);
    cout << " 1 " << endl;
    for (int i = 0; i < 3; i++)
        buf >> val(i);
    return val;
}
mat44 read(const std::string& data, const Type2Type<mat44>&) {
    mat44 val;
    stringstream buf(data);
    cout << " 2 " << endl;
    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
        buf >> val(i,j);
    return val;
}


std::string write(const vec3& obj) {
    stringstream buf;
    for (int i = 0; i < 3; i++)
        buf << obj(i) << " ";
    return buf.str();
}

std::string write(const mat44& obj) {
    stringstream buf;
    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
        buf << obj(i,j) << " ";
    return buf.str();
}

}
