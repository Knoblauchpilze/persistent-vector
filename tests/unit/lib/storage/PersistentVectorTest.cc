
#include "PersistentVector.hh"
#include "PersistentVectorBlock.hh"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace std::literals;

namespace storage {
using PersistentVector = v2::PersistentVector;

namespace {
constexpr std::size_t LOOP_COUNT = 100000u;

auto createTempDirectory() -> std::filesystem::path
{
  std::filesystem::path dataDir("dataDir");
  EXPECT_TRUE(std::filesystem::create_directory(dataDir));
  return dataDir;
}

auto generateAllChars() -> std::string
{
  std::string rv;

  for (char c = std::numeric_limits<char>::min(); c != std::numeric_limits<char>::max(); ++c)
  {
    rv += c;
  }

  rv += std::numeric_limits<char>::max();

  return rv;
}
} // namespace

TEST(Unit_Storage_PersistentVector, Test_One)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  const auto path = createTempDirectory();
  PersistentVector vec(path);

  vec.push_back("foo");
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(1, vec.size());

  const auto allChars = generateAllChars();
  vec.push_back(allChars);

  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ(2, vec.size());

  auto start = std::chrono::system_clock::now();

  auto start2 = std::chrono::system_clock::now();
  for (auto i = 0u; i < LOOP_COUNT; ++i)
  {
    if (i != 0u && i % 1000 == 0)
    {
      auto end2 = std::chrono::system_clock::now();
      std::cout << "Duration for " << i - 1000 << " - " << i << " took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count()
                << std::endl;
      start2 = std::chrono::system_clock::now();
    }
    std::stringstream s;
    s << "loop " << i;
    vec.push_back(s.str());
  }

  auto end = std::chrono::system_clock::now();

  std::cout << "Duration: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << std::endl;

  ASSERT_LT((end - start) / 1s, 1);
  ASSERT_EQ(LOOP_COUNT + 2, vec.size());
}

TEST(Unit_Storage_PersistentVector, Test_Two)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  const auto allChars = generateAllChars();

  ASSERT_EQ(LOOP_COUNT + 2, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 871", vec.at(873));

  vec.erase(873);

  ASSERT_EQ(LOOP_COUNT + 1, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 872", vec.at(873));
}

TEST(Unit_Storage_PersistentVector, Test_Three)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  const auto allChars = generateAllChars();

  ASSERT_EQ(LOOP_COUNT + 1, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 872", vec.at(873));

  vec.erase(873);

  ASSERT_EQ(LOOP_COUNT, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 873", vec.at(873));
}

TEST(Unit_Storage_PersistentVector, DISABLED_Test_Four)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  auto start = std::chrono::system_clock::now();

  auto start2 = std::chrono::system_clock::now();
  while (vec.size() > 0)
  {
    if (vec.size() % 1000 == 0)
    {
      auto end2 = std::chrono::system_clock::now();
      std::cout << "Duration for removing " << vec.size() + 1000 << " - " << vec.size() << " took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count()
                << std::endl;
      start2 = std::chrono::system_clock::now();
    }

    vec.erase(vec.size() - 1);
  }

  auto end = std::chrono::system_clock::now();

  ASSERT_LT((end - start) / 1s, 1);
  ASSERT_EQ(0, vec.size());
}

TEST(Unit_Storage_PersistentVector, Test_Two_Short)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  const auto allChars = generateAllChars();

  ASSERT_EQ(LOOP_COUNT + 2, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 6", vec.at(8));

  vec.erase(8);

  ASSERT_EQ(LOOP_COUNT + 1, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 7", vec.at(8));
}

TEST(Unit_Storage_PersistentVector, Test_Two_SecondBlock)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  const auto allChars = generateAllChars();

  ASSERT_EQ(LOOP_COUNT + 2, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 100", vec.at(102));

  vec.erase(102);

  ASSERT_EQ(LOOP_COUNT + 1, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 101", vec.at(102));
}

TEST(Unit_Storage_PersistentVector, Test_Three_Short)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  const auto allChars = generateAllChars();

  ASSERT_EQ(LOOP_COUNT + 1, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 7", vec.at(8));

  vec.erase(8);

  ASSERT_EQ(LOOP_COUNT, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 8", vec.at(8));
}

TEST(Unit_Storage_PersistentVector, Test_Three_SecondBlock)
{
  std::filesystem::current_path(std::filesystem::temp_directory_path());

  std::filesystem::path dataDir("dataDir");
  PersistentVector vec(dataDir);

  const auto allChars = generateAllChars();

  ASSERT_EQ(LOOP_COUNT + 1, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 101", vec.at(102));

  vec.erase(102);

  ASSERT_EQ(LOOP_COUNT, vec.size());
  ASSERT_EQ("foo", vec.at(0));
  ASSERT_EQ(allChars, vec.at(1));
  ASSERT_EQ("loop 102", vec.at(102));
}

} // namespace storage
