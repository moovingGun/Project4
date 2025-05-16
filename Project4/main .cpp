//
//  main.cpp
//  Project3
//
//  Created by 이동건 on 4/29/25.
//

#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;


class FilesystemComponent {
protected:
    std::string name;
public:
    FilesystemComponent(const std::string& name) : name(name) {}
    virtual ~FilesystemComponent() = default;
    virtual void display(int depth = 0) const = 0;
    virtual uintmax_t getSize() const = 0;
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
};


class File : public FilesystemComponent {
    uintmax_t size;
public:
    File(const std::string& name = "", uintmax_t size = 0) : FilesystemComponent(name), size(size) {}

    void display(int depth = 0) const override {
        std::cout << std::string(depth * 2, ' ') << "[File] " << name << " (" << size << " bytes)\n";
    }

    uintmax_t getSize() const override {
        return size;
    }

    std::string serialize() const override {
        return "FILE," + name + "," + std::to_string(size);
    }

    void deserialize(const std::string& data) override {
        std::istringstream iss(data);
        std::string type;
        getline(iss, type, ',');
        getline(iss, name, ',');
        std::string sizeStr;
        getline(iss, sizeStr);
        size = std::stoull(sizeStr);
    }
};


class Directory : public FilesystemComponent {
    std::vector<std::shared_ptr<FilesystemComponent>> children;
public:
    Directory(const std::string& name = "") : FilesystemComponent(name) {}

    void add(std::shared_ptr<FilesystemComponent> component) {
        children.push_back(component);
    }

    void display(int depth = 0) const override {
        uintmax_t totalSize = getSize();
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

    std::string serialize() const override {
        std::ostringstream oss;
        oss << "DIR," << name << "\n";
        for (const auto& child : children) {
            oss << child->serialize() << "\n";
        }
        oss << "ENDDIR";
        return oss.str();
    }

    void deserialize(const std::string& data) override {
        std::istringstream iss(data);
        std::string line;
        getline(iss, line);
        name = line.substr(4); 

        while (getline(iss, line)) {
            if (line == "ENDDIR") break;
            if (line.rfind("FILE,", 0) == 0) {
                auto file = std::make_shared<File>();
                file->deserialize(line);
                add(file);
            } else if (line.rfind("DIR,", 0) == 0) {
                std::ostringstream subdir_data;
                subdir_data << line << "\n";
                int depth = 1;
                while (getline(iss, line)) {
                    if (line == "ENDDIR") {
                        subdir_data << line;
                        break;
                    }
                    subdir_data << line << "\n";
                }
                auto dir = std::make_shared<Directory>();
                dir->deserialize(subdir_data.str());
                add(dir);
            }
        }
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
                traverse(entry.path(), dir);
            }
        }
    };

    traverse(currentPath, root);

    std::cout << "Original Structure:\n";
    root->display();

    std::string data = root->serialize();
    std::cout << "\nSerialized Data:\n" << data << "\n";

    auto cloned = std::make_shared<Directory>();
    cloned->deserialize(data);

    std::cout << "\nDeserialized Structure:\n";
    cloned->display();

    return 0;
}
