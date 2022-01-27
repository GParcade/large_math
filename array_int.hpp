#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

namespace large_math{
    template<uint64_t byte_expand>
    class array_uint {
    public:
        static const size_t expand_bits = []() { size_t res = 1; for (size_t i = 0; i <= byte_expand; i++) res <<= 2; return res << 6; }();
        static const size_t expand_bytes = []() { size_t res = 1; for (size_t i = 0; i <= byte_expand; i++) res <<= 2; return res; }();
        uint64_t integers[expand_bytes];
    //private:
        uint64_t bits() const {
            uint64_t out = 0;
            int64_t max_elem = expand_bytes - 1;
            for (int64_t i = max_elem; i >= 0; --i) 
                if (integers[i])
                    max_elem = i;
            uint64_t tmp = integers[max_elem];
            out = (expand_bytes - 1 -max_elem) * 64;
            while (tmp) {
                tmp >>= 1;
                out++;
            }
            return out;
        }
        std::pair <array_uint, array_uint> divmod(const array_uint& lhs, const array_uint& rhs) const {
            if (rhs == 0) {
                throw std::domain_error("Error: division or modulus by 0");
            }
            else if (rhs == 1) {
                return std::pair <array_uint, array_uint>(lhs, 1);
            }
            else if (lhs == rhs) {
                return std::pair <array_uint, array_uint>(1, 0);
            }
            else if ((lhs == 0) || (lhs < rhs)) {
                return std::pair <array_uint, array_uint>(0, lhs);
            }

            std::pair <array_uint, array_uint> qr(0, lhs);
            array_uint& copyd = *new array_uint(rhs << (lhs.bits() - rhs.bits()));
            array_uint& adder = *new array_uint(array_uint(1) << (lhs.bits() - rhs.bits()));
            if (copyd > qr.second) {
                copyd >>= 1;
                adder >>= 1;
            }

            while (qr.second >= rhs) {
                if (qr.second >= copyd) {
                    qr.second -= copyd;
                    qr.first |= adder;
                }
                copyd >>= 1;
                adder >>= 1;
            }
            delete &copyd;
            delete &adder;
            return qr;
        }
        void zero_all() {
            for (int64_t i = expand_bytes - 1; i >= 0; --i)
                integers[i] = 0;
        }

    public:  
        static array_uint get_max() {
            array_uint tmp;
            for (int64_t i = expand_bytes - 1; i >= 0; --i)
                tmp.integers[i] = -1;
            return tmp;
        }
        

        array_uint() {}
        array_uint(const array_uint& rhs) {
            for (int64_t i = expand_bytes - 1; i >= 0; --i)
                integers[i] = rhs.integers[i];
        }
        array_uint(array_uint&& rhs) noexcept {
            for (int64_t i = expand_bytes - 1; i >= 0; --i)
                integers[i] = rhs.integers[i];
        }
        template <uint64_t byt_expand>
        array_uint(const array_uint<byt_expand>& rhs) {
            if constexpr (expand_bits <= array_uint<byt_expand>::expand_bits) {
                for (int64_t i = expand_bytes - 1; i >= 0; --i)
                    integers[i] = rhs.integers[i];
            }
            else {
                zero_all();
                for (int64_t i = array_uint<byt_expand>::expand_bytes - 1; i >= 0; --i)
                    integers[i] = rhs.integers[i];
            }
        }
        array_uint& operator=(const array_uint& rhs) = default;
        array_uint& operator=(array_uint&& rhs) = default;
        array_uint(const long long rhs) { zero_all(); integers[expand_bytes - 1] = rhs; }
        array_uint(const unsigned long long rhs) { zero_all(); integers[expand_bytes - 1] = rhs; }
        array_uint(const unsigned int rhs) { zero_all(); integers[expand_bytes - 1] = rhs; }
        array_uint(const double rhs) { zero_all(); integers[expand_bytes - 1] = rhs; }
        array_uint(const int rhs) { zero_all(); integers[expand_bytes - 1] = rhs; }
        array_uint(const std::string str) {
            *this = str.c_str();
        }
        array_uint(const char* str) {
            zero_all();
            array_uint* mult = new array_uint(10);
            size_t str_len = strlen(str);
            for (size_t i = 0; i < str_len; i++) {
                *this *= *mult;
                *this += str[i] - '0';
            }
            delete mult;
        }
        std::string to_ansi_string() const {
            std::string res;
            {
                std::pair <array_uint, array_uint>& tmp = *new std::pair <array_uint, array_uint>(*this, 0);
                array_uint& dever = *new array_uint(10000000000000000000);
                std::string len_tmp;
                while (true) {
                    tmp = divmod(tmp.first, dever);
                    len_tmp = std::to_string(tmp.second.integers[expand_bytes - 1]);
                    for (size_t i = len_tmp.size(); i < 19;i++) 
                        len_tmp = '0' + len_tmp;
                    res = len_tmp+res;
                    if (tmp.first == 0)
                        break;
                }
                delete& dever;
                delete& tmp;
            }
            while (res[0] == '0') {
                res.erase(res.begin());
                if (!res.size()) {
                    res = '0';
                    break;
                }
            }
            return res;
        }
        std::string to_hex_str() const {
            static const char* digits = "0123456789ABCDEF";
            std::string res;
            std::string rc(8, '0');
            for (auto i : integers){
                for (size_t l = 0, j = 28; l < 8; ++l, j -= 4)
                    rc[l] = digits[(i >> j) & 0x0f];
                res += rc;
            }

            while (res[0] == '0')
                res.erase(res.begin());

            if (res.empty())
                res = '0';
            return "0x" + res;
        }




        static array_uint pow(const array_uint& A, const array_uint& N) {
            array_uint res = 1;
            array_uint* a = new array_uint(A);
            array_uint* n = new array_uint(N);
            size_t tot_bits = n->bits();
            uint64_t* proxy_n_end = &n->integers[expand_bytes - 1];
            for (size_t i = 0; i < tot_bits; i++) {
                if (*proxy_n_end & 1)
                    res *= *a;
                *a *= *a;
                *n >>= 1;
            }
            delete a;
            delete n;
            return res;
        }
        array_uint& pow(const array_uint& n) {
            return *this = pow(*this,n);
        }
        array_uint& sqrt() {
            array_uint* a = new array_uint(1);
            array_uint* b = new array_uint(*this);

            while (*b - *a > 1) {
                *b = *this / *a;
                *a = (*a + *b) >> 1;
            }
            *this = *a;
            delete a;
            delete b;
            return *this;
        }

        array_uint operator&(const array_uint& rhs) const {
            return array_uint(*this) &= rhs;
        }
        array_uint& operator&=(const array_uint& rhs) {
            for (size_t i = 0; i < expand_bytes; i++)
                integers[i] &= rhs.integers[i];
            return *this;
        }
        array_uint operator|(const array_uint& rhs) const {
            return array_uint(*this) |= rhs;
        }
        array_uint& operator|=(const array_uint& rhs) {
            for (size_t i = 0; i < expand_bytes; i++)
                integers[i] |= rhs.integers[i];
            return *this;
        }
        array_uint operator^(const array_uint& rhs) const {
            return array_uint(*this) ^= rhs;
        }
        array_uint& operator^=(const array_uint& rhs) {
            for (size_t i = 0; i < expand_bytes; i++)
                integers[i] ^= rhs.integers[i];
            return *this;
        }
        array_uint operator~() const {
            array_uint res(*this);
            for (size_t i = 0; i < expand_bytes; i++)
                res.integers[i] = ~integers[i];
            return res;
        }



        array_uint& operator<<=(uint64_t shift) {
            uint64_t bits1 = 0, bits2 = 0;
            uint64_t block_shift = shift / 64;
            uint64_t sub_block_shift = shift % 64;
            for (size_t block_shift_c = 0; block_shift_c < block_shift; block_shift_c++)
                for (int64_t i = expand_bytes - 1; i >= 0; --i) {
                    bits1 = integers[i];
                    integers[i] = bits2;
                    bits2 = bits1;
                }
            if (sub_block_shift) {
                bits1 = bits2 = 0;
                uint64_t anti_shift = 64 - sub_block_shift;
                uint64_t and_op = (uint64_t)-1 << anti_shift;
                for (int64_t i = expand_bytes - 1; i >= 0; --i) {
                    bits2 = integers[i] & and_op;
                    integers[i] <<= sub_block_shift;
                    integers[i] |= bits1 >> anti_shift;
                    bits1 = bits2;
                }
            }
            return *this;
        }
        array_uint& operator>>=(uint64_t shift) {
            uint64_t bits1 = 0, bits2 = 0;
            uint64_t block_shift = shift / 64;
            uint64_t sub_block_shift = shift % 64;
            for (size_t block_shift_c = 0; block_shift_c < block_shift; block_shift_c++)
                for (size_t i = 0; i < expand_bytes; i++) {
                    bits1 = integers[i];
                    integers[i] = bits2;
                    bits2 = bits1;
                }
            if (sub_block_shift) {
                bits1 = bits2 = 0;
                uint64_t anti_shift = 64 - sub_block_shift;
                uint64_t and_op = -1 >> anti_shift;
                for (size_t i = 0; i < expand_bytes; i++) {
                    bits2 = integers[i] & and_op;
                    integers[i] >>= sub_block_shift;
                    integers[i] |= bits1 << anti_shift;
                    bits1 = bits2;
                }
            }
            return *this;
        }
        array_uint operator<<(uint64_t shift) const {
            return array_uint(*this) <<= shift;
        }
        array_uint operator>>(uint64_t shift) const {
            return array_uint(*this) >>= shift;
        }


        bool operator!() const {
            return !(bool)(*this);
        }
        bool operator&&(const array_uint& rhs) const {
            return ((bool)*this && rhs);
        }
        bool operator||(const array_uint& rhs) const {
            return ((bool)*this || rhs);
        }
        bool operator==(const array_uint& rhs) const {
            for (size_t i = 0; i < expand_bytes; i++)
                if (integers[i] != rhs.integers[i])
                    return false;
            return true;
        }
        bool operator!=(const array_uint& rhs) const {
            return !(*this == rhs);
        }
        bool operator>(const array_uint& rhs) const {
            for (size_t i = 0; i < expand_bytes; i++) {
                if (integers[i] > rhs.integers[i])
                    return true;
                else if (integers[i] < rhs.integers[i])
                    return false;
            }
            return false;
        }
        bool operator<(const array_uint& rhs) const {
            for (size_t i = 0; i < expand_bytes; i++) {
                if (integers[i] < rhs.integers[i])
                    return true;
                else if (integers[i] > rhs.integers[i])
                    return false;
            }
            return false;
        }
        bool operator>=(const array_uint& rhs) const {
            return !(*this < rhs);
        }
        bool operator<=(const array_uint& rhs) const {
            return !(*this > rhs);
        }


        array_uint& operator++() {
            return *this += 1;
        }
        array_uint operator++(int) {
            array_uint temp(*this);
            *this += 1;
            return temp;
        }
        array_uint& operator--() {
            return *this -= 1;
        }
        array_uint operator--(int) {
            array_uint temp(*this);
            --* this;
            return temp;
        }

        array_uint operator+(const array_uint& rhs) const {
            return array_uint(*this) += rhs;
        }
        array_uint operator-(const array_uint& rhs) const {
            return array_uint(*this) -= rhs;
        }
        array_uint& operator+=(const array_uint& rhs) {
            uint64_t bits1 = 0, bits2 = 0;
            for (int64_t i = expand_bytes - 1; i >= 0; --i) {
                bits1 = ((integers[i] + rhs.integers[i] + bits2) < integers[i]);
                integers[i] += rhs.integers[i] + bits2;
                bits2 = bits1;
            }
            return *this;
        }
        array_uint& operator-=(const array_uint& rhs) {
            uint64_t bits1 = 0, bits2 = 0;
            for (int64_t i = expand_bytes - 1; i >= 0; --i) {
                bits1 = ((integers[i] - rhs.integers[i] - bits2) > integers[i]);
                integers[i] -= rhs.integers[i] + bits2;
                bits2 = bits1;
            }
            return *this;
        }


        array_uint operator*(const array_uint& rhs) const {
            array_uint res(0);
            array_uint& multer = *new array_uint(rhs);
            size_t tot_bits = multer.bits();
            uint64_t& proxy_multer_end = multer.integers[expand_bytes - 1];
            for (size_t i = 0; i < tot_bits; i++) {
                if (proxy_multer_end & 1) {
                    res += *this << i;
                }
                multer >>= 1;
            }
            delete& multer;
            return res;
        }
        array_uint& operator*=(const array_uint& rhs) {
            *this = *this * rhs;
            return *this;
        }

        array_uint operator/(const array_uint& rhs) const {
            return divmod(*this, rhs).first;
        }
        array_uint& operator/=(const array_uint& rhs) {
            return *this = divmod(*this, rhs).first;
        }

        array_uint operator%(const array_uint& rhs) const {
            return divmod(*this, rhs).second;
        }
        array_uint& operator%=(const array_uint& rhs) {
            *this = divmod(*this, rhs).second;
            return *this;
        }
        explicit operator bool() const {
            uint64_t res = 0;
            for (size_t i = 0; i < expand_bytes; i++)
                res |= integers[i];
            return (bool)res;
        }
        explicit operator uint8_t() const {
            return (uint8_t)integers[expand_bytes - 1];
        }
        explicit operator uint16_t() const {
            return (uint16_t)integers[expand_bytes - 1];
        }
        explicit operator uint32_t() const {
            return (uint32_t)integers[expand_bytes - 1];
        }
        explicit operator uint64_t() const {
            return (uint64_t)integers[expand_bytes - 1];
        }
        template <uint64_t byt_expand>
        explicit operator array_uint<byt_expand>() const {
            return array_uint<byt_expand>(*this);
        }
    };

    template<uint64_t byte_expand>
    class array_int {
        array_int& switch_my_siqn() {
            val.unsigned_int = ~val.unsigned_int;
            val.unsigned_int += 1;
            return *this;
        }
        array_int switch_my_siqn() const {
            return array_int(*this).switch_my_siqn();
        }

        array_int& switch_to_unsiqn() {
            if (val.is_minus)
                switch_my_siqn();
            return *this;
        }
        array_int switch_to_unsiqn() const {
            if (val.is_minus)
                return array_int(*this).switch_my_siqn();
            else return *this;
        }

        array_int& switch_to_siqn() {
            if (!val.is_minus)
                switch_my_siqn();
            return *this;
        }
        array_int switch_to_siqn() const {
            if (!val.is_minus)
                return array_int(*this).switch_my_siqn();
            else return *this;
        }
    public:
        union for_constructor {
            struct s {
                bool _is_minus : 1;
            public:
                s() {}
                s(bool val) {
                    _is_minus = val;
                }
                void operator = (bool val) {
                    _is_minus = val;
                }
                operator bool() const {
                    return _is_minus;
                }
            } is_minus;

            array_uint<byte_expand> unsigned_int;
            for_constructor() {
            }
        } val;
        static array_int get_min() {
            array_int tmp = 0;
            tmp.val.is_minus = 1;
            return tmp;
        }
        static array_int get_max() {
            array_int tmp;
            tmp.val.unsigned_int = array_uint<byte_expand>::get_max();
            tmp.val.is_minus = 0;
            return tmp;
        }


        array_int() {
            val.unsigned_int = 0;
        }
        array_int(const array_int& rhs) {
            val.unsigned_int = rhs.val.unsigned_int;
        }
        array_int(array_int&& rhs) {
            val.unsigned_int = rhs.val.unsigned_int;
        } 
        array_int(const std::string str) {
            *this = str.c_str();
        }
        array_int(const char* str) {
            bool set_minus = 0;
            if (*str++ == '-')
                set_minus = 1;
            else str--;
            val.unsigned_int = array_uint<byte_expand>(str);
            if (set_minus) {
                switch_to_siqn();
                val.is_minus = 1;
            }
        }
        template <uint64_t byt_expand>
        array_int(const array_int<byt_expand> rhs) {
            array_int<byt_expand> tmp = rhs;
            bool is_minus = tmp.val.is_minus;
            tmp.val.is_minus = 0;
            val.unsigned_int = tmp.val.unsigned_int;
            val.is_minus = is_minus;
        }
        template <typename T>
        array_int(const T& rhs)
        {
            val.unsigned_int = (array_uint<byte_expand>)rhs;
        }
        template <typename T>
        array_int(const T&& rhs)
        {
            val.unsigned_int = (array_uint<byte_expand>)rhs;
        }
        array_int(const long long rhs) { *this = std::to_string(rhs).c_str(); }
        array_int(const int rhs) { *this = std::to_string(rhs).c_str(); }

        std::string to_ansi_string() const {
            if (val.is_minus)
                return
                '-' + array_int(*this).switch_to_unsiqn().
                val.unsigned_int.to_ansi_string();
            else
                return val.unsigned_int.to_ansi_string();
        }
        std::string to_hex_str() const {
            return val.unsigned_int.to_hex_str();
        }

        array_int& operator=(const array_int& rhs) = default;
        array_int& operator=(array_int&& rhs) = default;


        array_int operator&(const array_int& rhs) const {
            return array_int(*this) &= rhs;
        }
        array_int& operator&=(const array_int& rhs) {
            val.unsigned_int &= rhs.val.unsigned_int;
            return *this;
        }
        array_int operator|(const array_int& rhs) const {
            return array_int(*this) |= rhs;
        }
        array_int& operator|=(const array_int& rhs) {
            val.unsigned_int |= rhs.val.unsigned_int;
            return *this;
        }
        array_int operator^(const array_int& rhs) const {
            return array_int(*this) ^= rhs;
        }
        array_int& operator^=(const array_int& rhs) {
            val.unsigned_int ^= rhs.val.unsigned_int;
            return *this;
        }
        array_int operator~() const {
            array_int tmp(*this);
            ~tmp.val.unsigned_int;
            return tmp;
        }


        array_int& operator<<=(uint64_t shift) {
            val.unsigned_int <<= shift;
            return *this;
        }
        array_int operator<<(uint64_t shift) const {
            return array_int(*this) <<= shift;
        }
        array_int& operator>>=(uint64_t shift) {
            val.unsigned_int >>= shift;
            return *this;
        }
        array_int operator>>(uint64_t shift) const {
            return array_int(*this) >>= shift;
        }

        bool operator!() const {
            return !val.unsigned_int;
        }
        bool operator&&(const array_int& rhs) const {
            return ((bool)*this && rhs);
        }
        bool operator||(const array_int& rhs) const {
            return ((bool)*this || rhs);
        }
        bool operator==(const array_int& rhs) const {
            return val.unsigned_int == rhs.val.unsigned_int;
        }
        bool operator!=(const array_int& rhs) const {
            return val.unsigned_int != rhs.val.unsigned_int;
        }
        bool operator>(const array_int& rhs) const {
            if (val.is_minus && !rhs.val.is_minus)
                return false;
            if (!val.is_minus && rhs.val.is_minus)
                return true;
            return array_int(*this).switch_to_unsiqn().val.unsigned_int > rhs.switch_to_unsiqn().val.unsigned_int;
        }
        bool operator<(const array_int& rhs) const {
            if (val.is_minus && !rhs.val.is_minus)
                return true;
            if (!val.is_minus && rhs.val.is_minus)
                return false;
            return array_int(*this).switch_to_unsiqn().val.unsigned_int < rhs.switch_to_unsiqn().val.unsigned_int;
        }
        bool operator>=(const array_int& rhs) const {
            return ((*this > rhs) | (*this == rhs));
        }
        bool operator<=(const array_int& rhs) const {
            return ((*this < rhs) | (*this == rhs));
        }

        array_int& operator++() {
            if (val.is_minus) {
                switch_my_siqn().val.unsigned_int += 1;
                switch_my_siqn();
            }
            else
                val.unsigned_int++;
            return *this;
        }
        array_int operator++(int) {
            array_int temp(*this);
            ++* this;
            return temp;
        }
        array_int& operator--() {
            if (val.is_minus) {
                switch_my_siqn().val.unsigned_int -= 1;
                switch_my_siqn();
            }
            else
                val.unsigned_int--;
            return *this;
        }
        array_int operator--(int) {
            array_int temp(*this);
            --* this;
            return temp;
        }

        array_int operator+(const array_int& rhs) const {
            return array_int(*this) += rhs;
        }
        array_int operator-(const array_int& rhs) const {
            return array_int(*this) -= rhs;
        }

        array_int& operator+=(const array_int& rhs) {
            if (val.is_minus && rhs.val.is_minus) {
                switch_my_siqn().val.unsigned_int += rhs.switch_my_siqn().val.unsigned_int;
                switch_to_siqn();
            }
            else if (val.is_minus) {
                if (switch_my_siqn().val.unsigned_int < rhs.val.unsigned_int) {
                    val.unsigned_int = rhs.val.unsigned_int - val.unsigned_int;
                    switch_to_unsiqn();
                }
                else {
                    val.unsigned_int -= rhs.val.unsigned_int;
                    switch_to_siqn();
                }
            }
            else if (rhs.val.is_minus) {
                array_int* temp = new array_int(rhs);
                temp->switch_my_siqn();
                if (val.unsigned_int < temp->val.unsigned_int) {
                    val.unsigned_int = temp->val.unsigned_int - val.unsigned_int;
                    switch_to_siqn();
                }
                else
                    val.unsigned_int -= temp->val.unsigned_int;
                delete temp;
            }
            else
                val.unsigned_int += rhs.val.unsigned_int;
            return *this;
        }
        array_int& operator-=(const array_int& rhs) {
            if (val.is_minus && rhs.val.is_minus) {
                if (*this > rhs) {
                    switch_my_siqn().val.unsigned_int -= rhs.switch_my_siqn().val.unsigned_int;
                    switch_my_siqn();
                }
                else {
                    switch_my_siqn();
                    val.unsigned_int = rhs.switch_my_siqn().val.unsigned_int - val.unsigned_int;
                }
            }
            else if (val.is_minus) {
                switch_my_siqn();
                if (this->val.unsigned_int > rhs.val.unsigned_int) {
                    val.unsigned_int += rhs.val.unsigned_int;
                    switch_to_siqn();
                }
                else 
                    val.unsigned_int -= rhs.val.unsigned_int;
            }
            else if (rhs.val.is_minus) {
                array_int* temp = new array_int(rhs);
                temp->switch_my_siqn();
                val.unsigned_int += temp->val.unsigned_int;
                delete temp;
            }
            else {
                if (val.unsigned_int >= rhs.val.unsigned_int)
                    val.unsigned_int -= rhs.val.unsigned_int;
                else {
                    val.unsigned_int = rhs.val.unsigned_int - val.unsigned_int;
                    switch_my_siqn();
                }
            }
            return *this;
        }

        array_int operator*(const array_int& rhs) const {
            return array_int(*this) *= rhs;
        }
        array_int& operator*=(const array_int& rhs) {
            if (val.is_minus && rhs.val.is_minus)
                switch_my_siqn().val.unsigned_int *= rhs.switch_my_siqn().val.unsigned_int;
            else if (val.is_minus || rhs.val.is_minus) {
                switch_to_unsiqn().val.unsigned_int *= rhs.switch_to_unsiqn().val.unsigned_int;
                switch_to_siqn();
            }
            else
                val.unsigned_int *= rhs.val.unsigned_int;
            return *this;
        }


        array_int operator/(const array_int& rhs) const {
            return array_int(*this) /= rhs;
        }
        array_int& operator/=(const array_int& rhs) {
            if (val.is_minus && rhs.val.is_minus)
                switch_my_siqn().val.unsigned_int /= rhs.switch_my_siqn().val.unsigned_int;
            else if (val.is_minus || rhs.val.is_minus) {
                switch_to_unsiqn().val.unsigned_int /= rhs.switch_to_unsiqn().val.unsigned_int;
                switch_my_siqn();
            }
            else
                val.unsigned_int /= rhs.val.unsigned_int;
            return *this;
        }

        array_int operator%(const array_int& rhs) const {
            return array_int(*this) %= rhs;
        }
        array_int& operator%=(const array_int& rhs) {
            if (val.is_minus && rhs.val.is_minus)
                switch_my_siqn().val.unsigned_int %= rhs.switch_my_siqn().val.unsigned_int;
            else if (val.is_minus || rhs.val.is_minus) {
                switch_to_unsiqn().val.unsigned_int %= rhs.switch_to_unsiqn().val.unsigned_int;
                switch_my_siqn();
            }
            else
                val.unsigned_int %= rhs.val.unsigned_int;
            return *this;
        }


        array_int operator+() const {
            return switch_to_unsiqn();
        }
        array_int operator-() const {
            return switch_to_siqn();
        }


        explicit operator bool() const {
            return (bool)val.unsigned_int;
        }
        explicit operator uint8_t() const {
            return (uint8_t)val.unsigned_int;
        }
        explicit operator uint16_t() const {
            return (uint16_t)val.unsigned_int;
        }
        explicit operator uint32_t() const {
            return (uint32_t)val.unsigned_int;
        }
        explicit operator uint64_t() const {
            return (uint64_t)val.unsigned_int;
        }
        template <uint64_t byt_expand>
        explicit operator array_uint<byt_expand>() const {
            return array_uint<byt_expand>(val.unsigned_int);
        }
        template <uint64_t byt_expand>
        explicit operator array_int<byt_expand>() {
            return to_ansi_string().c_str();
        }
        explicit operator int64_t() const {
            return std::stoll(to_ansi_string());;
        }
    };


    typedef array_uint<0> auint128_t;
    typedef array_uint<1> auint256_t;
    typedef array_uint<2> auint512_t;
    typedef array_uint<3> auint1024_t;

    typedef array_int<0> aint128_t;
    typedef array_int<1> aint256_t;
    typedef array_int<2> aint512_t;
    typedef array_int<3> aint1024_t;
}