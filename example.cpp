#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {
    std::ios::sync_with_stdio(false);
    cerr.tie(nullptr);
    cerr << std::nounitbuf;

    std::string s;
    std::stringstream ss;

    int frame;

    // 读取初始地图信息
    std::getline(cin, s, '\0');
    ss << s;

    int width, height;
    ss >> width >> height;
    cerr << "Map size: " << width << "x" << height << std::endl;

    // 读取剩下的地图信息，此处仅作示例
    frame = 1000000;
    ss.str("");

    for (int i = 0; i < frame; i++) {
        std::getline(cin, s, '\0');
        ss << s;

        // 如果输入流中还有数据，说明游戏已经在请求下一帧了。
        // 这时候我们应该跳过当前帧，以便能够及时响应游戏。
        if (cin.rdbuf()->in_avail() > 0) {
            cerr << "Warning: skipping frame " << i
                 << " to catch up with the game" << std::endl;
            continue;
        }

        // 读取当前帧的游戏状态
        ss >> s;
        assert(s == "Frame");
        int currentFrame;
        ss >> currentFrame;
        assert(currentFrame == i);

        // 读取剩下的信息，此处仅作示例
        for (int k = 0; k < 1000000; k++)
            ;
        ss.str("");

        // 输出当前帧的操作，此处仅作示例
        cout << "Frame " << i << "\n";
        cout << "Move R\n";
        cout << "Move U\n";
        // 不要忘记刷新输出流，否则游戏将无法及时收到响应
        cout.flush();
    }
}
