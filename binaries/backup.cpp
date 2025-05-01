#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <ctime>
#include <cstdlib>
#include <libgen.h>

using namespace std;

bool contains_dangerous_chars(const string &input) {
    // Check for dangerous characters like ; & | `
    return (input.find_first_of(";&|`") != string::npos);
}

bool is_valid_path(const string &path) {
    // Check if the path starts with /var/log/
    return (path.substr(0, 8) == "/var/log");
}

bool file_exists(const string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

string get_current_time() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", ltm);
    return string(buffer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "No file path provided!" << endl;
        return 1;
    }

    string file_path = argv[1];

    if (!file_exists(file_path)) {
        cerr << "File not found or invalid input!" << endl;
        return 1;
    }

    if (contains_dangerous_chars(file_path)) {
        cerr << "Dangerous characters detected in input!" << endl;
        return 1;
    }

    if (file_path.find("../") != string::npos) {
        cerr << "Invalid characters detected!" << endl;
        return 1;
    }

    if (!is_valid_path(file_path)) {
        cerr << "This script can only be run on files in the /var/log/ directory!" << endl;
        return 1;
    }

    // Create the backup directory if it doesn't exist
    string backup_dir = "/backup/";
    mkdir(backup_dir.c_str(), 0755); // use mkdir instead of system()

    // Get basename of the file for backup naming
    char *path_dup = strdup(file_path.c_str());
    string backup_file = backup_dir + string(basename(path_dup)) + "_backup_" + get_current_time();
    free(path_dup);

    ifstream src(file_path, ios::binary);
    ofstream dst(backup_file, ios::binary);

    if (!src || !dst) {
        cerr << "Backup failed!" << endl;
        return 1;
    }

    dst << src.rdbuf();

    cout << "Backup successful: " << backup_file << endl;
    return 0;
}
