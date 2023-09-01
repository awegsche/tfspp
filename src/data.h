#pragma once

#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <complex>

namespace tfs
{
    constexpr int FIELDWIDTH = 15;

    enum DataType {
        S,  // string
        LE, // float (double)
        D,  // int (decimal?)
        B,  // bool
        C,  // complex
    };

    template<typename real>
    struct data_value {
        DataType type;
        std::variant<int, real, bool, std::complex<real>, std::string> payload;

        data_value() :type(DataType::D), payload(0) {}
        data_value(int i) :type(DataType::D), payload(i) {}
        data_value(double d) :type(DataType::LE), payload(d) {}
        data_value(bool b) :type(DataType::B), payload(b) {}
        data_value(const std::string& s) : type(DataType::S), payload(s) {}

        data_value(data_value<real>&& other) : type(other.type), payload(std::move(other.payload)) { }

        template<typename U>
        data_value(const data_value<U>& other) : type(other.type), payload(other.payload) { }

        std::string pretty_print() const {
            std::ostringstream ss;
            switch (type) {
                case S:
                    ss << std::get<std::string>(payload); break;
                case D:
                    ss << std::get<int>(payload); break;
                case LE:
                    ss << std::get<real>(payload); break;
                case C:
                    ss << std::get<std::complex<real>>(payload); break;
                case B:
                    if (std::get<bool>(payload)) ss << "True";
                    else ss << "False";
             }
            return ss.str();
        }

        int get_int() const {
            if (type == DataType::D) return std::get<int>(payload);
            throw std::runtime_error("not an int");
        }
        real get_double() const {
            if (type == DataType::LE) return std::get<real>(payload);
            throw std::runtime_error("not a double");
        }
        std::complex<real> get_complex() const {
            if (type == DataType::C) return std::get<std::complex<real>>(payload);
            if (type == DataType::LE) return std::complex<real>(std::get<real>(payload), 0.0);
            throw std::runtime_error("not a complex");
        }
        const std::string& get_string() const {
            if (type == DataType::S) return std::get<std::string>(payload);
            throw std::runtime_error("not a string");
        }
    };

    template <typename real>
    struct data_vector {
        union udata_vec {
            std::vector<std::string> string_vector;
            std::vector<real> double_vector;
            std::vector<int> int_vector;
            std::vector<bool> bool_vector;

            udata_vec(): string_vector() {} // does nothing, construction is handled by parent
            ~udata_vec() {} // does nothing, destruction is handled by parent
        } payload;
        DataType type;
        std::string name;

        // ----------------------------------------------------------------------------------------
        // ---- Init ------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------
        /**
         * @brief Construct a new data vector object with the given datatype
         * 
         * @param t 
         * @param s 
         */
        data_vector(DataType t, const std::string& s) : type(t), name(s)  {
            
            payload.~udata_vec(); // get rid of the default string_vec inside;
            switch(t) {
            case DataType::B:
                break;
            case DataType::LE:
                payload.double_vector = std::vector<real>();
                break;
            case DataType::D:
                payload.int_vector = std::vector<int>();
                break;
            case DataType::S:
                payload.string_vector = std::vector<std::string>();
                break;
            }
            
        }

        /**
         * @brief Move construct
         * 
         * @param other 
         */
        data_vector(data_vector&& other) :type(std::move(other.type)), name(std::move(other.name)) {

            std::cout << "moving datavector" << name << "'\n";
            switch(other.type) {
            case DataType::B:
                payload.bool_vector = std::move(other.payload.bool_vector);
                break;
            case DataType::LE:
                payload.double_vector = std::move(other.payload.double_vector);
                break;
            case DataType::D:
                payload.int_vector = std::move(other.payload.int_vector);
                break;
            case DataType::S:
                payload.string_vector = std::move(other.payload.string_vector);
                break;
            }
        }

        /**
         *@brief Copy construct
         * 
         * @param other 
         */
        data_vector(const data_vector& other) :type(other.type), name(other.name) {

            //std::cout << "WARNING, copy of data_vector '" << name << "'\n";
            switch(other.type) {
            case DataType::B:
                payload.bool_vector = other.payload.bool_vector;
                break;
            case DataType::LE:
                payload.double_vector = other.payload.double_vector;
                break;
            case DataType::D:
                payload.int_vector = other.payload.int_vector;
                break;
            case DataType::S:
                payload.string_vector = other.payload.string_vector;
                break;
            }
        }

        ~data_vector() {
            switch (type) {
            case DataType::B:
                payload.bool_vector.~vector();
                break;
            case DataType::D:
                payload.int_vector.~vector();
                break;
            case DataType::LE:
                payload.double_vector.~vector();
                break;
            case DataType::S:
                payload.string_vector.~vector();
                break;
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---- Insertion -------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

        /**
         * @brief Converts `s` into the datatype of the current data_vector
         */
        void convert_back(const std::string& s) {
            char* end;
            switch(type) {
            case DataType::D:
                push_back(static_cast<int>(strtol(s.c_str(), &end, 10)));
                break;
            case DataType::LE:
                push_back(strtod(s.c_str(), &end));
                break;
            case DataType::S:
                push_back(s);
                break;
            }
        }

        void push_back(bool b) {
            if (type != DataType::B) throw std::runtime_error("this is not a bool vector");
            payload.bool_vector.push_back(b);
        }

        void push_back(int i) {
            as_int_vector_mut().push_back(i);
        }

        void push_back(double d) {
            as_real_vector_mut().push_back((static_cast<real>(d)));
        }

        void push_back(float d) {
            push_back(static_cast<double>(d));
        }

        void push_back(const std::string& s) {
            as_string_vector_mut().push_back(s);
        }

        // ----------------------------------------------------------------------------------------
        // ---- Extraction ------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

        [[nodiscard]]
        auto as_real_vector() const -> std::vector<real> const& {
            if (type != DataType::LE) throw std::runtime_error("this is not a double vector");
            return payload.double_vector;
        }

        [[nodiscard]]
        auto as_string_vector() const -> std::vector<std::string> const& {
            if (type != DataType::S) throw std::runtime_error("this is not a string vector");
            return payload.string_vector;
        }

        [[nodiscard]]
        auto as_int_vector() const -> std::vector<int> const& {
            if (type != DataType::D) throw std::runtime_error("this is not an int vector");
            return payload.int_vector;
        }

        // ---- and _mut versions -----------------------------------------------------------------

        [[nodiscard]]
        auto as_real_vector_mut() -> std::vector<real>& {
            return const_cast<std::vector<real>&>(static_cast<data_vector<real>*>(this)->as_real_vector());
        }

        [[nodiscard]]
        auto as_string_vector_mut() -> std::vector<std::string>& {
            return const_cast<std::vector<std::string>&>(static_cast<data_vector<real>*>(this)->as_string_vector());
        }

        [[nodiscard]]
        auto as_int_vector_mut() -> std::vector<int>& {
            return const_cast<std::vector<int>&>(static_cast<data_vector<real>*>(this)->as_int_vector());
        }

        // ----------------------------------------------------------------------------------------
        // ---- Properties ------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------

        [[nodiscard]] auto size() const -> size_t {
            switch(type) {
            case DataType::D:
                return payload.int_vector.size();
                break;
            case DataType::LE:
                return payload.double_vector.size();
                break;
            case DataType::S:
                return payload.string_vector.size();
                break;
            }
        }

        void print_at(size_t i, std::ostream& os) const {
            switch(type) {
            case DataType::D:
                os << std::setw(FIELDWIDTH) << payload.int_vector[i] << " ";
                break;
            case DataType::LE:
                os << std::setw(FIELDWIDTH) << payload.double_vector[i] << " ";
                break;
            case DataType::S:
                os << std::setw(FIELDWIDTH) << payload.string_vector[i] << " ";
                break;
            }
        }

    };
    inline DataType DT_from_string(const std::string& token) {
        if (token == "%d") return DataType::D;
        if (token == "%le") return DataType::LE;
        if (token == "%b") return DataType::B;
        return DataType::S; // ddefault to S is safe
    }
    inline const char* string_fromDT(DataType t)
    {
        switch (t)
        {
        case DataType::D:
            return "%d";
        case DataType::LE:
            return "%le";
        default:
            return "%s";
        }
    }

    template <typename real>
    std::ostream& operator<<(std::ostream& os, const data_value<real>& v)
    {
        return os << v.pretty_print();
    }
} // namespace tfs

