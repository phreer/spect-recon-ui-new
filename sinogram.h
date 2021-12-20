#ifndef SINOGRAM_H
#define SINOGRAM_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

template<typename T>
std::vector<T> ReadArray(const std::string &path, size_t num_elements)
{
    std::ifstream fs(path, std::ios::binary);
    if (!fs.is_open()) {
        std::cerr << "Cannnot open file " << path << " to read." << std::endl;
        exit(-1);
    }

    std::vector<T> result(num_elements);
    fs.read((char*) result.data(), num_elements * sizeof(T));
    if (!fs) {
        std::cerr << "Only " << fs.gcount() << " counld be read, but "
            << num_elements << "is required." << std::endl;
        exit(-1);
    }
    return result;
}

template<typename T>
void WriteArray(const std::string &path, T* data, size_t count)
{
    std::ofstream fs(path, std::ios::binary);
    if (!fs.is_open()) {
        std::cerr << "Cannnot open file " << path << " to write." << std::endl;
        exit(-1);
    }
    fs.write((char*)data, count * sizeof(T));
}

template <typename T>
class Sinogram
{
public:
    Sinogram(size_t num_slices = 0, size_t num_angles = 0, size_t num_detectors = 0):
        num_slices_(num_slices), num_angles_(num_angles),
        num_detectors_(num_detectors), vec_(num_slices * num_angles * num_detectors)
    {}
    Sinogram(const std::vector<T>& vec, size_t num_slices, size_t num_angles, size_t num_detectors):
        vec_(vec), num_slices_(num_slices), num_angles_(num_angles),
        num_detectors_(num_detectors)
    {
        assert(num_slices * num_angles * num_detectors == vec.size());
    }
    Sinogram(const std::vector<T>&& vec, size_t num_slices, size_t num_angles, size_t num_detectors):
        vec_(std::move(vec)), num_slices_(num_slices), num_angles_(num_angles),
        num_detectors_(num_detectors)
    {
        assert(num_slices * num_angles * num_detectors == vec.size());
    }
    Sinogram(const std::string &path, size_t num_slices, size_t num_angles,
             size_t num_detectors, bool normalize=true):
        num_slices_(num_slices), num_angles_(num_angles), num_detectors_(num_detectors)
    {
        ReadFromFilePath(path, num_slices, num_angles, num_detectors, normalize);
    }

    T* GetData() { return vec_.data(); }
    const T* GetData() const { return vec_.data(); }
    size_t GetNumSlices() const { return num_slices_; }
    size_t GetNumAngles() const { return num_angles_; }
    size_t GetNumDetectors() const { return num_detectors_; }
    size_t GetNumElements() const
    {
        return GetNumSlices() * GetNumAngles() * GetNumDetectors();
    }

    void ReadFromFilePath(const std::string &path, size_t num_slices,
                          size_t num_angles, size_t num_detectors, bool normalize)
    {
        const size_t kNumElements = num_slices * num_angles * num_detectors;
        vec_ = ReadArray<T>(path, kNumElements);
        T m = *std::min_element(vec_.cbegin(), vec_.cend());

        if (normalize) {
            for (size_t i = 0; i < kNumElements; ++i) {
                vec_[i] -= m;
            }
            double s = 0.;
            for (auto x : vec_) {
                s += x;
            }
            for (auto &x: vec_) {
                x = x * kNumElements / s;
            }
        }
    }

    void WriteToFilePath(const std::string &path)
    {
        WriteArray(path, vec_.data(), vec_.size());
    }

    template<typename OtherType>
    Sinogram<OtherType> TransformType() const
    {
        std::vector<OtherType> data(vec_.size());
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<OtherType>(vec_[i]);
        }
        return Sinogram<OtherType>(std::move(data), num_slices_, num_angles_, num_detectors_);
    }

    void L1NormalizeInPlace()
    {
        int num_elements = num_slices_ * num_angles_ * num_detectors_;
        double norm = std::accumulate(vec_.cbegin(), vec_.cend(), 0.) / static_cast<double>(num_elements);
        for (auto& x: vec_) {
            x /= norm;
        }
    }
private:
    size_t num_slices_;
    size_t num_angles_;
    size_t num_detectors_;
    std::vector<T> vec_;
};


#endif // SINOGRAM_H
