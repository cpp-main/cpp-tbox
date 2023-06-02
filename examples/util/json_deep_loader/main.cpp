#include <iostream>

#include <base/json.hpp>
#include <util/json_deep_loader.h>

using namespace std;

void PrintUsage(const char *proc) {
    cout << "Usage: " << proc << " your.json" << endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    const char *filename = argv[1];

    try {
        auto json = tbox::util::json::LoadDeeply(filename);
        cout << json.dump(2) << endl;
    } catch (const std::exception &e) {
        cerr << "Catch: " << e.what() << endl;
    }
    return 0;
}
