
#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace storage::v1 {

// TODO: Not thread safe.
class PersistentVector
{
  public:
  explicit PersistentVector(const std::filesystem::path &directory);

  auto size() const -> std::size_t;
  auto at(const std::size_t index) const -> std::string_view;
  void push_back(const std::string &value);
  void erase(const std::size_t index);

  private:
  std::filesystem::path directory{};
  std::filesystem::path headerFilePath{};
  std::ofstream headerFileStream{};
  std::filesystem::path indexFilePath{};

  struct Element
  {
    std::filesystem::path path{};
    mutable std::optional<std::string> cached{};
  };

  std::size_t capacity{};
  std::size_t length{};
  std::vector<Element> elements{};

  enum class Operation
  {
    INSERT,
    ERASE
  };

  void init();
  void updateState(const Operation &operation);

  void loadFromDisk();
  void saveToDisk();

  void loadHeader();
  void saveHeader();

  void loadIndex();
  void saveIndex() const;
  void appendToIndex(const std::size_t startIndex) const;

  auto loadElementFromDisk(const std::filesystem::path &path) const -> std::string;
  void saveElementToDisk(const std::filesystem::path &path, const std::string &value) const;
  void eraseElementFromDisk(const std::filesystem::path &path) const;

  void grow(const std::size_t sizeIncrement);
};

} // namespace storage::v1
