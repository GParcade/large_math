#include "array_int.hpp"
namespace large_math{
template<uint64_t byte_expand>
    class real_t {
        union for_constructor {

            struct s {
                uint64_t unused : 63 - (byte_expand >= 57 ? 63 : (5 + byte_expand));
                uint64_t _dot_pos : (byte_expand >= 57 ? 63 : (5 + byte_expand));
                uint64_t _is_minus : 1;
            public:
                s() {

                }
                s(uint64_t val) {
                    _dot_pos = val;
                }
                void operator = (uint64_t val) {
                    _dot_pos = val;
                }
                operator uint64_t() const {
                    return _dot_pos;
                }
                bool is_minus() const {
                    return _is_minus;
                }
                bool modify_minus(bool bol) {
                    _is_minus = bol;
                }
            } dot_pos;
            array_int<byte_expand> signed_int;
            for_constructor() {}
        } val;

        static std::vector<std::string> split_dot(std::string value) {
            std::vector<std::string> strPairs;
            size_t pos = 0;
            if ((pos = value.find(".")) != std::string::npos) {
                strPairs.push_back(value.substr(0, pos));
                value.erase(0, pos + 1);
            }
            if (!value.empty())
                strPairs.push_back(value);
            return strPairs;
        }

        uint64_t temp_denormalize_struct() {
            uint64_t tmp = val.dot_pos;
            val.dot_pos = 0;
            return tmp;
        }
        void normalize_struct(uint64_t value) {
            val.dot_pos = value;
        }
        void normalize_dot(uint64_t cur_dot_pos = -1) {
            if (cur_dot_pos == (uint64_t)-1)
                cur_dot_pos = temp_denormalize_struct();

            if (cur_dot_pos == 0)
                return;

            while (val.dot_pos || cur_dot_pos > max_dot_pos) {
                val.signed_int /= 10;
                cur_dot_pos--;
            }
            std::string tmp_this_value = to_ansi_string();
            array_int<byte_expand>* remove_nuls_mult = new array_int<byte_expand>(10);
            array_int<byte_expand>* remove_nuls = new array_int<byte_expand>(1);
            size_t nul_count = 0;
            for (int64_t i = tmp_this_value.length() - 1; i >= 0; i--)
            {
                if (tmp_this_value[i] != '0')
                    break;
                else {
                    *remove_nuls *= *remove_nuls_mult;
                    nul_count++;
                }
            }
            delete remove_nuls_mult;
            uint64_t modify_dot = cur_dot_pos - nul_count;
            if (nul_count)
                val.signed_int /= *remove_nuls;
            delete remove_nuls;
            normalize_struct(modify_dot);
        }
        uint64_t denormalize_dot() {
            if (val.signed_int) {
                uint64_t this_dot_pos = temp_denormalize_struct();
                array_int<byte_expand>& mult_int = *new array_int<byte_expand>(10);
                for (;;) {
                    if ((val.signed_int * mult_int) / mult_int == val.signed_int && this_dot_pos != 31) {
                        val.signed_int *= mult_int;
                        this_dot_pos++;
                        mult_int *= 10;
                    }
                    else
                        break;
                }
                delete &mult_int;
                return this_dot_pos;
            }
            return 0;
        }
        void div(const real_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();

            this_dot_pos -= rhs_dot_pos;
            while (true) {
                if (!(tmp.val.signed_int % 10) && tmp.val.signed_int) {
                    if (this_dot_pos != 31) {
                        this_dot_pos++;
                        tmp.val.signed_int /= 10;
                        continue;
                    }
                }
                this_dot_pos += denormalize_dot();
                val.signed_int /= tmp.val.signed_int;
                break;
            }
            normalize_dot();
        }
        void mod(const real_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();

            this_dot_pos -= rhs_dot_pos;
            while (true) {
                if (!(tmp.val.signed_int % 10) && tmp.val.signed_int) {
                    if (this_dot_pos != 31) {
                        this_dot_pos++;
                        tmp.val.signed_int /= 10;
                        continue;
                    }
                }
                this_dot_pos += denormalize_dot();
                val.signed_int %= tmp.val.signed_int;
                break;
            }
            normalize_dot();
        }
    public:
        static const uint64_t max_dot_pos;
        void mod_dot(uint64_t pos) {
            val.dot_pos = pos;
        }
        static real_t get_min() {
            real_t tmp = 0;
            tmp.val.signed_int = array_int<byte_expand>::get_min();
            tmp.val.dot_pos = 0;
            return tmp;
        }
        static real_t get_max() {
            real_t tmp = 0;
            tmp.val.signed_int = array_int<byte_expand>::get_max();
            tmp.val.dot_pos = 0;
            return tmp;
        }

        real_t() {
            val.signed_int = 0;
        }
        real_t(const real_t& rhs) {
            val.signed_int = rhs.val.signed_int;
        }
        real_t(real_t&& rhs) {
            val.signed_int = rhs.val.signed_int;
        }
        real_t(const char* str) {
            *this = std::string(str);
        }
        real_t(const std::string str) {
            size_t found_pos = str.find('.');
            if (found_pos == std::string::npos)
            {
                val.signed_int = array_int<byte_expand>(str.c_str());
                val.dot_pos = 0;
            }
            else {
                //check str for second dot
                if (str.find('.', found_pos) == std::string::npos)
                    throw std::invalid_argument("Real value can contain only one dot");

                std::string tmp = str;
                tmp.erase(tmp.begin() + found_pos);
                val.signed_int = array_int<byte_expand>(tmp.c_str());
                val.dot_pos = (str.length() - found_pos - 1);
            }
            normalize_struct(temp_denormalize_struct());
        }
        template <uint64_t byt_expand>
        real_t(const array_int<byt_expand>& rhs) {
            *this = rhs.to_ansi_string().c_str();
        }
        template <typename T>
        real_t(const T& rhs)
        {
            val.signed_int = (array_int<byte_expand>)rhs;
        }
        template <typename T>
        real_t(const T&& rhs)
        {
            val.signed_int = (array_int<byte_expand>)rhs;
        }
        std::string to_ansi_string() {
            uint64_t tmp = temp_denormalize_struct();
            bool has_minus = val.dot_pos.is_minus();
            if (has_minus)
                val.dot_pos = -1;

            std::string str = val.signed_int.to_ansi_string();
            if (has_minus)
                str.erase(str.begin());
            normalize_struct(tmp);
            if (val.dot_pos) {
                if (str.length() <= val.dot_pos) {
                    uint64_t resize_result_len = val.dot_pos - str.length() + 1;
                    std::string to_add_zeros;
                    while (resize_result_len--)
                        to_add_zeros += '0';
                    str = to_add_zeros + str;
                }
                str.insert(str.end() - tmp, '.');
            }
            return (has_minus ? "-" : "") + str;
        }
        std::string to_ansi_string() const {
            return real_t(*this).to_ansi_string();
        }
        std::string to_hex_str() const {
            return val.signed_int.to_hex_str();
        }

        real_t& operator=(const real_t& rhs) = default;
        real_t& operator=(real_t&& rhs) = default;

        real_t operator&(const real_t& rhs) const {
            return real_t(*this) &= rhs;
        }

        real_t& operator&=(const real_t& rhs) {
            val.signed_int &= rhs.val.signed_int;
            return *this;
        }

        real_t operator|(const real_t& rhs) const {
            return real_t(*this) |= rhs;
        }

        real_t& operator|=(const real_t& rhs) {
            val.signed_int |= rhs.val.signed_int;
            return *this;
        }

        real_t operator^(const real_t& rhs) const {
            return real_t(*this) ^= rhs;
        }

        real_t& operator^=(const real_t& rhs) {
            val.signed_int ^= rhs.val.signed_int;
            return *this;
        }

        real_t operator~() const {
            real_t tmp(*this);
            ~tmp.val.signed_int;
            return tmp;
        }



        real_t& operator<<=(uint64_t shift) {
            val.signed_int <<= shift;
            return *this;
        }
        real_t operator<<(uint64_t shift) const {
            return real_t(*this) <<= shift;
        }


        real_t& operator>>=(uint64_t shift) {
            val.signed_int >>= shift;
            return *this;
        }
        real_t operator>>(uint64_t shift) const {
            return real_t(*this) >>= shift;
        }
        bool operator!() const {
            return !val.signed_int;
        }

        bool operator&&(const real_t& rhs) const {
            return ((bool)*this && rhs);
        }

        bool operator||(const real_t& rhs) const {
            return ((bool)*this || rhs);
        }

        bool operator==(const real_t& rhs) const {
            return val.signed_int == rhs.val.signed_int;
        }

        bool operator!=(const real_t& rhs) const {
            return val.signed_int != rhs.val.signed_int;
        }

        bool operator>(const real_t& rhs) const {
            std::vector<std::string> this_parts = split_dot(to_ansi_string());
            std::vector<std::string> rhs_parts = split_dot(rhs.to_ansi_string());
            {
                array_int<byte_expand> temp1(this_parts[0].c_str());
                array_int<byte_expand> temp2(rhs_parts[0].c_str());
                if (temp1 == temp2);
                else return temp1 > temp2;
            }
            if (this_parts.size() == 2 && rhs_parts.size() == 2) {
                array_int<byte_expand> temp1(this_parts[1].c_str());
                array_int<byte_expand> temp2(rhs_parts[1].c_str());
                return temp1 > temp2;
            }
            else
                return this_parts.size() > rhs_parts.size();
        }

        bool operator<(const real_t& rhs) const {
            return !(*this > rhs) && *this != rhs;
        }

        bool operator>=(const real_t& rhs) const {
            return ((*this > rhs) | (*this == rhs));
        }

        bool operator<=(const real_t& rhs) const {
            return !(*this > rhs);
        }


        real_t& operator++() {
            return *this += 1;
        }
        real_t operator++(int) {
            real_t temp(*this);
            *this += 1;
            return temp;
        }
        real_t operator+(const real_t& rhs) const {
            return real_t(*this) += rhs;
        }
        real_t operator-(const real_t& rhs) const {
            return real_t(*this) -= rhs;
        }

        real_t& operator+=(const real_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                array_int<byte_expand> move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.signed_int *= move_tmp;
            }
            else {
                array_int<byte_expand> move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.signed_int *= move_tmp;
            }
            val.signed_int += tmp.val.signed_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }
        real_t& operator-=(const real_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                array_int<byte_expand> move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.signed_int *= move_tmp;
            }
            else {
                array_int<byte_expand> move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.signed_int *= move_tmp;
            }
            val.signed_int -= tmp.val.signed_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        real_t operator*(const real_t& rhs) const {
            return real_t(*this) *= rhs;
        }


        real_t& operator*=(const real_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            this_dot_pos += rhs_dot_pos;

            val.signed_int *= tmp.val.signed_int;

            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        real_t operator/(const real_t& rhs) const {
            return real_t(*this) /= rhs;
        }

        real_t& operator/=(const real_t& rhs) {
            div(rhs);
            return *this;
        }

        real_t operator%(const real_t& rhs) const {
            return real_t(*this) %= rhs;
        }

        real_t& operator%=(const real_t& rhs) {
            mod(rhs);
            return *this;
        }

        explicit operator bool() const {
            return (bool)val.signed_int;
        }
        explicit operator uint8_t() const {
            return (uint8_t)val.signed_int;
        }
        explicit operator uint16_t() const {
            return (uint16_t)val.signed_int;
        }
        explicit operator uint32_t() const {
            return (uint32_t)val.signed_int;
        }
        explicit operator uint64_t() const {
            return (uint64_t)val.signed_int;
        }


        template <uint64_t byt_expand>
        explicit operator array_int<byt_expand>() const {
            real_t tmp = *this;
            array_int<byte_expand> div = 10;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; temp++)
                tmp.val.signed_int /= div;
            return tmp.val.signed_int;
        }
        template <uint64_t byt_expand>
        explicit operator array_uint<byt_expand>() const {
            return (array_uint<byt_expand>)((array_int<byt_expand>)*this);
        }
        template <uint64_t byt_expand>
        explicit operator real_t<byt_expand>() {
            return to_ansi_string().c_str();
        }

        explicit operator double() const {
            real_t tmp = *this;
            array_uint<byte_expand> div = 10;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; i++)
                tmp.val.signed_int /= div;

            double res = (int64_t)tmp.val.signed_int;
            for (uint64_t i = 0; i < temp; i++)
                tmp.val.signed_int *= div;
            tmp = *this - tmp.val.signed_int;
            double res_part = (int64_t)tmp.val.signed_int;;
            for (uint64_t i = 0; i < temp; i++)
                res_part /= 10;
            return res + res_part;
        }

    };

    template<uint64_t byte_expand>
    const uint64_t real_t<byte_expand>::max_dot_pos = []() {
        uint64_t _dot_pos : (byte_expand >= 58 ? 64 : (5 + byte_expand));
        _dot_pos = -1;
        return _dot_pos;
    }();


    template<uint64_t byte_expand>
    class unreal_t {
        union for_constructor {
            struct s {
                uint64_t unused : 64 - (byte_expand >= 57 ? 63 : (5 + byte_expand));
                uint64_t _dot_pos : (byte_expand >= 58 ? 64 : (5 + byte_expand));
            public:
                s() {

                }
                s(uint64_t val) {
                    _dot_pos = val;
                }
                void operator = (uint64_t val) {
                    _dot_pos = val;
                }
                operator uint64_t() const {
                    return _dot_pos;
                }
            } dot_pos;
            array_uint<byte_expand> unsigned_int;
            for_constructor() {}
        } val;
        static std::vector<std::string> split_dot(std::string value) {
            std::vector<std::string> strPairs;
            size_t pos = 0;
            if ((pos = value.find(".")) != std::string::npos) {
                strPairs.push_back(value.substr(0, pos));
                value.erase(0, pos + 1);
            }
            if (!value.empty())
                strPairs.push_back(value);
            return strPairs;
        }
        uint64_t temp_denormalize_struct() {
            uint64_t tmp = val.dot_pos;
            val.dot_pos = 0;
            return tmp;
        }
        void normalize_struct(uint64_t value) {
            val.dot_pos = value;
        }
        void normalize_dot(uint64_t cur_dot_pos = -1) {
            if (cur_dot_pos == (uint64_t)-1)
                cur_dot_pos = temp_denormalize_struct();

            if (cur_dot_pos == 0)
                return;

            array_uint<byte_expand> shrinker(10);
            while (val.dot_pos || cur_dot_pos > max_dot_pos) {
                val.unsigned_int /= shrinker;
                cur_dot_pos--;
            }
            size_t nul_count = 0;
            while(val.unsigned_int % shrinker != 0){
                val.unsigned_int /= shrinker;
                nul_count++;
            }
            normalize_struct(cur_dot_pos - nul_count);
        }

        void denormalize_dot_fixer(array_uint<byte_expand>& src, array_uint<byte_expand>& dst, uint64_t& dot_pos, uint64_t add, uint64_t limit) {
            array_uint<byte_expand>& old = *new array_uint<byte_expand>(src);
            for (;;) {
                if (dot_pos + add <= limit) {
                    src *= dst;
                    if (old != src / dst) {
                        src = old;
                        break;
                    }
                    old = src;
                    dot_pos += add;
                }
                else break;
            }
            delete &old;
        }

        uint64_t denormalize_dot() {
            if (val.unsigned_int) {


                uint64_t this_dot_pos = temp_denormalize_struct();
                array_uint<byte_expand> move_tmp;
                if (max_dot_pos > 2432) {
                    move_tmp = "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 2432, max_dot_pos);
                }
                if (max_dot_pos > 1216) {
                    move_tmp = "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 1216, max_dot_pos);
                }
                if (max_dot_pos > 608) {
                    move_tmp = "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 608, max_dot_pos);
                }
                if (max_dot_pos > 304) {
                    move_tmp = "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 304, max_dot_pos);
                }
                if (max_dot_pos > 152) {
                    move_tmp = "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 152, max_dot_pos);
                }
                if (max_dot_pos > 76) {
                    move_tmp = "10000000000000000000000000000000000000000000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 76, max_dot_pos);
                }
                if (max_dot_pos > 38) {
                    move_tmp = "100000000000000000000000000000000000000";
                    denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 38, max_dot_pos);
                }
                move_tmp = 10000000000000000000;
                denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 19, max_dot_pos);
                move_tmp = 100000;
                denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos, 5, max_dot_pos);
                move_tmp = 10;
                denormalize_dot_fixer(val.unsigned_int, move_tmp, this_dot_pos,1, max_dot_pos);

                return this_dot_pos;
            }
            return 0;
        }
        void div(const unreal_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();

            this_dot_pos -= rhs_dot_pos;


            array_uint<byte_expand> move_tmp = 10;
            while (true) {
                if (!(tmp.val.unsigned_int % move_tmp) && tmp.val.unsigned_int) {
                    if (this_dot_pos <= max_dot_pos) {
                        this_dot_pos++;
                        tmp.val.unsigned_int /= move_tmp;
                        continue;
                    }
                }
                this_dot_pos += denormalize_dot();
                val.unsigned_int /= tmp.val.unsigned_int;
                break;
            }
            normalize_dot(this_dot_pos);
        }
        void mod(const unreal_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();

            this_dot_pos -= rhs_dot_pos;


            array_uint<byte_expand> move_tmp = 10;
            while (true) {
                if (val.unsigned_int % tmp.val.unsigned_int) {
                    this_dot_pos++;
                    if (this_dot_pos != max_dot_pos) {
                        val.unsigned_int = val.unsigned_int * move_tmp;
                        continue;
                    }
                    this_dot_pos--;
                }
                this_dot_pos += denormalize_dot();
                val.unsigned_int %= tmp.val.unsigned_int;
                break;
            }
            normalize_dot(this_dot_pos);
        }
    public:
        static const uint64_t max_dot_pos;
        void mod_dot(uint64_t pos) {
            val.dot_pos = pos;
        }
        static unreal_t get_min() {
            return 0;
        }
        static unreal_t get_max() {
            unreal_t tmp = 0;
            tmp.val.unsigned_int = array_uint<byte_expand>::get_max();
            tmp.val.dot_pos = 0;
            return tmp;
        }

        unreal_t() {
            val.unsigned_int = 0;
        }
        unreal_t(const unreal_t& rhs) {
            val.unsigned_int = rhs.val.unsigned_int;
        }
        unreal_t(unreal_t&& rhs) {
            val.unsigned_int = rhs.val.unsigned_int;
        }
        unreal_t(const char* str) {
            *this = std::string(str);
        }
        unreal_t(const std::string str) {
            size_t found_pos = str.find('.');
            if (found_pos == std::string::npos)
            {
                val.unsigned_int = array_uint<byte_expand>(str.c_str());
                val.dot_pos = 0;
            }
            else {
                //check str for second dot
                if (str.find('.', found_pos) == std::string::npos)
                    throw std::invalid_argument("Real value can contain only one dot");

                std::string tmp = str;
                tmp.erase(tmp.begin() + found_pos);
                val.unsigned_int = array_uint<byte_expand>(tmp.c_str());
                val.dot_pos = (str.length() - found_pos - 1);
            }
            normalize_struct(temp_denormalize_struct());
        }
        template <uint64_t byt_expand>
        unreal_t(const unreal_t<byt_expand> rhs) {
            *this = rhs.to_ansi_string().c_str();
        }


        template <typename T>
        unreal_t(const T& rhs)
        {
            val.unsigned_int = (array_uint<byte_expand>)rhs;
        }
        template <typename T>
        unreal_t(const T&& rhs)
        {
            val.unsigned_int = (array_uint<byte_expand>)rhs;
        }
        std::string to_ansi_string() {
            uint64_t tmp = temp_denormalize_struct();
            std::string str = val.unsigned_int.to_ansi_string();
            normalize_struct(tmp);
            if (val.dot_pos) {
                if (str.length() <= val.dot_pos) {
                    uint64_t resize_result_len = val.dot_pos - str.length() + 1;
                    std::string to_add_zeros;
                    while (resize_result_len--)
                        to_add_zeros += '0';
                    str = to_add_zeros + str;
                }
                str.insert(str.end() - tmp, '.');
            }
            return str;
        }
        std::string to_ansi_string() const {
            return unreal_t(*this).to_ansi_string();
        }
        std::string to_hex_str() const {
            return val.unsigned_int.to_hex_str();
        }

        unreal_t& operator=(const unreal_t& rhs) = default;
        unreal_t& operator=(unreal_t&& rhs) = default;

        unreal_t operator&(const unreal_t& rhs) const {
            return unreal_t(*this) &= rhs;
        }

        unreal_t& operator&=(const unreal_t& rhs) {
            val.unsigned_int &= rhs.val.unsigned_int;
            return *this;
        }

        unreal_t operator|(const unreal_t& rhs) const {
            return unreal_t(*this) |= rhs;
        }

        unreal_t& operator|=(const unreal_t& rhs) {
            val.unsigned_int |= rhs.val.unsigned_int;
            return *this;
        }

        unreal_t operator^(const unreal_t& rhs) const {
            return unreal_t(*this) ^= rhs;
        }

        unreal_t& operator^=(const unreal_t& rhs) {
            val.unsigned_int ^= rhs.val.unsigned_int;
            return *this;
        }

        unreal_t operator~() const {
            unreal_t tmp(*this);
            ~tmp.val.unsigned_int;
            return tmp;
        }



        unreal_t& operator<<=(uint64_t shift) {
            val.unsigned_int <<= shift;
            return *this;
        }
        unreal_t operator<<(uint64_t shift) const {
            return unreal_t(*this) <<= shift;
        }


        unreal_t& operator>>=(uint64_t shift) {
            val.unsigned_int >>= shift;
            return *this;
        }
        unreal_t operator>>(uint64_t shift) const {
            return unreal_t(*this) >>= shift;
        }
        bool operator!() const {
            return !val.unsigned_int;
        }

        bool operator&&(const unreal_t& rhs) const {
            return ((bool)*this && rhs);
        }

        bool operator||(const unreal_t& rhs) const {
            return ((bool)*this || rhs);
        }

        bool operator==(const unreal_t& rhs) const {
            return val.unsigned_int == rhs.val.unsigned_int;
        }

        bool operator!=(const unreal_t& rhs) const {
            return val.unsigned_int != rhs.val.unsigned_int;
        }

        bool operator>(const unreal_t& rhs) const {
            std::vector<std::string> this_parts = split_dot(to_ansi_string());
            std::vector<std::string> rhs_parts = split_dot(rhs.to_ansi_string());
            {
                array_uint<byte_expand> temp1(this_parts[0].c_str());
                array_uint<byte_expand> temp2(rhs_parts[0].c_str());
                if (temp1 == temp2);
                else return temp1 > temp2;
            }
            if (this_parts.size() == 2 && rhs_parts.size() == 2) {
                array_uint<byte_expand> temp1(this_parts[1].c_str());
                array_uint<byte_expand> temp2(rhs_parts[1].c_str());
                return temp1 > temp2;
            }
            else
                return this_parts.size() > rhs_parts.size();
        }

        bool operator<(const unreal_t& rhs) const {
            return !(*this > rhs) && *this != rhs;
        }

        bool operator>=(const unreal_t& rhs) const {
            return ((*this > rhs) | (*this == rhs));
        }

        bool operator<=(const unreal_t& rhs) const {
            return !(*this > rhs);
        }


        unreal_t& operator++() {
            return *this += 1;
        }
        unreal_t operator++(int) {
            unreal_t temp(*this);
            *this += 1;
            return temp;
        }
        unreal_t operator+(const unreal_t& rhs) const {
            return unreal_t(*this) += rhs;
        }
        unreal_t operator-(const unreal_t& rhs) const {
            return unreal_t(*this) -= rhs;
        }

        unreal_t& operator+=(const unreal_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                array_uint<byte_expand> move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.unsigned_int *= move_tmp;
            }
            else {
                array_uint<byte_expand> move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.unsigned_int *= move_tmp;
            }
            val.unsigned_int += tmp.val.unsigned_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }
        unreal_t& operator-=(const unreal_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                array_uint<byte_expand> move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.unsigned_int *= move_tmp;
            }
            else {
                array_uint<byte_expand> move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.unsigned_int *= move_tmp;
            }
            val.unsigned_int -= tmp.val.unsigned_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        unreal_t operator*(const unreal_t& rhs) const {
            return unreal_t(*this) *= rhs;
        }


        unreal_t& operator*=(const unreal_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            this_dot_pos += rhs_dot_pos;

            val.unsigned_int *= tmp.val.unsigned_int;

            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        unreal_t operator/(const unreal_t& rhs) const {
            return unreal_t(*this) /= rhs;
        }

        unreal_t& operator/=(const unreal_t& rhs) {
            div(rhs);
            return *this;
        }

        unreal_t operator%(const unreal_t& rhs) const {
            return unreal_t(*this) %= rhs;
        }

        unreal_t& operator%=(const unreal_t& rhs) {
            mod(rhs);
            return *this;
        }

        explicit operator bool() const {
            return (bool)val.unsigned_int;
        }
        explicit operator uint8_t() const {
            return (uint8_t)(array_uint<byte_expand>)val.unsigned_int;
        }
        explicit operator uint16_t() const {
            return (uint16_t)(array_uint<byte_expand>)val.unsigned_int;
        }
        explicit operator uint32_t() const {
            return (uint32_t)(array_uint<byte_expand>)val.unsigned_int;
        }
        explicit operator uint64_t() const {
            return (uint64_t)(array_uint<byte_expand>)val.unsigned_int;
        }


        template <uint64_t byt_expand>
        explicit operator array_int<byt_expand>() const {
            return (array_int<byt_expand>)((array_uint<byt_expand>)*this);
        }
        template <uint64_t byt_expand>
        explicit operator array_uint<byt_expand>() const {
            unreal_t tmp = *this;
            array_uint<byte_expand> div = 10;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; temp++)
                tmp.val.unsigned_int /= div;
            return tmp.val.unsigned_int;
        }
        template <uint64_t byt_expand>
        explicit operator real_t<byt_expand>() const {
            return to_ansi_string().c_str();
        }
        template <uint64_t byt_expand>
        explicit operator unreal_t<byt_expand>() {
            return to_ansi_string().c_str();
        }

        explicit operator double() const {
            unreal_t tmp = *this;
            array_uint<byte_expand> div = 10;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; i++)
                tmp.val.unsigned_int /= div;

            double res = (uint64_t)tmp.val.unsigned_int;
            for (uint64_t i = 0; i < temp; i++)
                tmp.val.unsigned_int *= div;
            tmp = *this - tmp.val.unsigned_int;

            double res_part = (uint64_t)tmp.val.unsigned_int;;
            for (uint64_t i = 0; i < temp; i++)
                res_part /= 10;
            return res + res_part;
        }
    };
    template<uint64_t byte_expand>
    const uint64_t unreal_t<byte_expand>::max_dot_pos = []() {
        uint64_t _dot_pos : (byte_expand >= 58 ? 64 : (5 + byte_expand));
        _dot_pos = -1;
        return _dot_pos;
    }();

    class real64_t {
        union for_constructor {
            struct s {
            public:
                uint64_t unused : 58;
                uint64_t _dot_pos : 5;
                uint64_t _is_minus : 1;
                s() {

                }
                s(uint64_t val) {
                    _dot_pos = val;
                }
                void operator = (uint64_t val) {
                    _dot_pos = val;
                }
                operator uint64_t() const {
                    return _dot_pos;
                }
                bool is_minus() const {
                    return _is_minus;
                }
                void modify_minus(bool bol) {
                    _is_minus = bol;
                }
            } dot_pos;
            int64_t signed_int = 0;
            for_constructor() {}
        } val;


        static std::vector<std::string> split_dot(std::string value) {
            std::vector<std::string> strPairs;
            size_t pos = 0;
            if ((pos = value.find(".")) != std::string::npos) {
                strPairs.push_back(value.substr(0, pos));
                value.erase(0, pos + 1);
            }
            if (!value.empty())
                strPairs.push_back(value);
            return strPairs;
        }

        uint64_t temp_denormalize_struct() {
            uint64_t tmp = val.dot_pos;
            val.dot_pos = 0;
            return tmp;
        }
        void normalize_struct(uint64_t value) {
            val.dot_pos = value;
        }
        void normalize_dot(uint64_t cur_dot_pos = -1) {
            if (cur_dot_pos == (uint64_t)-1)
                cur_dot_pos = temp_denormalize_struct();

            if (cur_dot_pos == 0)
                return;

            while (val.dot_pos || cur_dot_pos > 31) {
                val.signed_int /= 10;
                cur_dot_pos--;
            }
            std::string tmp_this_value = to_ansi_string();
            int64_t remove_nuls_mult(10);
            int64_t remove_nuls(1);
            size_t nul_count = 0;
            for (int64_t i = tmp_this_value.length() - 1; i >= 0; i--)
            {
                if (tmp_this_value[i] != '0')
                    break;
                else {
                    remove_nuls *= remove_nuls_mult;
                    nul_count++;
                }
            }
            uint64_t modify_dot = cur_dot_pos - nul_count;
            if (nul_count)
                val.signed_int /= remove_nuls;
            normalize_struct(modify_dot);
        }

        uint64_t denormalize_dot() {
            if (val.signed_int) {
                uint64_t this_dot_pos = temp_denormalize_struct();
                uint64_t mult_int = 10;
                for (char i = 0; i < 24; i++) {
                    if ((val.signed_int * mult_int) / 10 == val.signed_int && this_dot_pos != 31) {
                        val.signed_int *= mult_int;
                        this_dot_pos++;
                        mult_int *= 10;
                    }
                    else
                        break;
                }
                return this_dot_pos;
            }
            return 0;
        }

        void div(const real64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            this_dot_pos -= rhs_dot_pos;
            while (true) {
                if (!(tmp.val.signed_int % 10) && tmp.val.signed_int) {
                    if (this_dot_pos != 31) {
                        this_dot_pos++;
                        tmp.val.signed_int /= 10;
                        continue;
                    }
                }
                this_dot_pos += denormalize_dot();
                val.signed_int /= tmp.val.signed_int;
                break;
            }
            normalize_dot(this_dot_pos);
        }
        void mod(const real64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            this_dot_pos -= rhs_dot_pos;
            while (true) {
                if (!(tmp.val.signed_int % 10) && tmp.val.signed_int) {
                    if (this_dot_pos != 31) {
                        this_dot_pos++;
                        tmp.val.signed_int /= 10;
                        continue;
                    }
                }
                this_dot_pos += denormalize_dot();
                val.signed_int %= tmp.val.signed_int;
                break;
            }
            normalize_dot(this_dot_pos);
        }
    public:
        void mod_dot(uint64_t pos) {
            val.dot_pos = pos;
        }
        static real64_t get_min() {
            real64_t tmp = 0;
            tmp.val.signed_int = 0x8000000000000000;
            tmp.val.dot_pos = 0;
            return tmp;
        }
        static real64_t get_max() {
            real64_t tmp = 0;
            tmp.val.signed_int = 0x7FFFFFFFFFFFFFFF;
            tmp.val.dot_pos = 0;
            return tmp;
        }

        real64_t() {
            val.signed_int = 0;
        }
        real64_t(const real64_t& rhs) {
            val.signed_int = rhs.val.signed_int;
        }
        real64_t(real64_t&& rhs) noexcept {
            val.signed_int = rhs.val.signed_int;
        }
        real64_t(const char* str) {
            *this = std::string(str);
        }
        real64_t(const std::string str) {
            size_t found_pos = str.find('.');
            if (found_pos == std::string::npos)
            {
                val.signed_int = std::stoull(str.c_str());
                val.dot_pos = 0;
            }
            else {
                //check str for second dot
                if (str.find('.', found_pos) == std::string::npos)
                    throw std::invalid_argument("Real value can contain only one dot");

                std::string tmp = str;
                tmp.erase(tmp.begin() + found_pos);
                val.signed_int = std::stoull(tmp.c_str());
                val.dot_pos = (str.length() - found_pos - 1);
            }
            normalize_struct(temp_denormalize_struct());
        }
        template <uint64_t byt_expand>
        real64_t(const array_int<byt_expand>& rhs) {
            *this = rhs.to_ansi_string().c_str();
        }
        template <typename T>
        real64_t(const T& rhs)
        {
            val.signed_int = (int64_t)rhs;
        }
        template <typename T>
        real64_t(const T&& rhs)
        {
            val.signed_int = (int64_t)rhs;
        }
        std::string to_ansi_string() {
            uint64_t tmp = temp_denormalize_struct();
            bool has_minus = val.dot_pos.is_minus();
            if (has_minus)
                val.dot_pos = -1;

            std::string str = std::to_string(val.signed_int);
            if (has_minus)
                str.erase(str.begin());
            normalize_struct(tmp);
            if (val.dot_pos) {
                if (str.length() <= val.dot_pos) {
                    uint64_t resize_result_len = val.dot_pos - str.length() + 1;
                    std::string to_add_zeros;
                    while (resize_result_len--)
                        to_add_zeros += '0';
                    str = to_add_zeros + str;
                }
                str.insert(str.end() - tmp, '.');
            }
            return (has_minus ? "-" : "") + str;
        }
        std::string to_ansi_string() const {
            return real64_t(*this).to_ansi_string();
        }

        real64_t& operator=(const real64_t& rhs) = default;
        real64_t& operator=(real64_t&& rhs) = default;

        real64_t operator&(const real64_t& rhs) const {
            return real64_t(*this) &= rhs;
        }

        real64_t& operator&=(const real64_t& rhs) {
            val.signed_int &= rhs.val.signed_int;
            return *this;
        }

        real64_t operator|(const real64_t& rhs) const {
            return real64_t(*this) |= rhs;
        }

        real64_t& operator|=(const real64_t& rhs) {
            val.signed_int |= rhs.val.signed_int;
            return *this;
        }

        real64_t operator^(const real64_t& rhs) const {
            return real64_t(*this) ^= rhs;
        }

        real64_t& operator^=(const real64_t& rhs) {
            val.signed_int ^= rhs.val.signed_int;
            return *this;
        }

        real64_t operator~() const {
            real64_t tmp(*this);
            tmp.val.signed_int = ~tmp.val.signed_int;
            return tmp;
        }



        real64_t& operator<<=(uint64_t shift) {
            val.signed_int <<= shift;
            return *this;
        }
        real64_t operator<<(uint64_t shift) const {
            return real64_t(*this) <<= shift;
        }


        real64_t& operator>>=(uint64_t shift) {
            val.signed_int >>= shift;
            return *this;
        }
        real64_t operator>>(uint64_t shift) const {
            return real64_t(*this) >>= shift;
        }
        bool operator!() const {
            return !val.signed_int;
        }

        bool operator&&(const real64_t& rhs) const {
            return ((bool)*this && rhs);
        }

        bool operator||(const real64_t& rhs) const {
            return ((bool)*this || rhs);
        }

        bool operator==(const real64_t& rhs) const {
            return val.signed_int == rhs.val.signed_int;
        }

        bool operator!=(const real64_t& rhs) const {
            return val.signed_int != rhs.val.signed_int;
        }

        bool operator>(const real64_t& rhs) const {
            std::vector<std::string> this_parts = split_dot(to_ansi_string());
            std::vector<std::string> rhs_parts = split_dot(rhs.to_ansi_string());
            {
                uint64_t temp1 = std::stoull(this_parts[0].c_str());
                uint64_t temp2 = std::stoull(rhs_parts[0].c_str());
                if (temp1 == temp2);
                else return temp1 > temp2;
            }
            if (this_parts.size() == 2 && rhs_parts.size() == 2) {
                uint64_t temp1 = std::stoull(this_parts[1].c_str());
                uint64_t temp2 = std::stoull(rhs_parts[1].c_str());
                return temp1 > temp2;
            }
            else
                return this_parts.size() > rhs_parts.size();
        }

        bool operator<(const real64_t& rhs) const {
            return !(*this > rhs) && *this != rhs;
        }

        bool operator>=(const real64_t& rhs) const {
            return ((*this > rhs) || (*this == rhs));
        }

        bool operator<=(const real64_t& rhs) const {
            return !(*this > rhs);
        }


        real64_t& operator++() {
            return *this += 1;
        }
        real64_t operator++(int) {
            real64_t temp(*this);
            *this += 1;
            return temp;
        }
        real64_t operator+(const real64_t& rhs) const {
            return real64_t(*this) += rhs;
        }
        real64_t operator-(const real64_t& rhs) const {
            return real64_t(*this) -= rhs;
        }

        real64_t& operator+=(const real64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                uint64_t move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.signed_int *= move_tmp;
            }
            else {
                uint64_t move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.signed_int *= move_tmp;
            }
            val.signed_int += tmp.val.signed_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }
        real64_t& operator-=(const real64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                uint64_t move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.signed_int *= move_tmp;
            }
            else {
                uint64_t move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.signed_int *= move_tmp;
            }
            val.signed_int -= tmp.val.signed_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        real64_t operator*(const real64_t& rhs) const {
            return real64_t(*this) *= rhs;
        }


        real64_t& operator*=(const real64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            real64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            this_dot_pos += rhs_dot_pos;

            val.signed_int *= tmp.val.signed_int;

            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        real64_t operator/(const real64_t& rhs) const {
            return real64_t(*this) /= rhs;
        }

        real64_t& operator/=(const real64_t& rhs) {
            div(rhs);
            return *this;
        }

        real64_t operator%(const real64_t& rhs) const {
            return real64_t(*this) %= rhs;
        }

        real64_t& operator%=(const real64_t& rhs) {
            mod(rhs);
            return *this;
        }

        explicit operator bool() const {
            return (bool)val.signed_int;
        }
        explicit operator uint8_t() const {
            return (uint8_t)val.signed_int;
        }
        explicit operator uint16_t() const {
            return (uint16_t)val.signed_int;
        }
        explicit operator uint32_t() const {
            return (uint32_t)val.signed_int;
        }
        explicit operator uint64_t() const {
            real64_t tmp = *this;
            tmp.temp_denormalize_struct();
            return tmp.val.signed_int;
        }


        explicit operator int64_t() const {
            real64_t tmp = *this;
            tmp.temp_denormalize_struct();
            return tmp.val.signed_int;
        }
        template <uint64_t byt_expand>
        explicit operator real_t<byt_expand>() {
            return to_ansi_string().c_str();
        }

        explicit operator double() const {
            real64_t tmp = *this;
            uint64_t div = 10;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; temp++)
                tmp.val.signed_int /= div;

            double res = (int64_t)tmp.val.signed_int;
            for (uint64_t i = 0; i < temp && i < 15; temp++)
                tmp.val.signed_int *= div;
            tmp = *this - tmp.val.signed_int;
            double res_part = (int64_t)tmp.val.signed_int;;
            for (uint64_t i = 0; i < temp && i<15; temp++)
                res_part /= 10;
            return res + res_part;
        }

    };

    class unreal64_t {
        union for_constructor {
            struct s {
                uint64_t unused : 59;
                uint64_t _dot_pos : 5;
            public:
                s() {

                }
                s(uint64_t val) {
                    _dot_pos = val;
                }
                void operator = (uint64_t val) {
                    _dot_pos = val;
                }
                operator uint64_t() const {
                    return _dot_pos;
                }
            } dot_pos;
            uint64_t unsigned_int;
            for_constructor() {}
        } val;
        static std::vector<uint64_t> split_dot(std::string value) {
            std::vector<uint64_t> strPairs;
            size_t pos = 0;
            if ((pos = value.find(".")) != std::string::npos) {
                strPairs.push_back(std::stoull(value.substr(0, pos)));
                value.erase(0, pos + 1);
            }
            if (!value.empty())
                strPairs.push_back(std::stoull(value));
            return strPairs;
        }

        uint64_t temp_denormalize_struct() {
            uint64_t tmp = val.dot_pos;
            val.dot_pos = 0;
            return tmp;
        }
        void normalize_struct(uint64_t value) {
            val.dot_pos = value;
        }
        void normalize_dot(uint64_t cur_dot_pos = -1) {
            if (cur_dot_pos == (uint64_t)-1)
                cur_dot_pos = temp_denormalize_struct();

            if (cur_dot_pos == 0)
                return;

            while (val.dot_pos || cur_dot_pos > 31) {
                val.unsigned_int /= 10;
                cur_dot_pos--;
            }
            std::string tmp_this_value = to_ansi_string();
            unreal64_t remove_nuls_mult(10);
            unreal64_t remove_nuls(1);
            size_t nul_count = 0;
            for (int64_t i = tmp_this_value.length() - 1; i >= 0; i--)
            {
                if (tmp_this_value[i] != '0')
                    break;
                else {
                    remove_nuls *= remove_nuls_mult;
                    nul_count++;
                }
            }
            uint64_t modify_dot = temp_denormalize_struct() - nul_count;
            if (nul_count)
                div(remove_nuls, false);
            normalize_struct(modify_dot);
        }
        uint64_t denormalize_dot() {
            if (val.unsigned_int) {
                uint64_t this_dot_pos = temp_denormalize_struct();
                uint64_t mult_int = 10;
                for (char i = 0; i < 24; i++) {
                    if ((val.unsigned_int * mult_int) / 10 == val.unsigned_int && this_dot_pos != 31) {
                        val.unsigned_int *= mult_int;
                        this_dot_pos++;
                        mult_int *= 10;
                    }
                    else
                        break;
                }
                return this_dot_pos;
            }
            return 0;
        }
        void div(const unreal64_t& rhs, bool do_normalize_dot = true) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();

            this_dot_pos -= rhs_dot_pos;
            while (true) {
                if (!(tmp.val.unsigned_int % 10) && tmp.val.unsigned_int) {
                    if (this_dot_pos != 31) {
                        this_dot_pos++;
                        tmp.val.unsigned_int /= 10;
                        continue;
                    }
                }
                this_dot_pos += denormalize_dot();
                val.unsigned_int /= tmp.val.unsigned_int;
                break;
            }
            normalize_dot(this_dot_pos);
        }
        void mod(const unreal64_t& rhs, bool do_normalize_dot = true) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();

            this_dot_pos -= rhs_dot_pos;


            unreal64_t shift_tmp;
            shift_tmp.val.dot_pos = -1;
            while (true) {
                if (val.unsigned_int % tmp.val.unsigned_int) {
                    this_dot_pos++;
                    if (this_dot_pos != shift_tmp.val.dot_pos) {
                        val.unsigned_int *= 10;
                        continue;
                    }
                    this_dot_pos--;
                }
                this_dot_pos += denormalize_dot();
                val.unsigned_int %= tmp.val.unsigned_int;
                break;
            }
            normalize_dot(this_dot_pos);
        }
    public:
        void mod_dot(uint64_t pos) {
            val.dot_pos = pos;
        }
        static unreal64_t get_min() {
            unreal64_t tmp = 0;
            tmp.val.unsigned_int = 0;
            tmp.val.dot_pos = 0;
            return tmp;
        }
        static unreal64_t get_max() {
            unreal64_t tmp = 0;
            tmp.val.unsigned_int = -1;
            tmp.val.dot_pos = 0;
            return tmp;
        }

        unreal64_t() {
            val.unsigned_int = 0;
        }
        unreal64_t(const unreal64_t& rhs) {
            val.unsigned_int = rhs.val.unsigned_int;
        }
        unreal64_t(unreal64_t&& rhs) noexcept {
            val.unsigned_int = rhs.val.unsigned_int;
        }
        unreal64_t(const char* str) {
            *this = std::string(str);
        }
        unreal64_t(const std::string str) {
            size_t found_pos = str.find('.');
            if (found_pos == std::string::npos)
            {
                val.unsigned_int = std::stoull(str.c_str());
                val.dot_pos = 0;
            }
            else {
                //check str for second dot
                if (str.find('.', found_pos) == std::string::npos)
                    throw std::invalid_argument("Real value can contain only one dot");

                std::string tmp = str;
                tmp.erase(tmp.begin() + found_pos);
                val.unsigned_int = std::stoull(tmp.c_str());
                val.dot_pos = (str.length() - found_pos - 1);
            }
            normalize_struct(temp_denormalize_struct());
        }

        //1 004 006 004 001
        template <typename T>
        unreal64_t(const T& rhs) {
            val.unsigned_int = (uint64_t)rhs;
        }
        template <typename T>
        unreal64_t(const T&& rhs) {
            val.unsigned_int = (uint64_t)rhs;
        }
        std::string to_ansi_string() {
            uint64_t tmp = temp_denormalize_struct();
            std::string str = std::to_string(val.unsigned_int);
            normalize_struct(tmp);
            if (val.dot_pos) {
                if (str.length() <= val.dot_pos) {
                    uint64_t resize_result_len = val.dot_pos - str.length() + 1;
                    std::string to_add_zeros;
                    while (resize_result_len--)
                        to_add_zeros += '0';
                    str = to_add_zeros + str;
                }
                str.insert(str.end() - tmp, '.');
            }
            return str;
        }
        std::string to_ansi_string() const {
            return unreal64_t(*this).to_ansi_string();
        }
        std::string to_hex_str() const {
            static const char* digits = "0123456789ABCDEF";
            std::string rc(8, '0');
            for (size_t i = 0, j = 28; i < 8; ++i, j -= 4)
                rc[i] = digits[(val.unsigned_int >> j) & 0x0f];
            return rc;
        }

        unreal64_t& operator=(const unreal64_t& rhs) = default;
        unreal64_t& operator=(unreal64_t&& rhs) = default;

        unreal64_t operator&(const unreal64_t& rhs) const {
            return unreal64_t(*this) &= rhs;
        }

        unreal64_t& operator&=(const unreal64_t& rhs) {
            val.unsigned_int &= rhs.val.unsigned_int;
            return *this;
        }

        unreal64_t operator|(const unreal64_t& rhs) const {
            return unreal64_t(*this) |= rhs;
        }

        unreal64_t& operator|=(const unreal64_t& rhs) {
            val.unsigned_int |= rhs.val.unsigned_int;
            return *this;
        }

        unreal64_t operator^(const unreal64_t& rhs) const {
            return unreal64_t(*this) ^= rhs;
        }

        unreal64_t& operator^=(const unreal64_t& rhs) {
            val.unsigned_int ^= rhs.val.unsigned_int;
            return *this;
        }

        unreal64_t operator~() const {
            unreal64_t tmp(*this);
            tmp.val.unsigned_int = ~tmp.val.unsigned_int;
            return tmp;
        }



        unreal64_t& operator<<=(uint64_t shift) {
            val.unsigned_int <<= shift;
            return *this;
        }
        unreal64_t operator<<(uint64_t shift) const {
            return unreal64_t(*this) <<= shift;
        }


        unreal64_t& operator>>=(uint64_t shift) {
            val.unsigned_int >>= shift;
            return *this;
        }
        unreal64_t operator>>(uint64_t shift) const {
            return unreal64_t(*this) >>= shift;
        }
        bool operator!() const {
            return !val.unsigned_int;
        }

        bool operator&&(const unreal64_t& rhs) const {
            return ((bool)*this && rhs);
        }

        bool operator||(const unreal64_t& rhs) const {
            return ((bool)*this || rhs);
        }

        bool operator==(const unreal64_t& rhs) const {
            return val.unsigned_int == rhs.val.unsigned_int;
        }

        bool operator!=(const unreal64_t& rhs) const {
            return val.unsigned_int != rhs.val.unsigned_int;
        }

        bool operator>(const unreal64_t& rhs) const {
            std::vector<uint64_t> this_parts = split_dot(to_ansi_string());
            std::vector<uint64_t> rhs_parts = split_dot(rhs.to_ansi_string());
            {
                if (this_parts[0] == rhs_parts[0]);
                else return this_parts[0] > rhs_parts[0];
            }
            if (this_parts.size() == 2 && rhs_parts.size() == 2) {
                return this_parts[1] > rhs_parts[1];
            }
            else
                return this_parts.size() > rhs_parts.size();
        }

        bool operator<(const unreal64_t& rhs) const {
            return !(*this > rhs) && *this != rhs;
        }

        bool operator>=(const unreal64_t& rhs) const {
            return ((*this > rhs) || (*this == rhs));
        }

        bool operator<=(const unreal64_t& rhs) const {
            return !(*this > rhs);
        }


        unreal64_t& operator++() {
            return *this += 1;
        }
        unreal64_t operator++(int) {
            unreal64_t temp(*this);
            *this += 1;
            return temp;
        }
        unreal64_t operator+(const unreal64_t& rhs) const {
            return unreal64_t(*this) += rhs;
        }
        unreal64_t operator-(const unreal64_t& rhs) const {
            return unreal64_t(*this) -= rhs;
        }

        unreal64_t& operator+=(const unreal64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                uint64_t move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.unsigned_int *= move_tmp;
            }
            else {
                uint64_t move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.unsigned_int *= move_tmp;
            }
            val.unsigned_int += tmp.val.unsigned_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }
        unreal64_t& operator-=(const unreal64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            if (this_dot_pos == rhs_dot_pos);
            else if (this_dot_pos > rhs_dot_pos) {
                uint64_t move_tmp = 1;
                while (this_dot_pos != rhs_dot_pos++)
                    move_tmp *= 10;
                tmp.val.unsigned_int *= move_tmp;
            }
            else {
                uint64_t move_tmp = 1;
                while (rhs_dot_pos != this_dot_pos++)
                    move_tmp *= 10;
                val.unsigned_int *= move_tmp;
            }
            val.unsigned_int -= tmp.val.unsigned_int;
            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        unreal64_t operator*(const unreal64_t& rhs) const {
            return unreal64_t(*this) *= rhs;
        }


        unreal64_t& operator*=(const unreal64_t& rhs) {
            uint64_t this_dot_pos = temp_denormalize_struct();
            unreal64_t tmp = rhs;
            uint64_t rhs_dot_pos = tmp.temp_denormalize_struct();
            this_dot_pos += rhs_dot_pos;

            val.unsigned_int *= tmp.val.unsigned_int;

            normalize_struct(this_dot_pos);
            normalize_dot();
            return *this;
        }

        unreal64_t operator/(const unreal64_t& rhs) const {
            return unreal64_t(*this) /= rhs;
        }

        unreal64_t& operator/=(const unreal64_t& rhs) {
            div(rhs);
            return *this;
        }

        unreal64_t operator%(const unreal64_t& rhs) const {
            return unreal64_t(*this) %= rhs;
        }

        unreal64_t& operator%=(const unreal64_t& rhs) {
            mod(rhs);
            return *this;
        }

        explicit operator bool() const {
            return (bool)val.unsigned_int;
        }
        explicit operator uint8_t() const {
            return (uint64_t)val.unsigned_int;
        }
        explicit operator uint16_t() const {
            return (uint64_t)val.unsigned_int;
        }
        explicit operator uint32_t() const {
            return (uint64_t)val.unsigned_int;
        }

        explicit operator int64_t() const {
            return (uint64_t)*this;
        }
        explicit operator uint64_t() const {
            unreal64_t tmp = *this;
            uint64_t div = 10;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; temp++)
                tmp.val.unsigned_int /= div;
            return tmp.val.unsigned_int;
        }

        explicit operator double() const {
            unreal64_t tmp = *this;
            uint64_t temp = tmp.temp_denormalize_struct();
            for (uint64_t i = 0; i < temp; temp++)
                tmp.val.unsigned_int /= 10;

            double res = (double)(uint64_t)tmp.val.unsigned_int;
            for (uint64_t i = 0; i < temp && i < 15; temp++)
                tmp.val.unsigned_int *= 10;
            tmp = *this - tmp.val.unsigned_int;

            double res_part = (double)(uint64_t)tmp.val.unsigned_int;;
            for (uint64_t i = 0; i < temp && i<15; temp++)
                res_part /= 10;
            return res + res_part;
        }
    };

    typedef real_t<0> real128_t;
    typedef real_t<1> real256_t;
    typedef real_t<2> real512_t;
    typedef real_t<3> real1024_t;

    typedef unreal_t<0> unreal128_t;
    typedef unreal_t<1> unreal256_t;
    typedef unreal_t<2> unreal512_t;
    typedef unreal_t<3> unreal1024_t;
}