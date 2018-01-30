#include <gtest/gtest.h>
#include "Pool.h"

TEST(IPool, IPsInRanges)
{
    std::string ip0 = "192.168.0.1";
    std::string ip1 = "192.168.1.1";
    std::string ip2 = "192.168.2.1";

    auto range0 = Pool::makeRange("192.168.0.0", "192.168.0.100");
    auto range1 = Pool::makeRange("192.168.1.0", "192.168.1.100");
    auto range2 = Pool::makeRange("192.168.2.0", "192.168.2.100");

    Pool pool;
    pool.addRange(range0);
    pool.addRange(range1);
    pool.addRange(range2);

    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(range0, pool.findRange(ip0));
    EXPECT_EQ(range1, pool.findRange(ip1));
    EXPECT_EQ(range2, pool.findRange(ip2));
}

TEST(IPool, IPsNotInRanges)
{
    std::string ip0 = "192.167.0.1";
    std::string ip1 = "193.168.1.1";
    std::string ip2 = "192.108.2.1";

    auto range0 = Pool::makeRange("192.168.0.0", "192.168.0.100");
    auto range1 = Pool::makeRange("192.168.1.0", "192.168.1.100");
    auto range2 = Pool::makeRange("192.168.2.0", "192.168.2.100");

    Pool pool;
    pool.addRange(range0);
    pool.addRange(range1);
    pool.addRange(range2);

    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(Optional<Range>{}, pool.findRange(ip0));
    EXPECT_EQ(Optional<Range>{}, pool.findRange(ip1));
    EXPECT_EQ(Optional<Range>{}, pool.findRange(ip2));
}

TEST(IPool, MergeRanges)
{
    std::string ip0 = "192.168.0.1";
    std::string ip1 = "192.168.1.1";
    std::string ip2 = "192.168.2.1";

    auto range0 = Pool::makeRange("192.168.0.0", "192.168.0.100");
    auto range1 = Pool::makeRange("192.168.1.0", "192.168.1.100");
    auto range2 = Pool::makeRange("192.168.2.0", "192.168.2.100");

    Pool pool;
    pool.addRange(range0);
    pool.addRange(range1);
    pool.addRange(range2);

    auto range00 = Pool::makeRange("192.168.0.0", "192.168.0.100");
    pool.addRange(range00);

    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(range0, pool.findRange(ip0));
    EXPECT_EQ(range1, pool.findRange(ip1));
    EXPECT_EQ(range2, pool.findRange(ip2));

    auto range01 = Pool::makeRange("192.167.0.200", "192.168.0.200");
    pool.addRange(range01);

    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(range01, pool.findRange(ip0));
    EXPECT_EQ(range1, pool.findRange(ip1));
    EXPECT_EQ(range2, pool.findRange(ip2));

    auto range11 = Pool::makeRange("192.168.0.250", "192.168.1.50");
    pool.addRange(range11);

    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(range01, pool.findRange(ip0));
    EXPECT_EQ(Pool::makeRange(range11.first, range1.second), pool.findRange(ip1));
    EXPECT_EQ(range2, pool.findRange(ip2));

    auto range22 = Pool::makeRange("192.168.2.50", "192.168.2.150");
    pool.addRange(range22);

    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(range01, pool.findRange(ip0));
    EXPECT_EQ(Pool::makeRange(range11.first, range1.second), pool.findRange(ip1));
    EXPECT_EQ(Pool::makeRange(range2.first, range22.second), pool.findRange(ip2));

    auto range02 = Pool::makeRange("192.168.0.0", "192.168.2.50");
    pool.addRange(range02);

    EXPECT_EQ(1, pool.size());
    EXPECT_EQ(Pool::makeRange(range01.first, range22.second), pool.findRange(ip0));
    EXPECT_EQ(Pool::makeRange(range01.first, range22.second), pool.findRange(ip1));
    EXPECT_EQ(Pool::makeRange(range01.first, range22.second), pool.findRange(ip2));

    auto range3 = Pool::makeRange("192.167.0.0", "192.168.3.0");
    pool.addRange(range3);

    EXPECT_EQ(1, pool.size());
    EXPECT_EQ(range3, pool.findRange(ip0));
    EXPECT_EQ(range3, pool.findRange(ip1));
    EXPECT_EQ(range3, pool.findRange(ip2));
}

TEST(IPool, DiffRanges)
{
    Pool pool;
    pool.addRange(Pool::makeRange("192.168.0.0", "192.168.0.100"));
    pool.addRange(Pool::makeRange("192.168.1.0", "192.168.1.100"));
    pool.addRange(Pool::makeRange("192.168.2.0", "192.168.2.100"));

    Pool newPool;
    newPool.addRange(Pool::makeRange("192.168.0.50", "192.168.0.80"));
    newPool.addRange(Pool::makeRange("192.168.0.90", "192.168.1.80"));
    newPool.addRange(Pool::makeRange("192.168.2.10", "192.168.2.90"));

    auto diffPool = find_diff(pool, newPool);

    ASSERT_EQ(5, diffPool.size());

    auto poolIt = diffPool.begin();
    EXPECT_EQ(Pool::makeRange("192.168.0.0", "192.168.0.49"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.0.81", "192.168.0.89"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.1.81", "192.168.1.100"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.2.0", "192.168.2.9"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.2.91", "192.168.2.100"), *poolIt++);

    newPool.clear();
    newPool.addRange(Pool::makeRange("192.167.0.50", "192.169.0.80"));
    diffPool = find_diff(pool, newPool);
    EXPECT_EQ(0, diffPool.size());

    newPool.clear();
    newPool.addRange(Pool::makeRange("192.167.0.50", "192.168.0.80"));
    diffPool = find_diff(pool, newPool);
    ASSERT_EQ(3, diffPool.size());

    poolIt = diffPool.begin();
    EXPECT_EQ(Pool::makeRange("192.168.0.81", "192.168.0.100"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.1.0", "192.168.1.100"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.2.0", "192.168.2.100"), *poolIt++);

    newPool.clear();
    newPool.addRange(Pool::makeRange("192.168.0.50", "192.168.2.50"));
    diffPool = find_diff(pool, newPool);
    ASSERT_EQ(2, diffPool.size());

    poolIt = diffPool.begin();
    EXPECT_EQ(Pool::makeRange("192.168.0.0", "192.168.0.49"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.2.51", "192.168.2.100"), *poolIt++);

    newPool.clear();
    newPool.addRange(Pool::makeRange("192.168.0.0", "192.168.0.100"));
    diffPool = find_diff(pool, newPool);
    ASSERT_EQ(2, diffPool.size());

    poolIt = diffPool.begin();
    EXPECT_EQ(Pool::makeRange("192.168.1.0", "192.168.1.100"), *poolIt++);
    EXPECT_EQ(Pool::makeRange("192.168.2.0", "192.168.2.100"), *poolIt++);
}
