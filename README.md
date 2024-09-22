# Persistent vector

## Task

In `task.cc` you will find the interface of a sample class `vector` as well as
a set of unit tests. Your task is to implement the functions defined in `vector`
so that they comply with the following requirements:

- the contents of `vector` are persisted to the directory given to the c'tor.
- the `vector` component recreates a previous state from the persistence directory
  when created
- the persistence schema must account for unexpected service shutdowns, like in the
  case of power outages
- the functions produce the same results as their STL counterparts, ie.
  `push_back` will append a string to the end of the `vector`
- all test cases shall pass

You may assume that:

- your data directory has unlimited disk space
- strings added to the vector are less than 4K long
- you can ignore RAM limitations

## Installation instructions

You can use the root [Makefile](Makefile) to run the tests.

### With a standalone executable

To do this you can just run `make tests` in the root directory of the project.

### With GTest

To use this you need to have [google test](https://github.com/google/googletest) installed. Usually running the following command is enough:

```bash
sudo apt-get install libgtest-dev
```

You can then run `make gtests`.

## Implementation details

This repository contains two implementation of the persistent vector. They are available respectively under `storage::v1::PersistentVector` (defined in [PersistentVector.hh](src/lib/PersistentVector.hh)) and `storage::v2::PersistentVector` (defined in [PersistentVectorBlock](src/lib/PersistentVectorBlock.hh)).

**Note:** by default `v2` is active.

### More info about v1

The persistent vector defined in the `v1` namespace uses the following approach:

- in the directory passed to the vector we have a `HEADER.txt` file which contains the list of capacities/lengths that the vector assumed during runtime.
- there's also a `INDEX.txt` file which contains a list of directories holding the vector's data.
- the vector grows in 'blocks' which contains a parameterizable amount of elements.
- each data block is stored in a dedicated folder to avoid listing too many files at once for large vectors.
- the data block folders contain one file per element of the vector.
- adding and removing an element of the vector means removing/adding a file on the corresponding directory.

This is relatively simple but does not meet the performance criteria for the insertion (taking about 2s instead of 1s for 100k elements).

### More info about v2

The persistent vector defined in the `v2` namespace uses the following approach:

- in the directory passed to the vector we have a `HEADER.txt` file which contains the list of capacities/lengths that the vector assumed during runtime.
- there's also a `INDEX.txt` file which contains a list of files holding the vector's data.
- the vector grows in 'blocks' which contains a parameterizable amount of elements.
- each data block is stored in a single file.
- we assume that the maximum size of an element stored in the vector is known, meaning that we can estimate the size of each data block before hand.
- each data block file contains 'regions' of identical size holding the element at a specific index.
- adding an element means adding an entry to the last data block. If there's no space left we create a new data block.
- erasing an element means reorganizing the data in the data block where the element is stored.
- this is done by recreating the file and skipping the erased value, effectively producing a data block 'shorter' than the other ones.
- we then have someb bookkeeping to do to update the first index held in each subsequent data block.

This vector matches the criteria in terms of performance (about 600ms for 100k elements).

### Additional consideration

This project also defines `Test_Four` which checks the performance of the removal of elements. As this was not part of the test suite both implementations could be improved here. The `v2` takes about 4s to remove 10k elements while `v1` takes about 180s.
