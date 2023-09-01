#pragma once
#include <map>
#include <utility>
#include <vector>
#include <variant>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <iterator>

#include "data.h"

namespace tfs
{
    template<typename real=double>
    class dataframe
    {
    public:

        // ----------------------------------------------------------------------------------------
        // ---- Init ------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------
        dataframe(){};
        explicit dataframe(const std::string &path, const std::string& index = "");

        // ----------------------------------------------------------------------------------------
        // ---- Columns ---------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------
        data_vector<real> &get_column(const std::string &name);
        data_vector<real> &get_column(size_t index) { return columns[index];}

        /**
         * @brief Reserves space for n columns
         */
        void reserve(size_t n) { columns.reserve(n); }

        /**
         * @brief rvalue version of `add_column`
         */
        void add_column(std::vector<real>&& vec, const std::string& name) {
            column_headers[name] = columns.size();
            data_vector<real> v(DataType::LE, name);
            v.payload.double_vector = std::move(vec);
            columns.push_back(std::move(v));
        }

        /**
         * @brief Moves a columns into the dataframe
         * 
         * @param vec 
         * @param name 
         */
        void add_column(const data_vector<real>& vec, const std::string& name) {
            column_headers[name] = columns.size();
            columns.push_back(vec);
        }

        /**
         * @brief Adds a double column to the dataframe.
         * 
         * @param vec 
         * @param name 
         */
        void add_column(const std::vector<double>& vec, const std::string& name) {
            column_headers[name] = columns.size();
            data_vector<real> v(DataType::LE, name);
            v.payload.double_vector = vec;
            columns.push_back(std::move(v));
        }

        /**
         * @brief Adds a string column to the dataframe.
         * 
         * @param vec 
         * @param name 
         */
        void add_column(const std::vector<std::string>& vec, const std::string& name) {
            column_headers[name] = columns.size();
            data_vector<real> v(DataType::S, name);
            v.payload.string_vector = vec;
            columns.push_back(std::move(v));
        }

        /**
         * @brief Adds an int column to the dataframe.
         * 
         * @param vec 
         * @param name 
         */
        void add_column(const std::vector<int>& vec, const std::string& name) {
            column_headers[name] = columns.size();
            data_vector<real> v(DataType::D, name);
            v.payload.int_vector = vec;
            columns.push_back(std::move(v));
        }

        /**
         * @brief Adds a column of arbitrary type to the dataframe
         * 
         * @param name 
         * @param t 
         * @return data_vector<real>& 
         */
        data_vector<real>& add_column(const std::string& name, DataType t) {
            column_headers[name] = columns.size();
            columns.emplace_back(t, name);

            return columns.back();
        }

        /**
         * @brief Writes the dataframe to a file int tfs format.
         * 
         * @param fname 
         */
        void to_file(const std::string& fname);

        // ----------------------------------------------------------------------------------------
        // ---- TFS Properties --------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

        data_value<real>& get_property(const std::string& key) { return properties[key];}

        template<typename T>
        void insert_property(std::string const& key, T const& value) {
            properties.insert(std::make_pair(key, data_value<real>{value}));
        }

        // ----------------------------------------------------------------------------------------
        // ---- Metadata --------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

        /**
         * @brief Returns the size of the df, i.e. number of rows
         */
        auto size() const -> size_t
        {
            if (columns.size() == 0)
                return 0;
            return columns[(*column_headers.begin()).second].size();
        }

        /**
         * @brief Get the index of the given key
         * 
         * @param key 
         * @return size_t 
         */
        size_t get_index(const std::string& key) { return idx[key]; }

        /**
         * @brief Returns a formatted description of the dataframe
         */
        auto pretty_print() const -> std::string;

        void verify() const;

            
        // ----------------------------------------------------------------------------------------
        // ---- Private Functions -----------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

    private:
        void read_property(const std::string& line);
        void read_column_headers(const std::string& line);
        void read_column_types(const std::string& line);
        void read_line(const std::string& line);
        void check_ini();

        // ----------------------------------------------------------------------------------------
        // ---- Private Fields --------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

    private:
        std::vector<data_vector<real>> columns;
        std::map<std::string, size_t> column_headers;
        std::map<std::string, data_value<real>> properties;
        std::map<std::string, size_t> idx;
        bool ini_complete = false;
    };


    template <class ContainerT>
    void tokenize(const std::string &str, ContainerT &tokens,
                  const std::string &delimiters = " ", bool trimEmpty = false)
    {
        std::string::size_type pos, lastPos = str.find_first_not_of(delimiters, 0), length = str.length();

        using value_type = typename ContainerT::value_type;
        using size_type = typename ContainerT::size_type;

        while (lastPos < length + 1)
        {
            pos = str.find_first_of(delimiters, lastPos);
            if (pos == std::string::npos)
            {
                pos = length;
            }

            if (pos != lastPos || !trimEmpty){
                tokens.push_back(
                    value_type(str.data() + lastPos, (size_type)pos - lastPos));
            }

            lastPos = str.find_first_not_of(delimiters, pos);
        }


    }

    // ---------------------------------------------------------------------------------------------
    // - implementation ----------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------

    template<typename real>
    dataframe<real>::dataframe(const std::string &path, const std::string& index) 
    {
        std::ifstream file(path);
        std::string line;
        while(std::getline(file, line) && !ini_complete) {
            if (line[0] == '@')
                read_property(line);
            else if (line[0] == '*')
                read_column_headers(line);
            else if (line[0] == '$')
                read_column_types(line);
            check_ini();
        }
        do {
            read_line(line);
        }
        while(std::getline(file, line));

        if (index.empty()) return;
        auto& index_col = get_column(index).payload.string_vector;
        for (int i = 0; i < index_col.size(); i++)
            idx.insert(std::make_pair(index_col[i], i));
    }

    template<typename real>
    data_vector<real>& dataframe<real>::get_column(const std::string& name)
    {
        return columns[column_headers[name]];
    }

    template<typename real>
    void dataframe<real>::to_file(const std::string& filename)
    {
        std::ofstream file(filename);

        if (!file.is_open())
            return;

        for (auto& kvp : properties)
        {
            file << "@ "
                 << std::setw(32) << kvp.first << " "
                 << std::setw(4) << string_fromDT(kvp.second.type) << " "
                 << kvp.second << "\n";
        }
        file << "* ";
        for (auto& kvp : column_headers)
            file << std::setw(FIELDWIDTH) << kvp.first << " ";
        file << "\n$ ";
        for (auto& c : column_headers)
            file << std::setw(FIELDWIDTH) << string_fromDT(columns[c.second].type) << " ";
        file << "\n";

        for (int i = 0; i < size(); i++)
        {
            file << "  ";
            for (auto& c : column_headers)
                columns[c.second].print_at(i, file);
            file << "\n";
        }
        file.close();
    }

    template<typename real>
    void dataframe<real>::read_property(const std::string& line)
    {
        std::vector<std::string> tokens;
        tokenize(line, tokens);
        auto t = DT_from_string(tokens[2]);

        switch (t)
        {
        case DataType::D:
        {
            char* pEnd;
            properties.insert(std::make_pair(tokens[1], data_value<real>((int)strtol(tokens[3].c_str(), &pEnd, 10))));
            break;
        }
        case DataType::LE:
        {
            char* pEnd;
            properties.insert(std::make_pair(tokens[1], data_value<real>(strtod(tokens[3].c_str(), &pEnd))));
            break;
        }

        default:
            // collapse string
            std::ostringstream ss;
            std::copy(tokens.begin() + 3, tokens.end(),
                      std::ostream_iterator<std::string>(ss, " "));
            properties.insert(std::make_pair(tokens[1], ss.str()));
        }
    }

    template<typename real>
    void dataframe<real>::read_column_headers(const std::string& line)
    {
        std::vector<std::string> tokens;
        tokenize(line, tokens);

        for (int i = 0; i < tokens.size() - 1; i++)
            column_headers.insert(std::make_pair(tokens[i + 1], i));
    }

template<typename real>
    void dataframe<real>::read_column_types(const std::string& line)
    {
        std::vector<std::string> tokens;
        tokenize(line, tokens);

        for (auto it = tokens.begin() + 1; it != tokens.end(); ++it)
        {
            columns.emplace_back(DT_from_string(*it), "");
        }
    }

template<typename real>
    void dataframe<real>::read_line(const std::string& line)
    {
        std::vector<std::string> tokens;
        tokenize(line, tokens);


        for (int i = 0; i < tokens.size(); i++)
        {
            columns[i].convert_back(tokens[i]);
        }
    }

template<typename real>
    void dataframe<real>::check_ini()
    {
        if (columns.size() > 0 && columns.size() == column_headers.size())
            ini_complete = true;
    }

template<typename real>
    std::string dataframe<real>::pretty_print() const
    {
        std::ostringstream ss;
        ss << "DataFrame{\n";
        ss << columns.size() << " columns, " << size() << " rows\n";
        ss << "Headers: \n";
        for (auto& pair : properties)
            ss << std::setw(32) << pair.first << ": " << pair.second << "\n";
        ss << "---\n";
        return ss.str();
    }

    template<typename real>
    void dataframe<real>::verify() const {
        using std::cout;
        using std::endl;
        cout << " -- verifying dataframe --\n";
        for (auto& kvp : column_headers) {
            cout << kvp.first << ": " << columns[kvp.second].size() << " elements \n";
        }
        cout << " per column in columns:\n";
        for(auto& c: columns) 
            cout << c.size()  << "elements\n";

        cout << endl;
    }
    template<typename real>
    std::ostream& operator<<(std::ostream& os, const dataframe<real>& df)
    {
        return (os << df.pretty_print());
    }

}  // namespace tfs
