#pragma once

#include "mesh/generated/meshtastic/mesh.pb.h"
#include <cstdlib>
#include <limits>
#include <utility>

class FileManifest
{
  public:
    using Reallocate = void *(*)(void *, size_t);

    // The allocator must follow realloc() ownership and return memory compatible with free().
    explicit FileManifest(Reallocate reallocate = std::realloc) : reallocate(reallocate) {}
    ~FileManifest() { clear(); }
    FileManifest(const FileManifest &) = delete;
    FileManifest &operator=(const FileManifest &) = delete;
    FileManifest(FileManifest &&other) noexcept
        : reallocate(other.reallocate), entries(std::exchange(other.entries, nullptr)), count(std::exchange(other.count, 0))
    {
    }
    FileManifest &operator=(FileManifest &&other) noexcept
    {
        if (this != &other) {
            clear();
            reallocate = other.reallocate;
            entries = std::exchange(other.entries, nullptr);
            count = std::exchange(other.count, 0);
        }
        return *this;
    }

    bool append(const meshtastic_FileInfo &info)
    {
        if (count >= (std::numeric_limits<size_t>::max)() / sizeof(*entries))
            return false;
        const auto value = info;
        auto *resized = static_cast<meshtastic_FileInfo *>(reallocate(entries, (count + 1) * sizeof(*entries)));
        if (!resized)
            return false;
        entries = resized;
        entries[count++] = value;
        return true;
    }

    void clear()
    {
        std::free(entries);
        entries = nullptr;
        count = 0;
    }

    size_t size() const { return count; }
    const meshtastic_FileInfo *begin() const { return entries; }
    const meshtastic_FileInfo *end() const { return count ? entries + count : entries; }
    const meshtastic_FileInfo &operator[](size_t index) const { return entries[index]; }

  private:
    Reallocate reallocate;
    meshtastic_FileInfo *entries = nullptr;
    size_t count = 0;
};
