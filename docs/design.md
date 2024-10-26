# Gemstore Design

## Three main outputs:

* Server _(C++)_

* Small client/Test Programs _(C++)_

* Small web client _(Javascript)_

* Smaller output?: *libgemstore* - Used by all the programs above

## Two Main Components

* Store System (Memory, Disk): **Caching, other stuff**
  * Data Structures: Int, String, Float, Array[Generic]
  * Data Structures (Advanced): References, Structs
  * **Who will do this?:**
    * Sachin: Data Structures, Caching
    * Both: Concurrency handling, public interfaces
  * `store.cpp store.hpp`
  * ? `fs.cpp cache.cpp`

* Networking:
  * Interfacing with Client (TCP Connection)
  * Interfacing with Other Servers (TCP Connection)
  * Interfacing with HTTP Clients? Unify Both?


## Dependencies

* json `https://github.com/nlohmann/json`
  - Will pass config for servers using this
  - json responses to clients

* cpp-httplib `https://github.com/yhirose/cpp-httplib`
  - for the client-side REST api

## Code hygeine

* **CONFIGURE** LSP for C++ if not done already
  * `sudo apt install bear`
  * `brew install bear`
  * `DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

* 4 Space Indent

* `snake_case` for variables and functions

* `PascalCase` for types and classes

* 80 Columns Soft limit


* Optional: Configure `llvm_format`

* DO NOT use `class`es, use `struct`s
  * Prefix private members (if needed) with `_`

* Keep everything under namespace `gem`

* **DO NOT** use:
  ```
  std::map<std::pair<int, std::string s>, std::vector<int>> k
  ```

  Use:
  ```
  {
  	/* This is just an example */

  	using Token = std::pair<int, std::string s>;
  	using NumberList = std::vector<int>;

  	template <typename K, typename V>
  	using Map = std::map<K, V>;

  	using TokenVectorMap = Map<Token, NumberList>;

  	TokenVectorMap m;
  }
  ```
