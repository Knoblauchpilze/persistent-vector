
#include "PersistentVector.hh"

#include <fstream>
#include <iostream>

namespace storage::v1 {

constexpr auto HEADER_FILE_NAME                        = "HEADER.txt";
constexpr auto INDEX_FILE_NAME                         = "INDEX.txt";
constexpr std::size_t DATA_BLOCK_DIRECTORY_NAME_LENGTH = 4;
constexpr std::size_t ELEMENT_FILE_NAME_LENGTH         = 8;
constexpr auto ELEMENT_FILE_EXTENSION                  = ".txt";
constexpr std::size_t CAPACITY_INCREMENTS              = 10000;

PersistentVector::PersistentVector(const std::filesystem::path &directory)
  : directory(directory)
  , headerFilePath(directory / HEADER_FILE_NAME)
  , indexFilePath(directory / INDEX_FILE_NAME)
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

  auto &element = this->elements[index];
  if (element.cached)
  {
    std::cout << "[INFO] Element " << index
              << " was already in cache (size: " << element.cached->size() << ")\n";
    return *element.cached;
  }

  element.cached = this->loadElementFromDisk(element.path);
  // TODO: Verify that the size matches what we expect.

  std::cout << "[INFO] Loaded element " << index << " from " << element.path << " with size "
            << element.cached->size() << "\n";

  return *element.cached;
}

void PersistentVector::push_back(const std::string &value)
{
  if (this->capacity == 0u || this->length >= this->capacity - 1)
  {
    this->grow(CAPACITY_INCREMENTS);
  }

  auto &element = this->elements[this->length];
  this->saveElementToDisk(element.path, value);
  ++this->length;

  // std::cout << "[INFO] Saved element " << this->length << " at " << element.path
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

  const auto &element = this->elements.at(index);
  this->eraseElementFromDisk(element.path);

  auto toErase = this->elements.begin();
  std::advance(toErase, index);
  this->elements.erase(toErase);

  --this->length;
  --this->capacity;

  this->updateState(Operation::ERASE);

  std::cout << "[INFO] Erased element " << index << " at " << element.path << "\n";
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
  this->elements.clear();

  this->loadHeader();
  this->elements.reserve(this->capacity);

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
}

void PersistentVector::loadIndex()
{
  std::ifstream indexFile(this->indexFilePath);

  for (std::size_t id = 0; id < this->capacity; ++id)
  {
    std::size_t elementId;
    Element element{};
    indexFile >> elementId >> element.path;

    // std::cout << "[INFO] Loading element " << elementId << " with path " << element.path << "\n";

    this->elements.push_back(element);
  }
}

void PersistentVector::saveIndex() const
{
  std::ofstream indexFile;
  // TODO: Check that the file was correctly open.
  indexFile.open(this->indexFilePath, std::ofstream::trunc);

  // TODO: Maybe use binary instead of plain text.
  for (std::size_t id = 0; id < this->elements.size(); ++id)
  {
    indexFile << id << " " << this->elements[id].path << "\n";
  }
}

void PersistentVector::appendToIndex(const std::size_t startIndex) const
{
  std::ofstream indexFile;
  // TODO: Check that the file was correctly open.
  indexFile.open(this->indexFilePath, std::ofstream::app);

  // TODO: Maybe use binary instead of plain text.
  for (std::size_t id = startIndex; id < this->elements.size(); ++id)
  {
    indexFile << id << " " << this->elements[id].path << "\n";
  }
}

auto PersistentVector::loadElementFromDisk(const std::filesystem::path &path) const -> std::string
{
  std::ifstream in(path);
  std::stringstream buffer;
  buffer << in.rdbuf();

  auto data = buffer.str();
  std::cout << "[INFO] Loading content of " << path << " (size: " << data.size() << ")\n";
  return data;
}

void PersistentVector::saveElementToDisk(const std::filesystem::path &path,
                                         const std::string &value) const
{
  std::ofstream out;
  out.open(path, std::ofstream::binary | std::ofstream::trunc);

  out.write(value.c_str(), value.size());

  // std::cout << "[INFO] Saved \"" << value << "\" to " << path << "\n";
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

void PersistentVector::grow(const std::size_t sizeIncrement)
{
  std::cout << "[INFO] Growing by " << sizeIncrement << ", current length: " << this->length
            << " and capacity " << this->capacity << "\n";

  const auto directoryPrefix = generateRandomPrefix(DATA_BLOCK_DIRECTORY_NAME_LENGTH);
  auto blockDirectory        = this->directory / directoryPrefix;
  // TODO: Check that the directory does not exist and that it was successfully created.
  std::filesystem::create_directory(blockDirectory);

  for (std::size_t id = 0; id < sizeIncrement; ++id)
  {
    // TODO: Check that path does not exist yet.
    const auto fileName = generateRandomFileName(ELEMENT_FILE_NAME_LENGTH, ELEMENT_FILE_EXTENSION);
    const auto filePath = blockDirectory / fileName;

    Element element{
      .path   = filePath,
      .cached = {},
    };

    // TODO: Reserve instead of pushing back.
    this->elements.push_back(element);
  }

  this->capacity = this->elements.size();

  this->saveHeader();
  this->appendToIndex(this->capacity - sizeIncrement);
}

} // namespace storage::v1
