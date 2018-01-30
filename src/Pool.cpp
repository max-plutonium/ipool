#include <cassert>
#include <vector>
#include "Pool.h"

//static
IPAddress Pool::str2ip(const std::string &ip)
{
    uint32_t a, b, c, d;
    IPAddress addr = 0;

    if (sscanf(ip.c_str(), "%i.%i.%i.%i", &a, &b, &c, &d) != 4)
        return 0;

    addr = a << 24;
    addr |= b << 16;
    addr |= c << 8;
    addr |= d;
    return addr;
}

std::size_t Pool::size() const {
    return ranges.size() / 2;
}

void Pool::clear() {
    ranges.clear();
}

//static
Range Pool::makeRange(IPAddress ip1, IPAddress ip2)
{
    return std::make_pair(std::min(ip1, ip2), std::max(ip1, ip2));
}

//static
Range Pool::makeRange(const std::string &ip1, const std::string &ip2)
{
    const auto ip1value = str2ip(ip1);
    const auto ip2value = str2ip(ip2);
    return std::make_pair(std::min(ip1value, ip2value), std::max(ip1value, ip2value));
}

Range Pool::Iterator::operator*() const noexcept {
    IPAddress lower = 0, upper = 0;

    if(!end) {
        auto i = iter;
        lower = (i++)->first;
        upper = i->first;
    }

    return Pool::makeRange(lower, upper);
}

bool Pool::Iterator::operator==(const Iterator &o) const {
    return *iter == *o.iter;
}

bool Pool::Iterator::operator!=(const Iterator &o) const {
    return *iter != *o.iter;
}

Pool::Iterator &Pool::Iterator::operator++()
{
    ++iter; ++iter;
    return *this;
}

Pool::Iterator &Pool::Iterator::operator--()
{
    --iter; --iter;
    return *this;
}

Pool::Iterator Pool::Iterator::operator++(int)
{
    Iterator tmp = *this;
    ++iter; ++iter;
    return tmp;
}

Pool::Iterator Pool::Iterator::operator--(int)
{
    Iterator tmp = *this;
    --iter; --iter;
    return tmp;
}

void Pool::addRange(Range range)
{
    const auto size = ranges.size();

    if(!size) {
        ranges.emplace(range.first, false);
        ranges.emplace(range.second, true);
    }
    else
    {
        auto lowerIt = ranges.upper_bound(range.first);
        auto upperIt =  ranges.upper_bound(range.second);

        for(auto it = lowerIt; it != upperIt; ++it)
            if(it->first >= range.first && it->first < range.second)
                ranges.erase(it);

        if(ranges.end() == lowerIt || !lowerIt->second)
            ranges.emplace(range.first, false);
        if(ranges.end() == upperIt || !upperIt->second)
            ranges.emplace(range.second, true);

        assert(0 == size % 2);
    }
}

Optional<Range> Pool::findRange(IPAddress ip) const
{
    const auto size = ranges.size();
    assert(0 == size % 2);

    if(!size)
        return {};
    else {
        auto it = ranges.upper_bound(ip);
        if(ranges.end() == it)
            return {};

        IPAddress upperEntry, lowerEntry;

        if (it->second) {
            upperEntry = it->first;
            lowerEntry = (--it)->first;
        } else {
            lowerEntry = it->first;
            upperEntry = (++it)->first;
        }

        auto range = Pool::makeRange(lowerEntry, upperEntry);
        if (ip >= range.first && ip <= range.second)
            return {range};
    }

    return {};
}

Optional<Range> Pool::findRange(const std::string &ip) const {
    return findRange(str2ip(ip));
}

// Функция, вычисляющая разницу между старым и новым пулами:
// `old_pool` - старый пул IP-адресов
// `new_pool` - новый пул IP-адресов
// return value - пул диапазонов "устаревших" IP-адресов
Pool find_diff(const Pool& old_pool, const Pool& new_pool)
{
    Pool result;

    std::vector<Range> diffRanges;

    for(auto it = old_pool.begin(), newPoolIt = new_pool.begin(); it != old_pool.end(); )
    {
        auto range = *it;
        Range lastRange;
        auto newRange = *newPoolIt;

        if (diffRanges.empty() || diffRanges.back().second < range.first)
            diffRanges.push_back(range);

        lastRange = diffRanges.back();

        const bool firstInLastRange = newRange.first > lastRange.first && newRange.first < lastRange.second;
        const bool secondInLastRange = newRange.second > lastRange.first && newRange.second < lastRange.second;

        if (firstInLastRange) {
            diffRanges.back().second = newRange.first - 1;

            if (secondInLastRange) {
                diffRanges.push_back(Pool::makeRange(newRange.second + 1, lastRange.second));
                ++newPoolIt;
            }
        } else if (secondInLastRange) {
            diffRanges.back().first = newRange.second + 1;
            ++newPoolIt;
        } else if (newRange.first < lastRange.first && newRange.second > lastRange.second) {
            diffRanges.pop_back();
            ++it;
        } else if (newRange.first == lastRange.first && newRange.second == lastRange.second) {
            diffRanges.pop_back();
            ++newPoolIt; ++it;
        } else {
            ++it;
        }
    }

    for(auto range : diffRanges)
        result.addRange(range);

    return result;
}
