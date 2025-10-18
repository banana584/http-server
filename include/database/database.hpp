#ifndef DATABASE_DATABASE_HPP
#define DATABASE_DATABASE_HPP

#include <iostream>
#include <functional>
#include <fstream>
#include <bitset>
#include <deque>
#include <map>
#include <memory>
#include <string.h>
#include <cstdint>

/**
 * @namespace Caches
 * @brief Contains classes about caching.
 * @author banana584
 * @date 8/10/25
 */
namespace Caches {
    /**
     * @struct Data
     * @brief Data in a cache.
     * @author banana584
     * @date 11/10/25
     */
    template <typename T>
    struct Data {
        T data; ///< The actual data.
        int index; ///< The index in the cache.
        int usage; ///< The usage in the cache (amout of times accessed).

        /**
         * @brief Constructor.
         * @param data The data to use.
         * @param index The index in cache.
         * @author banana584
         * @date 11/10/25
         */
        Data(T data, int index) : data(data), index(index) {usage = 0;}

        /**
         * @brief Destructor.
         * @author banana584
         * @date 11/10/25
         */
        ~Data() {return;}

        /**
         * @brief Greater than comparison.
         * @param other The other Data struct.
         * @return A boolean on whether this has a higher usage than other.
         * @author banana584
         * @date 11/10/25
         */
        bool operator>(const Data& other) {return this->usage > other.usage;}

        /**
         * @brief Less than comparison.
         * @param other The other Data struct.
         * @return A boolean on whether this has a lower usage than other.
         * @author banana584
         * @date 11/10/25
         */
        bool operator<(const Data& other) {return this->usage < other.usage;}

        /**
         * @brief Equal to comparison.
         * @param other The other Data struct.
         * @return A boolean on whether this has the same usage as other.
         * @author banana584
         * @date 11/10/25
         */
        bool operator==(const Data& other) {return this->usage == other.usage;}
    };

    /**
     * @class LRU
     * @brief A Least Recently Used cache.
     * @author banana584
     * @date 8/10/25
     */
    template <typename T>
    class LRU {
        private:
            void* get_args;
            void* write_args;
        protected:
            std::deque<Data<T>> data; ///< All the data in cache - linked list since we might not have a contiguos part of memory to allocate.
            std::function<T(int,void*)> get; ///< Get data if not in cache.
            std::function<void(int,T,void*)> write; ///< Write data if expires in cache.
        public:
            size_t max_size; ///< The maximum size of the cache.
            uint min_usage;
        public:
            /**
             * @brief Constructor
             * @param max_size The maximum size of the cache.
             * @param get A function pointer for getting data from source.
             * @param get_args The other arguments to use when calling get.
             * @param write A function pointer for writing data to source.
             * @param write_args The other arguments to use when calling write.
             * @author banana584
             * @date 9/10/25
             */
            LRU(size_t max_size, uint min_usage, std::function<T(int,void*)> get, void* get_args, std::function<void(int,T,void*)> write, void* write_args);

            /**
             * @brief Constructor
             * @param max_size The maximum size of the cache.
             * @param get A function pointer for getting data from source.
             * @param write A function pointer for writing data to source.
             * @author banana584
             * @date 9/10/25
             */
            LRU(size_t max_size, uint min_usage, std::function<T(int,void*)> get, std::function<void(int,T,void*)> write);

            /**
             * @brief Destructor
             * @author banana584
             * @date 9/10/25
             */
            ~LRU();

            /**
             * @brief Prints all data in the cache.
             * @author banana584
             * @date 9/10/25
             */
            void print();

            /**
             * @brief Sets the arguments for get.
             * @param new_args The arguments for get.
             * @author banana584
             * @date 11/10/25
             */
            void GetArgs(void* new_args);

            /**
             * @brief Sets the arguments for write.
             * @param new_args The arguments for write.
             * @author banana584
             * @date 11/10/25
             */
            void WriteArgs(void* new_args);

            /**
             * @brief Gets data from the cache.
             * @param index The index of the data to recieve.
             * @return The data recieved.
             * @author banana584
             * @date 9/10/25
             */
            T Get(int index);

            /**
             * @brief Writes data to cache.
             * @author banana584
             * @date 9/10/25
             */
            void Write(int index, T data);

            /**
             * @brief Cleans data over max size in the cache.
             * @author banana584
             * @date 11/10/25
             */
            void Clean();
    };
}

/**
 * @namespace Databases
 * @brief Contains classes about databases.
 * @author banana584
 * @date 8/10/25
 */
namespace Databases {
    /**
     * @namespace KeyValues
     * @brief Contains classes and structs about key-value databases.
     * @author banana584
     * @date 11/10/25
     */
    namespace KeyValues {
        template <typename T> T get(int index, void* args);

        template <typename T> void write(int index, T data, void* args);
        
        /**
         * @class Headers
         * @brief Contains header data about a key-value database.
         * @author banana584
         * @date 12/10/25
         */
        class Headers {
            protected:
                uint64_t num_records; ///< The number of records in the key-value database.
                uint64_t record_size; ///< The size of each record in the key-value database.
            public:
                /**
                 * @brief Constructor
                 * @param num_records The number of records.
                 * @param record_size The size of each record.
                 * @author banana584
                 * @date 12/10/25
                 */
                Headers(uint64_t num_records, uint64_t record_size);

                /**
                 * @brief Constructor that parsed binary string.
                 * @param raw Raw binary string to parse as header.
                 * @author banana584
                 * @date 12/10/25
                 */
                Headers(std::string raw);

                /**
                 * @brief Destructor
                 * @author banana584
                 * @date 12/10/25
                 */
                ~Headers();
        };

        /**
        * @class KeyValue
        * @brief A key-value database.
        * @author banana584
        * @date 8/10/25
        */
        template <typename T>
        class KeyValue {
            private:
                Caches::LRU<std::map<std::string, T>> map; ///< In-memory representation.
                std::fstream file; ///< File to read from and write to.
            public:
                std::string filename; ///< The name of the file used.
            private:

            public:
                /**
                * @brief Constructor
                * @param filename The name of the file to use as the database.
                * @param save_on_exit A flag on if the database should be saved when destroyed.
                * @author banana584
                * @date 8/10/25
                */
                KeyValue(std::string filename, bool save_on_exit);

                /**
                * @brief Destructor
                * @author banana584
                * @date 8/10/25
                */
                ~KeyValue();

                /**
                * @brief Loads from the file.
                * @return a boolean on whether the database was loaded correctly.
                * @author banana584
                * @date 8/10/25
                */
                bool Load();

                /**
                * @brief Saves to the file.
                * @return A boolean on whether the database was saved correctly.
                * @author banana584
                * @date 8/10/25
                */
                bool Save();

                /**
                * @brief Reads from the database.
                * @param key The key to use.
                * @return The data found.
                * @author banana584
                * @date 8/10/25
                */
                T Read(std::string key);

                /**
                * @brief Writes to the database.
                * @param key The key to use.
                * @param data The data to write.
                * @return A boolean on whether the database was written to correctly.
                * @author banana584
                * @date 8/10/25
                */
                bool Write(std::string key, T& data);
        };
    }
}

#endif