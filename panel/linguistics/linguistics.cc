#include "linguistics.hh"
#include <utils/utils.hh>
#include <utils/fnv.h>
std::map<const char*, string_data> str_data;

const char* strings(const char* english) {
    const char* to_return{ "" };

    switch (g_cfg.language) {
    case 0: to_return = str_data[english].chinese; break;
    default: to_return = english; break;
    }

    if (fnv::hash_runtime(to_return) == fnv::hash("")) {
        return english;
    }

    switch (g_cfg.language) {
    case 0: return str_data[english].chinese;
    default: return english;
    }
}

void build_string(const char* english, const char* chinese) {
    auto& k = str_data[english];

    k.english = english;
    k.chinese = chinese;
}

void build_strings() {
    build_string("Vertigo Panel", u8"自动升级");
    build_string("Username", u8"账户");
    build_string("Password", u8"密码");
    build_string("Login", u8"登录");

    build_string("Username empty", u8"用户名为空");
    build_string("Error", u8"登录");

    build_string("Hosts file not Found or permission denied\nPlease re-run the program as an admin\n", u8"找不到主机文件或权限被拒绝\n请以管理員身份重新进行该程序\n");

    build_string("Accounts", u8"账户");
    build_string("Boost", u8"VS");
    build_string("Settings", u8"设置");

    build_string("ID", u8"ID");
    build_string("Friend Code", u8"邀请码");
    build_string("Team ID", u8"队伍");
    build_string("Steam ID", u8"Steam ID");

    build_string("User Filter", u8"账户搜索");
    build_string("Team Filter", u8"队伍查看");
    build_string("No Filter", u8"沒有过滤");

    build_string("Import", u8"账户输入");
    build_string("Auto assign teams", u8"自动分配队伍");
    build_string("Get SteamIDs", u8"得到steamID");
    build_string("Save", u8"保存");
    build_string("Start", u8"开始");
    build_string("Continue", u8"继续开始");
    build_string("Select", u8"选择");
    build_string("Select accounts", u8"选择账户");

    build_string("Map", u8"地图");
    build_string("Unassigned", u8"未分配");
    build_string("Team", u8"队伍");
    build_string("Team 1", u8"队伍 1");
    build_string("Team 2", u8"队伍 2");

    build_string("Stop boosting", u8"停止提升");
    build_string("And", u8"和");
    build_string("and", u8"和");
    build_string("Currently boosting", u8"目前的队伍");
    build_string("Times boosted", u8"完成的比赛");
    //build_string("Kill all CS:GO Instances", u8"关闭CSGO");
    build_string("Toggle window visibility", u8"隐藏窗口");
    build_string("directory", u8"目录");
    build_string("Steam directory", u8"Steam 目录");
    build_string("CS:GO directory", u8"CS:GO 目录");
    build_string("Delete", u8"删除");
    build_string("Hello", u8"你好");
    build_string("Lobby", u8"大厅");
    build_string("Match mode", u8"比赛模式");
    build_string("Repetition", u8"游戏次数");
    build_string("Draw\0Win Lose\0Win win lose lose\0Manual\0Team1 win\0Team2 win\0Rank unlocker\0", u8"平局\0赢输\0赢赢输输\0备用\0队伍1赢\0队伍2赢\0排名提升\0");
    build_string("Time to wait for a match: %.2f", u8"等待比赛时间");
    build_string("Webhook url", u8"网站");
    build_string("Test webhook", u8"测试");
    build_string("Language", u8"语言");
    build_string("Launch accounts timeout", u8"启动账户时间");
    build_string("The time the panel waits to after starting cs instances.\nKindly use relatively more time than what it takes you to load the cs instances.\nThis saves your accounts from getting penalties and saves us time.", u8"面板在启动csgo后等待的时间。");
    build_string("Stop boosting and kill CS:GO", u8"停止提升 和 关闭CSGO。");
}