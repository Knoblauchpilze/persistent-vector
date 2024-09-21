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
