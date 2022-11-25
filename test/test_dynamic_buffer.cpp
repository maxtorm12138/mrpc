#include <mrpc/mrpc.hpp>
#include <gtest/gtest.h>

TEST(DynamicBufferAdaptorTest, PointerConstruct)
{
    std::string source = "Hello World";
    std::string dest;

    auto buffer_dest = mrpc::net::dynamic_buffer(dest);

    mrpc::dynamic_buffer_adaptor adaptor(&buffer_dest);
    adaptor.grow(source.size());

    mrpc::net::buffer_copy(adaptor.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, ReferenceConstruct)
{
    std::string source = "Hello World";
    std::string dest;

    auto buffer_dest = mrpc::net::dynamic_buffer(dest);
    mrpc::dynamic_buffer_adaptor adaptor(buffer_dest);

    adaptor.grow(source.size());

    mrpc::net::buffer_copy(adaptor.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, RvalueConstruct)
{
    std::string source = "Hello World";
    std::string dest;

    mrpc::dynamic_buffer_adaptor adaptor(mrpc::net::dynamic_buffer(dest));

    adaptor.grow(source.size());

    mrpc::net::buffer_copy(adaptor.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, CopyConstruct)
{
    std::string source = "Hello World";
    std::string dest;

    mrpc::dynamic_buffer_adaptor adaptor(dest);
    adaptor.grow(source.size());

    auto adaptor_copy = adaptor;

    mrpc::net::buffer_copy(adaptor_copy.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor_copy.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, StringConstruct)
{
    std::string source = "Hello World";
    std::string dest;

    mrpc::dynamic_buffer_adaptor adaptor(dest);
    adaptor.grow(source.size());

    mrpc::net::buffer_copy(adaptor.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, VectorConstruct)
{
    const std::vector<uint8_t> source{1, 1, 4, 5, 1, 4};
    std::vector<uint8_t> dest;

    mrpc::dynamic_buffer_adaptor adaptor(dest);
    adaptor.grow(source.size());

    mrpc::net::buffer_copy(adaptor.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(adaptor.size(), source.size());
}

TEST(DynamicBufferAdaptorTest, MoveConstruct)
{
    std::string source = "Hello World";
    std::string dest;

    mrpc::dynamic_buffer_adaptor adaptor(dest);
    adaptor.grow(source.size());

    auto adaptor_copy = std::move(adaptor);

    mrpc::net::buffer_copy(adaptor_copy.data(0, source.size()), mrpc::net::buffer(source));

    EXPECT_EQ(source, dest);
    EXPECT_EQ(dest, "Hello World");
    EXPECT_EQ(adaptor_copy.size(), source.size());
}