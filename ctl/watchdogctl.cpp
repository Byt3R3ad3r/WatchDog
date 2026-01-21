#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <errno.h>
#include <limits>
#include <unistd.h>
#include <cctype>
#include <sstream>

//Prints picture seen
void print_ascii() {
    const char* paths[] = {
        "/usr/local/share/watchdog/Judge.txt",
        "./assets/Judge.txt"
    };

    for (const char* p : paths) {
        std::ifstream art(p);
        if (art) {
            std::cout << art.rdbuf() << "\n";
            return;
        }
    }
}

int main() {
    print_ascii();

    std::string path;
    std::string email;
    int count = 0;
    std::string count_str;
    std::string category;

    std::cout << "WatchDog V1.0\n";
    std::cout << "WatchDog setup\n";
    std::cout << "Canary Directory: ";
    std::getline(std::cin, path);
    
    while (true) {
    std::cout << "Number of decoys to create (Maximum 6): ";
    std::getline(std::cin, count_str);

    std::stringstream ss(count_str);
    if (ss >> count && ss.eof() && count >= 1 && count <= 6) {
        break;
    }

    std::cout << "Invalid\n";
    }

    while (true) {
    std::cout << "Type of decoy: ";
    std::cout << "Financial, Medical, Company, Personal:";
    std::getline(std::cin, category);

    for (char &c : category) c = std::tolower(c);
    if (category == "financial" ||
        category == "medical" ||
        category == "company" ||
        category == "personal") {
        break;
    }

    std::cout << "Invalid category\n";
    }

    std::cout << "Files will be saved in directory:" + path + "\n";

    std::cout << "Alert email: ";
    std::getline(std::cin, email);

    mkdir("/etc/watchdog", 0755);

    std::ofstream cfg("/etc/watchdog/watchdog.conf");

    if (!cfg) {
    perror("open");
    return 1;
    }

    //Input values to configuration file
    cfg << "[canary]\n";
    cfg << "path=" << path << "\n";
    cfg << "count=" << count << "\n";
    cfg << "category=" << category << "\n\n";

    cfg << "[alert]\nemail=" << email << "\n";
    cfg.close();

    std::cout << "Configuration written.\n";
    std::cout << "Run: sudo systemctl restart watchdog.service\n";
    return 0;
}