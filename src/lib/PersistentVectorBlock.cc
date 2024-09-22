
#include "PersistentVectorBlock.hh"

#include <fstream>
#include <iostream>

namespace storage::v2 {

constexpr auto HEADER_FILE_NAME                        = "HEADER.txt";
constexpr auto INDEX_FILE_NAME                         = "INDEX.txt";
constexpr std::size_t DATA_BLOCK_DIRECTORY_NAME_LENGTH = 4;
constexpr std::size_t ELEMENT_FILE_NAME_LENGTH         = 8;
constexpr auto ELEMENT_FILE_EXTENSION                  = ".txt";
constexpr std::size_t DATA_BLOCK_ELEMENT_SIZE          = 4096;
constexpr std::size_t DATA_BLOCK_SIZE                  = 100;

PersistentVector::PersistentVector(const std::filesystem::path &directory)
  : directory(directory)
  , headerFilePath(directory / HEADER_FILE_NAME)
  , indexFilePath(directory / INDEX_FILE_NAME)
  , saveToDiskBuffer(DATA_BLOCK_ELEMENT_SIZE, '\0')
{
  this->init();
}

auto PersistentVector::size() const -> std::size_t
{
  return this->length;
}

auto PersistentVector::at(const std::size_t index) const -> std::string_view
{
  if (index >= this->length)
  {
    throw std::out_of_range("Requested size " + std::to_string(index) + " but only "
                            + std::to_string(this->length) + " available");
  }

  // TODO: Check that the data block is valid.
  const auto dataBlockId = this->findDataBlockIdForIndex(index);

  auto &dataBlock = *this->dataBlocks.at(dataBlockId);
  if (dataBlock.cachedData)
  {
    std::cout << "[INFO] Element " << index
              << " was already in cache (size: " << dataBlock.cachedData->size() << ")\n";
    return this->fetchElementDataFromDataBlock(dataBlock, index);
  }

  // TODO: Verify that the size matches what we expect.
  dataBlock.cachedData = this->loadDataBlockFromDisk(dataBlock.path);

  std::cout << "[INFO] Loaded element " << index << " from " << dataBlock.path << " with size "
            << dataBlock.cachedData->size() << "\n";

  return this->fetchElementDataFromDataBlock(dataBlock, index);
}

void PersistentVector::push_back(const std::string &value)
{
  if (this->capacity == 0u || this->length >= this->capacity)
  {
    std::cout << "[INFO] Need to grow for \"" << value << "\", length is " << this->length
              << " and capacity " << this->capacity << "\n";
    this->grow();
  }

  // TODO: Check that the size of the value is not bigger than expected.
  auto &dataBlock = this->dataBlocks.back();
  this->saveElementToDisk(*dataBlock, value);
  dataBlock->cachedData.reset();
  ++this->length;

  // std::cout << "[INFO] Saved element " << this->length << " at " << dataBlock->path
  //           << ", size: " << value.size() << "\n";

  this->updateState(Operation::INSERT);
}

void PersistentVector::erase(const std::size_t index)
{
  if (index >= this->length)
  {
    throw std::out_of_range("Cannot erase element " + std::to_string(index) + ", only "
                            + std::to_string(this->length) + " available");
  }

  const auto dataBlockId = this->findDataBlockIdForIndex(index);
  auto &dataBlock        = *this->dataBlocks[dataBlockId];

  std::cout << "[INFO] Erasing element " << index << " out of " << this->length
            << " (capacity: " << this->capacity << ")\n";

  this->removeFromDataBlock(dataBlock, index);
  --dataBlock.size;
  dataBlock.cachedData.reset();

  this->updateFollowingDataBlocks(dataBlockId + 1);

  if (dataBlock.size == 0)
  {
    this->eraseElementFromDisk(dataBlock.path);

    auto toErase = this->dataBlocks.begin();
    std::advance(toErase, index);
    this->dataBlocks.erase(toErase);
  }

  --this->length;
  --this->capacity;

  this->updateState(Operation::ERASE);

  std::cout << "[INFO] Erased element " << index << " at " << dataBlock.path << "\n";
}

void PersistentVector::init()
{
  if (std::filesystem::exists(this->headerFilePath))
  {
    std::cout << "[INFO] Detected existing content at " << this->headerFilePath << ", loading...\n";
    this->loadFromDisk();
  }
  else
  {
    std::cout << "[INFO] Initializing empty directory at " << this->headerFilePath << "\n";
    this->saveToDisk();
  }

  // TODO: Check that the file was correctly open.
  this->headerFileStream.open(this->headerFilePath, std::ofstream::trunc);
  this->saveHeader();
}

void PersistentVector::updateState(const Operation &operation)
{
  this->saveHeader();
  if (operation == Operation::ERASE)
  {
    this->saveIndex();
  }
}

void PersistentVector::loadFromDisk()
{
  this->dataBlocks.clear();

  this->loadHeader();

  std::cout << "[INFO] Found " << this->length << " element(s) to load from "
            << this->headerFilePath << " (capacity: " << this->capacity << ")\n";

  this->loadIndex();
}

void PersistentVector::saveToDisk()
{
  this->saveHeader();
  this->saveIndex();
}

void PersistentVector::loadHeader()
{
  // TODO: Check that the file was correctly open.
  std::ifstream headerFile(this->headerFilePath);

  headerFile >> this->capacity >> this->length;

  std::size_t capacityFromFile;
  std::size_t lengthFromFile;

  bool moreEntries = (headerFile.peek() != EOF);
  while (moreEntries)
  {
    headerFile >> capacityFromFile >> lengthFromFile;
    moreEntries = (headerFile.peek() != EOF);

    if (moreEntries)
    {
      this->capacity = capacityFromFile;
      this->length   = lengthFromFile;
    }
  }

  std::cout << "[INFO] Loaded capacity " << this->capacity << " and length " << this->length
            << " from " << this->headerFilePath << "\n";
}

void PersistentVector::saveHeader()
{
  // TODO: Maybe use binary instead of plain text.
  this->headerFileStream << this->capacity << " " << this->length << "\n";
  this->headerFileStream.flush();
}

void PersistentVector::loadIndex()
{
  std::ifstream indexFile(this->indexFilePath);

  const auto dataBlocksCount = (this->capacity + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
  for (std::size_t id = 0; id < dataBlocksCount; ++id)
  {
    std::size_t firstId, size;
    std::filesystem::path path;
    indexFile >> firstId >> size >> path;

    std::ofstream dataStream(path, std::ios_base::app);

    auto dataBlock        = std::make_unique<DataBlock>();
    dataBlock->path       = path;
    dataBlock->dataStream = std::move(dataStream);
    dataBlock->firstId    = firstId;
    dataBlock->size       = size;

    std::cout << "[INFO] Loading element " << id << " with path " << path << " and first id "
              << firstId << " and size " << size << "\n";

    this->dataBlocks.push_back(std::move(dataBlock));
  }
}

void PersistentVector::saveIndex() const
{
  std::ofstream indexFile;
  // TODO: Check that the file was correctly open.
  indexFile.open(this->indexFilePath, std::ofstream::trunc);

  // TODO: Maybe use binary instead of plain text.
  for (std::size_t id = 0; id < this->dataBlocks.size(); ++id)
  {
    const auto &dataBlock = *this->dataBlocks[id];
    indexFile << dataBlock.firstId << " " << dataBlock.size << " " << dataBlock.path << "\n";
  }
}

void PersistentVector::appendToIndex(const DataBlock &dataBlock) const
{
  std::ofstream indexFile;
  // TODO: Check that the file was correctly open.
  indexFile.open(this->indexFilePath, std::ofstream::app);

  indexFile << dataBlock.firstId << " " << dataBlock.size << " " << dataBlock.path << "\n";
}

auto PersistentVector::loadDataBlockFromDisk(const std::filesystem::path &path) const -> std::string
{
  std::ifstream in(path, std::ios::binary | std::ios::ate);
  const auto size = in.tellg();
  in.seekg(0, std::ios::beg);

  std::string buffer;
  buffer.resize(size);
  in.read(buffer.data(), size);

  std::cout << "[INFO] Loading content of " << path << " (size: " << buffer.size() << ", " << size
            << "): \"" << buffer << "\"\n";
  return buffer;
}

void PersistentVector::saveElementToDisk(DataBlock &dataBlock, const std::string &value)
{
  std::fill(this->saveToDiskBuffer.begin(), this->saveToDiskBuffer.end(), '\0');

  const auto valueSize   = value.size();
  const auto rawSizeData = reinterpret_cast<const char *>(&valueSize);
  std::copy(rawSizeData, rawSizeData + sizeof(std::size_t), this->saveToDiskBuffer.begin());

  auto startOfData = this->saveToDiskBuffer.begin();
  std::advance(startOfData, sizeof(std::size_t));
  std::copy(value.begin(), value.end(), startOfData);

  dataBlock.dataStream.write(this->saveToDiskBuffer.c_str(), this->saveToDiskBuffer.size());
  dataBlock.dataStream.flush();

  // std::cout << "[INFO] Saved \"" << value << "\" to " << dataBlock.path << "\n";
}

void PersistentVector::eraseElementFromDisk(const std::filesystem::path &path) const
{
  // TODO: Check that the content was actually deleted.
  std::filesystem::remove(path);
  std::cout << "[INFO] Erased content at " << path << "\n";
}

namespace {
const std::string SYMBOLS = "0123456789abcdefghijklmnopqrstuvwxyz";

auto generateRandomPrefix(const std::size_t length) -> std::string
{
  std::string out;
  out.reserve(length);

  for (std::size_t id = 0; id < length; ++id)
  {
    out += SYMBOLS[std::rand() % SYMBOLS.size()];
  }

  return out;
}

auto generateRandomFileName(const std::size_t length, const std::string_view extension)
  -> std::string
{
  auto out = generateRandomPrefix(length);
  out += extension;
  return out;
}
} // namespace

void PersistentVector::grow()
{
  std::cout << "[INFO] Growing, current length: " << this->length << " and capacity "
            << this->capacity << "\n";

  // TODO: Check that path does not exist yet.
  const auto fileName = generateRandomFileName(ELEMENT_FILE_NAME_LENGTH, ELEMENT_FILE_EXTENSION);
  const auto filePath = this->directory / fileName;

  std::ofstream dataStream(filePath, std::ios_base::trunc);

  auto dataBlock        = std::make_unique<DataBlock>();
  dataBlock->path       = filePath;
  dataBlock->dataStream = std::move(dataStream);
  dataBlock->firstId    = this->capacity;
  dataBlock->size       = DATA_BLOCK_SIZE;

  this->dataBlocks.push_back(std::move(dataBlock));

  this->capacity += DATA_BLOCK_SIZE;

  this->saveHeader();
  this->appendToIndex(*this->dataBlocks.back());
}

auto PersistentVector::findDataBlockIdForIndex(const std::size_t index) const -> std::size_t
{
  std::size_t dataBlockId = 1;
  bool tooFar             = false;
  while (dataBlockId < this->dataBlocks.size() && !tooFar)
  {
    const auto &dataBlock = *this->dataBlocks[dataBlockId];
    tooFar                = dataBlock.firstId > index;
    if (!tooFar)
    {
      ++dataBlockId;
    }
  }

  return dataBlockId - 1;
}

auto PersistentVector::fetchElementDataFromDataBlock(const DataBlock &dataBlock,
                                                     const std::size_t index) const
  -> std::string_view
{
  const auto elementDataBlockId            = index - dataBlock.firstId;
  const auto positionOfSizeInDataStream    = elementDataBlockId * DATA_BLOCK_ELEMENT_SIZE;
  const auto positionOfElementInDataStream = positionOfSizeInDataStream + sizeof(std::size_t);

  const auto rawBlockData = dataBlock.cachedData->c_str();

  const auto elementSize = *reinterpret_cast<const std::size_t *>(rawBlockData
                                                                  + positionOfSizeInDataStream);

  std::string_view out(rawBlockData + positionOfElementInDataStream,
                       rawBlockData + positionOfElementInDataStream + elementSize);

  std::cout << "[INFO] Determined size " << elementSize << " for element " << index
            << " (data block offset: " << dataBlock.firstId << "), value = \"" << out << "\"\n";

  return out;
}

void PersistentVector::removeFromDataBlock(DataBlock &dataBlock, const std::size_t index)
{
  dataBlock.cachedData = this->loadDataBlockFromDisk(dataBlock.path);
  // TODO: Verify that we don't have truncated data.
  const auto actualElementsCount = dataBlock.cachedData->size() / DATA_BLOCK_ELEMENT_SIZE;

  dataBlock.dataStream.close();
  dataBlock.dataStream.open(dataBlock.path, std::ios_base::trunc);

  const auto elementDataBlockId = index - dataBlock.firstId;
  for (std::size_t id = 0; id < actualElementsCount; ++id)
  {
    if (id == elementDataBlockId)
    {
      continue;
    }

    const auto elementId = dataBlock.firstId + id;
    const auto value     = std::string(this->fetchElementDataFromDataBlock(dataBlock, elementId));

    std::cout << "[INFO] Saving element " << elementId << " (block index: " << id
              << ", offset: " << dataBlock.firstId << ") \"" << value << "\" to " << dataBlock.path
              << "\n";

    this->saveElementToDisk(dataBlock, value);
  }
}

void PersistentVector::updateFollowingDataBlocks(const std::size_t startDataBlockId)
{
  for (std::size_t id = startDataBlockId; id < this->dataBlocks.size(); ++id)
  {
    --this->dataBlocks[id]->firstId;
  }
}

} // namespace storage::v2
