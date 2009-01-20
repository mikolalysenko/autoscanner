//TODO: Document this

#ifndef CONFIG_H
#define CONFIG_H

#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>
#include <map>

#include <Eigen/Core>

namespace cfg {
    
    
    typedef Eigen::Vector3f vec3;
    typedef Eigen::Matrix4f mat44;
    
    template <typename T>
    class Type2Type {
        typedef T type;
    };

    class config {
    public:
        static config global;

        typedef std::map<std::string, std::string> data_map;
        typedef std::map<std::string, config> child_map;


    private:
        std::string name;       
        
        data_map data;
        child_map children;

        void read_data(std::ifstream& fin);

    public:
        config(const std::string& n = "") : name(n) { }

        template <typename T>
        T get(const std::string& key, const T& );

        template <typename T>
        T get(const std::string& key);

        config get(const std::string& key, const config&) { return children[key]; }

        template <typename T>
        void set(const std::string& key, const T& value);
        void set(const std::string& key, const config& value) { children[key] = value; }

        void save(const std::string& fname);
        void save(std::ofstream& fout, int level = 0);
        void load(const std::string& fname);
        void load(std::ifstream& fin);

        child_map::iterator childBegin() { return children.begin(); }
        child_map::iterator childEnd() { return children.end(); }

        data_map::iterator dataBegin() { return data.begin(); }
        data_map::iterator dataEnd() { return data.end(); }

    };

    template <typename T>
    T read(const std::string& data, const Type2Type<T>&) 
        { std::stringstream ss(data); T out; ss >> out; return out; }

    template <typename T>
    std::string write(const T& obj)
        { std::stringstream ss; ss << obj; return ss.str(); }

    std::string read(const std::string& data, const Type2Type<std::string>&);
    vec3 read(const std::string& data, const Type2Type<vec3>&);
    mat44 read(const std::string& data, const Type2Type<mat44>&);


    std::string write(const vec3& obj);
    std::string write(const mat44& obj);
    
    
    template <typename T>
    T config::get(const std::string& key, const T&) { return get<T>(key); }

    template <typename T>
    T config::get(const std::string& key) { return cfg::read(data[key], Type2Type<T>()); }

    template <typename T>
    void config::set(const std::string& key, const T& value) { data[key] = write(value); }
}

#endif




