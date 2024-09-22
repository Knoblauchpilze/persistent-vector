#include <filesystem>
#include <iostream>
#include <string>

#include "PersistentVector.hh"
#include "PersistentVectorBlock.hh"

using PersistentVector = storage::v2::PersistentVector;

std::size_t errors = 0;

#define ERROR(msg)                                                  \
  {                                                                 \
    std::cout << __FILE__ << ":" << __LINE__ << " " << msg << "\n"; \
    ++errors;                                                       \
  }
#define CHECK(x)           \
  do                       \
    if (!(x))              \
    {                      \
      ERROR(#x " failed"); \
    }                      \
  while (false)

constexpr unsigned LOOP_COUNT = 100000;

std::string all_chars()
{
  std::string rv;
  for (char c = std::numeric_limits<char>::min(); c != std::numeric_limits<char>::max(); ++c)
  {
    rv += c;
  }

  rv += std::numeric_limits<char>::max();
  return rv;
}

void run_test_one(const std::filesystem::path &p)
{
  PersistentVector v(p);
  using namespace std::literals;

  v.push_back("foo");
  CHECK(v.at(0) == "foo");
  CHECK(v.size() == 1);

  v.push_back(all_chars());
  CHECK(v.at(1) == all_chars());
  CHECK(v.size() == 2);

  auto start = std::chrono::system_clock::now();
  for (auto i = 0u; i < LOOP_COUNT; ++i)
  {
    std::stringstream s;
    s << "loop " << i;
    v.push_back(s.str());
  }
  auto end = std::chrono::system_clock::now();
  CHECK((end - start) / 1s < 1);
  CHECK(v.size() == LOOP_COUNT + 2);
}

void run_test_two(const std::filesystem::path &p)
{
  PersistentVector v(p);

  CHECK(v.size() == LOOP_COUNT + 2);
  CHECK(v.at(0) == "foo");
  CHECK(v.at(1) == all_chars());
  CHECK(v.at(873) == "loop 871");

  v.erase(873);
  CHECK(v.size() == LOOP_COUNT + 1);
  CHECK(v.at(0) == "foo");
  CHECK(v.at(1) == all_chars());
  CHECK(v.at(873) == "loop 872");
}

void run_test_three(const std::filesystem::path &p)
{
  PersistentVector v(p);

  CHECK(v.size() == LOOP_COUNT + 1);
  CHECK(v.at(0) == "foo");
  CHECK(v.at(1) == all_chars());
  CHECK(v.at(873) == "loop 872");

  v.erase(873);
  CHECK(v.size() == LOOP_COUNT);
  CHECK(v.at(0) == "foo");
  CHECK(v.at(1) == all_chars());
  CHECK(v.at(873) == "loop 873");
}

int main(int /*argc*/, char * /*argv*/[])
{
  constexpr auto DEFAULT_DATA_DIR = "dataDir";

  std::filesystem::current_path(std::filesystem::temp_directory_path());
  std::filesystem::remove_all(DEFAULT_DATA_DIR);

  std::filesystem::path data_dir(DEFAULT_DATA_DIR);
  std::filesystem::create_directory(data_dir);

  run_test_one(data_dir);
  run_test_two(data_dir);
  run_test_three(data_dir);

  if (errors != 0)
  {
    std::cout << "tests were failing\n";
    return 1;
  }

  std::cout << "tests succeeded\n";
  return 0;
}
