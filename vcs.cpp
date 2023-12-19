#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <functional>
#include <vector>

class Vcs {
public:
    Vcs(std::string path) : path_(path) {}
    // This functions initializes the VCS 
    void initiate() {
        // If the main directory doesn't exist, create it
        if (!std::filesystem::exists(path_)) {
            std::filesystem::create_directory(path_);
        }
        // Create files and directories 
        std::filesystem::create_directory(path_ + "\\.backup");
        std::ofstream file(path_ + "\\.commits");
        file.close();
        std::cout << "initiated VCS repository at " << path_ << std::endl;
    }

    // File tracking section 
    void trackFile(std::string filename) {
        // Check if the file exists before tracking
        std::filesystem::path filepath = path_;
        filepath /= filename;
        if (std::filesystem::exists(filepath)) {
            tracked_files_[filename] = "untracked";
            std::cout << "File tracked: " << filepath.filename().string() << std::endl;
        } else {
            std::cerr << "Error: File does not exist - " << filename << std::endl;
            createFile(filepath);
        }
    }

    // Show all the tracked files 
    void show_trackedfl() {
        for (auto& entry : tracked_files_) {
            std::cout << "File: " << entry.first << " - Status: " << entry.second << std::endl;
        }
    }

    // Commit changes 
void commit() {
    // Backup each tracked file
    for (auto& entry : tracked_files_) {
        std::filesystem::path filepath = path_;
        filepath /= entry.first;

        std::filesystem::path backup_filepath = path_;
        backup_filepath /= ".backup";
        backup_filepath /= entry.first;

        std::filesystem::copy_file(filepath, backup_filepath, std::filesystem::copy_options::overwrite_existing);

        // Calculate the hash and store it
        entry.second = calculateHash(filepath);
    }

    // Write commit details to the file
    std::ofstream commits_file(path_ + "\\.commits", std::ios_base::app);
    if (!commits_file) {
        std::cerr << "Error: Can not open commit file for editing." << std::endl;
        return;
    }

    commits_file << std::endl;
    commits_file.close();

    std::cout << "Committed changes." << std::endl;

    // Reset the status of all files to "unchanged" after the commit
    for (auto& entry : tracked_files_) {
        entry.second = "Not changed";
    }
}


    // Revert files 
    void revert() {
        std::filesystem::path backup_directory = path_;
        backup_directory /= ".backup";

        if (!std::filesystem::exists(backup_directory)) {
            std::cerr << "Error: Revert can't happen because there is no backup" << std::endl;
            return;
        }

        for (auto& entry : tracked_files_) {
            std::filesystem::path filepath = path_;
            filepath /= entry.first;

            std::filesystem::path backup_filepath = backup_directory;
            backup_filepath /= entry.first;

            if (!std::filesystem::exists(backup_filepath)) {
                std::cerr << "Error: No backup files could be found: " << entry.first << std::endl;
                continue;
            }

            // Recover the content from backup
            std::ifstream backup_file(backup_filepath.string());
            std::string content;
            content.assign(std::istreambuf_iterator<char>(backup_file), std::istreambuf_iterator<char>());
            backup_file.close();

            std::ofstream file(filepath.string());
            if (!file) {
                std::cerr << "Error: Can't open for editing: " << entry.first << std::endl;
                continue;
            }

            file << content;
            file.close();

            std::cout << "Reverted file: " << entry.first << std::endl;
            entry.second = std::to_string(std::hash<std::string>{}(content));
        }
    }

    void integrityCheck();

    // New public member function to get tracked files
    const std::map<std::string, std::string>& getTrackedFiles() const {
        return tracked_files_;
    }

private:
    std::map<std::string, std::string> tracked_files_;
    std::string path_;

    std::string readFile(const std::filesystem::path& filepath) {
        std::ifstream file(filepath.string(), std::ios::binary);
        if (!file) {
            std::cerr << "Error: Can't open file to examine: " << filepath << std::endl;
            return "";
        }

        std::ostringstream contentStream;
        contentStream << file.rdbuf();
        return contentStream.str();
    }

    std::string calculateHash(const std::filesystem::path& filepath) {
        return std::to_string(std::hash<std::string>{}(readFile(filepath)));
    }

    void createFile(const std::filesystem::path& filepath) {
        std::cout << "Creating file: " << filepath.filename().string() << std::endl;

        // Create the file
        std::ofstream file(filepath.string());
        if (!file) {
            std::cerr << "Error: Could not create file - " << filepath.filename().string() << std::endl;
            return;
        }
        file.close();

        // Track new created files
        tracked_files_[filepath.filename().string()] = "untracked";
        std::cout << "File tracked: " << filepath.filename().string() << std::endl;
    }
};

void Vcs::integrityCheck() {
    std::cout << "Currently perfoming integrity check..." << std::endl;

    for (const auto& entry : tracked_files_) {
        std::filesystem::path filepath = path_;
        filepath /= entry.first;

        // Calculate the current hash of the file
        std::string currentHash = std::to_string(std::hash<std::string>{}(readFile(filepath)));

        // Compare with the stored hash in tracked files created previously 
        if (currentHash != entry.second) {
            std::cerr << "Error: Integrity check failed for file: " << entry.first << std::endl;
        }
    }

    std::cout << "Integrity check completed." << std::endl;
}

int main() {
    std::string path = "C:\\Users\\tanam\\OneDrive\\Desktop\\Advanced\\final\\test5";
    Vcs vcs(path);
    vcs.initiate();

    // Tracking
    vcs.trackFile("test1.txt");  
    vcs.trackFile("test2.txt");
    vcs.trackFile("test3.txt");
    vcs.trackFile("test4.txt");
    vcs.show_trackedfl();

    vcs.commit();
    // Print hash values
    std::cout << "Hash values after commit:" << std::endl;
    for (const auto& entry : vcs.getTrackedFiles()) {
        std::cout << "File: " << entry.first << " - Hash: " << entry.second << std::endl;
    }

    vcs.revert();

    // Perform integrity check after committing and reverting
    vcs.integrityCheck();

    return 0;
}
