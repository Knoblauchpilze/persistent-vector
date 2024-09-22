
#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace storage::v2 {

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

  struct DataBlock
  {
    std::filesystem::path path{};
    std::ofstream dataStream{};
    std::size_t firstId{};
    std::size_t size{};
    mutable std::optional<std::string> cachedData{};
  };

  std::size_t capacity{};
  std::size_t length{};
  std::vector<std::unique_ptr<DataBlock>> dataBlocks{};

  std::string saveToDiskBuffer{};

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
  void appendToIndex(const DataBlock &dataBlock) const;

  auto loadDataBlockFromDisk(const std::filesystem::path &path) const -> std::string;
  void saveElementToDisk(DataBlock &dataBlock, const std::string &value);
  void eraseElementFromDisk(const std::filesystem::path &path) const;

  void grow();

  auto findDataBlockIdForIndex(const std::size_t index) const -> std::size_t;
  auto fetchElementDataFromDataBlock(const DataBlock &dataBlock, const std::size_t index) const
    -> std::string_view;
  void removeFromDataBlock(DataBlock &dataBlock, const std::size_t index);
  void updateFollowingDataBlocks(const std::size_t startDataBlockId);
};

} // namespace storage::v2
