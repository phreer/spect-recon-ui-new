//
// Created by Phree on 2021/10/29.
//

#ifndef TEST_TENSORINDEXITERATOR_TENSOR_H
#define TEST_TENSORINDEXITERATOR_TENSOR_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>

class Tensor {
public:
    class CannotOpenFileError: std::logic_error {
    public:
        CannotOpenFileError(): std::logic_error("file not found") {}
    };
    class FileLengthError: std::logic_error {
    public:
        FileLengthError(): std::logic_error("file length error") {}
    };
    class InconsistentShapeError: std::logic_error {
    public:
        InconsistentShapeError(): std::logic_error("inconsistent shape") {}
    };

    typedef double DataType;

    Tensor() {}

    static void CheckConsistency(const std::vector<int>& shape, int size) {
        int num_elements = 1;
        for (auto s: shape) num_elements *= s;
        if (num_elements < 0) {
            std::cerr << "Invalid shape (";
            for (auto s: shape) std::cerr << s << ", ";
            std::cerr << ")\n";
            throw std::invalid_argument("invalid shape");
        }
        if (num_elements != size) {
            std::cerr << "Inconsistent shape.\n";
            throw InconsistentShapeError();
        }
    }
    Tensor(const std::vector<int> &shape) : shape_(shape) {
        int num_elements = 1;
        for (auto s: shape) num_elements *= s;
        data_.resize(num_elements);
    }

    Tensor(const std::vector<int> &shape, const std::vector<DataType> &data) : shape_(shape){
        CheckConsistency(shape, data.size());
        data_ = data;
    }

    Tensor(const std::vector<int> &shape, std::vector<DataType> &&data): shape_(shape) {
        CheckConsistency(shape, data.size());
        data_ = move(data);
    }

    const std::vector<int> &shape() const { return shape_; }

    std::vector<int> shape() { return shape_; }
    void Set(const std::vector<int>& shape, const std::vector<DataType>& data) {
        CheckConsistency(shape, data.size());
        shape_ = shape;
        data_ = data;
    }
    void Set(const std::vector<int>& shape, std::vector<DataType>&& data) {
        CheckConsistency(shape, data.size());
        shape_ = shape;
        data_ = std::move(data);
    }
    enum class FileDataType {
        kFloat32,
        kFloat64,
        kUInt8,
        kInt8,
        kUInt16,
        kInt16,
        kUInt32,
        kInt32,
        kUInt64,
        kInt64,
    };

    void ReadFromRawFile(const std::string &file_name, const std::vector<int> &shape, FileDataType format);

    static Tensor
    CreateTensorFromRawFile(const std::string &file_name, const std::vector<int> &shape, FileDataType format) {
        Tensor result;
        result.ReadFromRawFile(file_name, shape, format);
        return result;
    }

    DataType &operator[](const std::vector<int> &index) {
        return data_[GetIndex_(index)];
    }

    const DataType &operator[](const std::vector<int> &index) const {
        return data_[GetIndex_(index)];
    }

    Tensor Permute(const std::vector<int> &p) const;
    std::string ToString() const;
    const std::vector<DataType>& data() const {
        return data_;
    }
    std::vector<DataType>& data() {
        return data_;
    }
    DataType GetMaximum() const {
        return *std::max_element(data_.cbegin(), data_.cend());
    }
    DataType GetMinimum() const {
        return *std::min_element(data_.cbegin(), data_.cend());
    }
    DataType GetSum() const {
        return std::accumulate(data_.cbegin(), data_.cend(), DataType(0));
    }
    void NormalizeInPlace() {
        DataType min = GetMinimum();
        for (auto& x: data_) {
            x -= min;
        }
        DataType max = GetMaximum();
        if (max < 1e-8) return;
        for (auto& x: data_) {
            x /= max;
        }
    }
private:
    void CheckIndex_(const std::vector<int> &index) const {
        assert (index.size() == shape_.size());
        for (size_t i = 0; i < shape_.size(); ++i) {
            assert (index[i] < shape_[i]);
            assert (index[i] >= 0);
        }
    }

    size_t GetIndex_(const std::vector<int> &index) const {
#ifdef DEBUG
        CheckIndex_();
#endif
        int result = 0;
        int curr = data_.size();
        for (size_t i = 0; i < index.size(); ++i) {
            curr /= shape_[i];
            result += index[i] * curr;
        }
        return result;
    }

    std::vector<int> ToIndex(size_t index) {
        std::vector<int> result(shape_.size());
        int curr = data_.size();
        for (size_t i = 0; i < shape_.size(); ++i) {
            curr /= shape_[i];
            result[i] = index / curr;
            index = index % curr;
        }
        return result;
    }

    std::vector<int> shape_;
    std::vector<DataType> data_;
};


#endif //TEST_TENSORINDEXITERATOR_TENSOR_H
