//
//  main.cpp
//  Project4
//
//  Created by 이동건 on 4/29/25.
//

#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

// 공통 인터페이스
class FilesystemComponent {
protected:
    std::string name;
public:
    FilesystemComponent(const std::string& name) : name(name) {}
    virtual ~FilesystemComponent() = default;
    virtual void display(int depth = 0) const = 0;
    virtual uintmax_t getSize() const = 0;
};

// 파일 클래스
class File : public FilesystemComponent {
    uintmax_t size;
public:
    File(const std::string& name, uintmax_t size) : FilesystemComponent(name), size(size) {}

    void display(int depth = 0) const override {
        std::cout << std::string(depth * 2, ' ') << "[File] " << name << " (" << size << " bytes)\n";
    }

    uintmax_t getSize() const override {
        return size;
    }
};

// 디렉토리 클래스
class Directory : public FilesystemComponent {
    std::vector<std::shared_ptr<FilesystemComponent>> children;
public:
    Directory(const std::string& name) : FilesystemComponent(name) {}

    void add(std::shared_ptr<FilesystemComponent> component) {
        children.push_back(component);
    }

    void display(int depth = 0) const override {
        uintmax_t totalSize = 0;
        for (const auto& child : children) {
            totalSize += child->getSize();
        }
        std::cout << std::string(depth * 2, ' ') << "[Directory] " << name << " (" << totalSize << " bytes)\n";
        for (const auto& child : children) {
            child->display(depth + 1);
        }
    }

    uintmax_t getSize() const override {
        uintmax_t totalSize = 0;
        for (const auto& child : children) {
            totalSize += child->getSize();
        }
        return totalSize;
    }
};

int main() {
    fs::path currentPath = fs::current_path();
    auto root = std::make_shared<Directory>(currentPath.filename().string());

    std::function<void(const fs::path&, std::shared_ptr<Directory>)> traverse;
    traverse = [&](const fs::path& path, std::shared_ptr<Directory> currentDir) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                auto file = std::make_shared<File>(entry.path().filename().string(), entry.file_size());
                currentDir->add(file);
            } else if (entry.is_directory()) {
                auto dir = std::make_shared<Directory>(entry.path().filename().string());
                currentDir->add(dir);
                traverse(entry.path(), dir); // 재귀
            }
        }
    };

    traverse(currentPath, root);
    root->display();

    return 0;
}
