#include <cstdint>
#include <map>
#include <experimental/optional>

#pragma once

using IPAddress = uint32_t;
using Range = std::pair<IPAddress, IPAddress>;

template<typename Tp>
using Optional = std::experimental::optional<Tp>;

class Pool
{
    std::map<IPAddress, bool /*End-of-Range flag*/> ranges;

    static IPAddress str2ip(const std::string &ip);

public:
    /// Returns number of ranges
    std::size_t size() const;
    void clear();

    static Range makeRange(IPAddress ip1, IPAddress ip2);
    static Range makeRange(const std::string &ip1, const std::string &ip2);

    class Iterator
    {
        using iterator = decltype(ranges)::const_iterator;
        iterator iter;
        bool end;

    public:
        explicit Iterator(iterator i, bool end = false) : iter(i), end(end) {}

        Range operator*() const noexcept;
        bool operator==(const Iterator &o) const;
        bool operator!=(const Iterator &o) const;
        Iterator &operator++();
        Iterator &operator--();
        Iterator operator++(int);
        Iterator operator--(int);
    };

    using iterator = Iterator;

    iterator begin() const { return Iterator(ranges.begin()); }
    iterator end() const { return Iterator(ranges.end(), true); }

    /// Append range to pool
    void addRange(Range range);

    /// Find range for given @a ip address
    Optional<Range> findRange(IPAddress ip) const;

    /// Find range for given @a ip address
    Optional<Range> findRange(const std::string &ip) const;
};

Pool find_diff(const Pool& old_pool, const Pool& new_pool);
