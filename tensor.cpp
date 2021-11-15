#include "tensor.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>

void Tensor::ReadFromRawFile(const std::string &file_name, const std::vector<int> &shape, FileDataType format) {
    if (shape.empty()) {
        throw std::invalid_argument("empty shape is not allowed");
    }
    shape_ = shape;
    int num_elements = 1;
    for (auto x: shape) {
        assert (x > 0);
        num_elements *= x;
    }
    data_.resize(num_elements);
    std::ifstream ifs(file_name, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file " << file_name << std::endl;
        throw CannotOpenFileError();
    }
    int total_count = 1;
    for (auto x: shape) total_count *= x;
    int total_bytes_count = 0;
    switch (format) {
        case FileDataType::kInt16:
        case FileDataType::kUInt16: {
            total_bytes_count = total_count * 2;
            break;
        }
        case FileDataType::kInt32:
        case FileDataType::kUInt32:
        case FileDataType::kFloat32: {
            total_bytes_count = total_count * 4;
            break;
        }
        case FileDataType::kInt64:
        case FileDataType::kUInt64:
        case FileDataType::kFloat64: {
            total_bytes_count = total_count * 8;
            break;
        }
        default:
            break;
    }
    std::vector<char> buffer(total_bytes_count);
    ifs.read(buffer.data(), total_bytes_count);
    if (!ifs) {
        std::cerr << "Failed to read " << total_bytes_count << " bytes from file" << file_name << std::endl;
        throw FileLengthError();
    }
    switch (format) {
        case FileDataType::kFloat32: {
            using Type = float;
            for (int i = 0; i < total_bytes_count; i += sizeof(Type)) {
                data_[i / sizeof(Type)] = static_cast<double>(*reinterpret_cast<Type *>(buffer.data() + i));
            }
            break;
        }
        case FileDataType::kFloat64: {
            using Type = double;
            for (int i = 0; i < total_bytes_count; i += sizeof(Type)) {
                data_[i / sizeof(Type)] = static_cast<double>(*reinterpret_cast<Type *>(buffer.data() + i));
            }
            break;
        }
        default:
            throw std::invalid_argument("not implemented format");
    }
}


class TensorIndexIterator {
public:
    TensorIndexIterator(const std::vector<int> &shape) : shape_(shape), index_(shape.size(), 0) {
        num_elements_ = 1;
        for (auto x: shape_) {
            num_elements_ *= x;
        }
    }

    void Next() {
        if (end_) {
            std::cerr << "TensorIndexIterator has ran out\n";
            exit(-1);
        }
        for (size_t i = 0; i < shape_.size(); ++i) {
            if (index_[i] < shape_[i] - 1) {
                index_[i]++;
                return;
            } else {
                index_[i] = 0;
            }
        }
        end_ = true;
    }

    const std::vector<int> &index() const {
        return index_;
    }

    bool end() const { return end_; }

private:
    std::vector<int> shape_;
    std::vector<int> index_;
    bool end_ = false;
    int num_elements_ = 1;
};

Tensor Tensor::Permute(const std::vector<int> &p) const {
    assert (p.size() == shape_.size());
    Tensor result(shape_);
    std::vector<int> p_index(p.size());
    for (size_t i = 0; i < shape_.size(); ++i) {
        result.shape_[i] = shape_[p[i]];
    }
    TensorIndexIterator iterator(shape_);
    while (!iterator.end()) {
        const auto &index = iterator.index();
        for (size_t i = 0; i < shape_.size(); ++i) {
            p_index[i] = index[p[i]];
        }
        result[p_index] = (*this)[index];
        iterator.Next();
    }
    return result;
}

std::string Tensor::ToString() const {
    std::stringstream ss;
    TensorIndexIterator iterator(shape_);
    while (!iterator.end()) {
        const auto &index = iterator.index();
        ss << "(";
        for (auto x: index) {
            ss << x << ", ";
        }
        ss << "): " << (*this)[index] << "\n";
        iterator.Next();
    }
    return ss.str();
}
