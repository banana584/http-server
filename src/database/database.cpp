#include "../../include/database/database.hpp"

template <typename T>
Caches::LRU<T>::LRU(size_t max_size, uint min_usage, std::function<T(int,void*)> get, void* get_args, std::function<void(int,T,void*)> write, void* write_args) {
    // Copy values into class.
    this->data = std::deque<Data>();
    this->data.resize(max_size);
    this->get = get;
    this->get_args = new void*(*get_args);
    this->write = write;
    this->write_args = new void*(*write_args);
    this->max_size = max_size;
    this->min_usage = min_usage;
}

template <typename T>
Caches::LRU<T>::LRU(size_t max_size, uint min_usage, std::function<T(int,void*)> get, std::function<void(int,T,void*)> write) {
    // Copy values into class.
    this->data = std::deque<Data>();
    this->get = get;
    this->get_args = nullptr;
    this->write = write;
    this->write_args = nullptr;
    this->max_size = max_size;
    this->min_usage = min_usage;
}

template <typename T>
Caches::LRU<T>::~LRU() {
    if (this->get_args) {
        delete this->get_args;
    }
    if (this->write_args) {
        delete this->write_args;
    }
    return;
}

template <typename T>
void Caches::LRU<T>::print() {
    // Loop over each item and print
    for (auto& item : data) {
        std::cout << item.data << " ";
    }
    std::cout << "\n";
}

template <typename T>
void Caches::LRU<T>::GetArgs(void* new_args) {
    if (this->get_args) {
        delete this->get_args;
    }
    this->get_args = new void*(*new_args);
}

template <typename T>
void Caches::LRU<T>::WriteArgs(void* new_args) {
    if (this->write_args) {
        delete this->write_args;
    }
    this->write_args = new void*(*new_args);
}

template <typename T>
T Caches::LRU<T>::Get(int index) {
    // Check if item is in cache.
    if (index < 0 || index >= data.size()) {
        // If not, create a new data struct.
        Caches::Data new_data<T>(get(index, get_args), index, 1);
        // Check if we are over the max size.
        if (data.size() > max_size) {
            // If we are, remove the last element.
            data.pop_back();
        }
        // Add the data to queue.
        data.push_back(new_data);
        return new_data.data;
    }
    // Increment usage.
    data[index].usage++;

    // Sort Based on usage.
    std::sort(data.begin(), data.end());

    return data[index].data;
}

template <typename T>
void Caches::LRU<T>::Write(int index, T data) {
    // Check if item is in cache.
    if (index < 0 || index >= data.size()) {
        // If not, create a new data struct.
        Caches::Data new_data<T>(data, index, 1);
        // Check if we are over the max size.
        if (data.size() > max_size) {
            // If we are, remove the last element.
            data.pop_back();
        }
        // Add the data to queue.
        data.insert(data.begin() + index, data);
    } else {
        // Increment useage.
        data[index].usage++;
    }

    // Write data.
    write(index, data, write_args);
}

template <typename T>
void Caches::LRU<T>::Clean() {
    // Erase all data that is above max size or below min usage.
    data.erase(std::remove_if(data.begin(), data.end(), [&](const Data& d) {
        return d.index > max_size || d.usage < min_usage;
    }), data.end());
}

Databases::KeyValues::Headers(uint64_t num_records, uint64_t record_size) {
    this->num_records = num_records;
    this->record_size = record_size;
}

uint64_t string_to_utint_64(const std::string& str) {
    uint64_t result = 0;
    for (char c : str) {
        result = result * 10 + (c - '0');
        if (result > std::numeric_limits<uint64_t>::max()) {
            throw std::out_of_range("String value exceeds uint64_t range");
        }
    }
    return result;
}

Databases::KeyValues::Headers(std::string raw) {
    std::string s_raw;
    for (int i = 0; i < raw.length(); i += 2) {
        std::string hex_digits = raw.substr(i, 2);
        char character = static_cast<char>(std::stoi(hex_digits, nullptr, 16));
        s_raw += character;
    }

    this->num_records = string_to_uint_64(s_raw.substr(0, 64));
    this->record_size = string_to_uint_64(s_raw.substr(64));
}

Databases::KeyValues::~Headers() {
    return;
}

template <typename T>
T Databases::KeyValues::get(int index, void* args) {
    KeyValue<T>* db = (KeyValue<T>*)args;

    char header_size_buffer[1024];
    db->file.read(header_size_buffer, 32);
    uint32_t header_size = *reinterpret_cast<uint32_t*>(header_size_buffer);
    
    char* header_buffer = new char[header_size + 1];
    
    db->file.read(header_buffer, header_size);
    std::string headers_txt(header_buffer);

    delete[] header_buffer;

    Headers headers(headers_txt);

    if (index < 0 || index > headers.num_records) {
        return T();
    }

    size_t position = index * headers.record_size;

    fseek(db->file, SEEK_SET, position);

    char* data_buffer = new char[headers.record_size + 1];

    db->file.read(data_buffer, headers.record_size);

    std::string data(data_buffer);

    delete[] data_buffer;

    return T(data);
}

template <typename T>
void Databases::KeyValues::write(int index, T data, void* args) {
    KeyValue<T>* db = (KeyValue<T>*)args;

    char header_size_buffer[1024];
    db->file.read(header_size_buffer, 32);
    uint32_t header_size = *reinterpret_cast<uint32_t*>(header_size_buffer);
    
    char* header_buffer = new char[header_size + 1];
    
    db->file.read(header_buffer, header_size);
    std::string headers_txt(header_buffer);

    delete[] header_buffer;

    Headers headers(headers_txt);

    if (index < 0 || index > headers.num_records) {
        return;
    }

    size_t position = index * headers.record_size;

    fseek(db->file, SEEK_SET, position);

    db->file.write(data.to_string(), sizeof(data));

    std::vector<char> buffer;
    std::istreambuf_iterator<char> begin(db->file);
    std::istreambuf_iterator<char> end;
    std::copy_n(begin, 64, std::back_inserter(buffer));

    buffer.erase(0, 64);
    buffer.insert(buffer.end(), headers.num_records + 1, headers.num_records + 1 + sizeof(headers.num_records));

    fseek(db->file, SEEK_SET, 0);

    db->file.write(buffer.data(), buffer.size());
}