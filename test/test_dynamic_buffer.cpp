#include <mrpc/mrpc.hpp>
#include <gtest/gtest.h>

TEST(DynamicBufferAdaptorTest, DynamicStringBuffer)
{
    std::string source = "Hello World";
    std::string dest;

    auto buffer_dest = mrpc::net::dynamic_buffer(dest);
    mrpc::dynamic_buffer_adaptor adaptor(buffer_dest);
    adaptor.grow(source.size());

    auto dst = adaptor.data(0, source.size());
    auto src = mrpc::net::buffer(source);

    mrpc::net::buffer_copy(dst, src);

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, DynamicStringBufferCopy)
{
    std::string source = "Hello World";
    std::string dest;

    auto buffer_dest = mrpc::net::dynamic_buffer(dest);

    mrpc::dynamic_buffer_adaptor adaptor(buffer_dest);

    auto adaptor1 = adaptor;
    adaptor1.grow(source.size());

    auto dst = adaptor1.data(0, source.size());
    auto src = mrpc::net::buffer(source);

    mrpc::net::buffer_copy(dst, src);

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor1.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, DynamicVectorBuffer)
{
    const std::vector<uint8_t> source{1, 1, 4, 5, 1, 4};
    std::vector<uint8_t> dest;

    auto buffer_dest = mrpc::net::dynamic_buffer(dest);
    mrpc::dynamic_buffer_adaptor adaptor(buffer_dest);
    adaptor.grow(source.size());

    auto dst = adaptor.data(0, source.size());
    auto src = mrpc::net::buffer(source);

    mrpc::net::buffer_copy(dst, src);

    EXPECT_EQ(source, dest);
    EXPECT_EQ(adaptor.size(), source.size());
}
