#include <iostream>
#include <string>
#include <sstream>

#ifdef WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

int main() {
    std::ios::sync_with_stdio(false);
    cerr.tie(nullptr);
    cerr << std::nounitbuf;

    std::string s;
    std::stringstream ss;

    int frame;

    // Read initial map and do your work.
    std::getline(cin, s, '\0');
    ss << s;

    // Read total game frame from ss.
    // Here is a placeholder
    frame = 1000000;

    usleep(3000000);
    ss.clear();

    for (int i = 0; i < frame ; i++) {
        std::getline(cin, s, '\0');
        ss << s;
        ss.clear();

        if (cin.rdbuf()->in_avail() > 0) {
            cerr << "Warning: skipping frame " << i << " to catch up with the game" << std::endl;
            continue;
        }

        cerr << "Handling frame " << i << std::endl;

        cout << "Frame " << i << "\n";
        cout << "Move R\n";
        cout << "Move U\n";
        cout.flush();

        for (int k = 0; k < 40000000; k++);
        if (rand() % 100 == 0) { sleep(1); }
    }
}
