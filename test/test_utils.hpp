#ifndef MRPC_TEST_UTILS_H
#define MRPC_TEST_UTILS_H
#include <gtest/gtest.h>
#include <boost/asio.hpp>
namespace net = boost::asio;

class net_test : public testing::Test
{
public:
    void TestBody() override
    {
        io_context_ = std::make_unique<net::io_context>(1);
        net::co_spawn(io_context_->get_executor(), coroutine_test_body(), [](std::exception_ptr ex) {
            if (ex != nullptr)
            {
                std::rethrow_exception(ex);
            }
        });

        io_context_->run();
    }

private:
    virtual net::awaitable<void> coroutine_test_body() = 0;

private:
    std::unique_ptr<net::io_context> io_context_;
};

#define MRPC_TEST_(test_suite_name, test_name, parent_id)                                                                                                                          \
    static_assert(sizeof(GTEST_STRINGIFY_(test_suite_name)) > 1, "test_suite_name must not be empty");                                                                             \
    static_assert(sizeof(GTEST_STRINGIFY_(test_name)) > 1, "test_name must not be empty");                                                                                         \
    class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                                                                                                       \
        : public net_test                                                                                                                                                          \
    {                                                                                                                                                                              \
    public:                                                                                                                                                                        \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() = default;                                                                                                            \
        ~GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() override = default;                                                                                                  \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                                                                                                         \
        (const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &) = delete;                                                                                                     \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &operator=(const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &) = delete; /* NOLINT */                           \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                                                                                                         \
        (GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &&) noexcept = delete;                                                                                                 \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &operator=(GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &&) noexcept = delete; /* NOLINT */                       \
                                                                                                                                                                                   \
    private:                                                                                                                                                                       \
        net::awaitable<void> coroutine_test_body() override;                                                                                                                       \
        static ::testing::TestInfo *const test_info_ GTEST_ATTRIBUTE_UNUSED_;                                                                                                      \
    };                                                                                                                                                                             \
                                                                                                                                                                                   \
    ::testing::TestInfo *const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::test_info_ =                                                                                    \
        ::testing::internal::MakeAndRegisterTestInfo(#test_suite_name, #test_name, nullptr, nullptr, ::testing::internal::CodeLocation(__FILE__, __LINE__), (parent_id),           \
            ::testing::internal::SuiteApiResolver<net_test>::GetSetUpCaseOrSuite(__FILE__, __LINE__),                                                                              \
            ::testing::internal::SuiteApiResolver<net_test>::GetTearDownCaseOrSuite(__FILE__, __LINE__),                                                                           \
            new ::testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)>);                                                                         \
    net::awaitable<void> GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::coroutine_test_body()

#define MRPC_TEST_F(test_fixture, test_name) MRPC_TEST_(test_fixture, test_name, ::testing::internal::GetTypeId<test_fixture>())

#endif
