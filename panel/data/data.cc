#include "data.hh"

#include <iomanip>
#include <fstream>
#include <includes.hh>
#include <utils/utils.hh>

cdata::cdata() {
    obj.clear();
    std::ifstream in("assets\\panel\\database.json");

    if (in.good()) {
        obj = nlohmann::json::parse(in, nullptr, false);
        if (obj.is_discarded()) {
            in.close();
            return;
        }
    }
    else { return; }

    for (size_t i = 0; i < obj.size(); i++) {
        acc_info acc = {};
        {
            if (obj[i].contains("user")) obj[i]["user"] .get_to(acc.user);
            if (obj[i].contains("pass")) obj[i]["pass"] .get_to(acc.pass);
            if (obj[i].contains("code")) obj[i]["code"] .get_to(acc.code);
            if (obj[i].contains("w_id")) obj[i]["w_id"] .get_to(acc.w_id);
            if (obj[i].contains("t_id")) obj[i]["t_id"] .get_to(acc.t_id);
            if (obj[i].contains("s_id")) obj[i]["s_id"] .get_to(acc.s_id);
        }
        accs.push_back(acc);
        accs[accs.size() - 1].idx = accs.size() - 1;
    }
}

void cdata::set_code(int idx, std::string code) {
    accs[idx].code = code;
}

void cdata::set_steam(int idx, size_t code) {
    accs[idx].s_id = std::to_string(code);
}

void cdata::exp() {
    std::ofstream o("assets\\panel\\database.json");
    nlohmann::json obj2;
    obj2.clear();

    for (size_t i = 0; i < accs.size(); i++) {
        auto& acc = accs[i];

        obj2[i]["user"] = acc.user;
        obj2[i]["pass"] = acc.pass;
        obj2[i]["code"] = acc.code;
        obj2[i]["t_id"] = acc.t_id;
        obj2[i]["s_id"] = acc.s_id;
        obj2[i]["w_id"] = acc.w_id;
    }

    o << std::setw(4) << obj2;
    o.close();
}

void cdata::imp_from_txt() {
    std::ifstream file("assets\\accounts.txt");
    if (!file.is_open()) return;

    std::string line;
    int num_of_lines = 0;

    while (std::getline(file, line)) {
        std::string username = "";
        std::string password = "";
        bool hit_delimiter = false;

        for (size_t i = 0; i < line.length(); i++) {
            if (line[i] != ':') {
                if (hit_delimiter) {
                    password += line[i];
                }
                else {
                    username += line[i];
                }
            }
            else {
                hit_delimiter = true;
            }
        }

        bool is_account_new = true;
        for (size_t i = 0; i < obj.size(); i++) {
            if (obj[i].contains("user") && obj[i]["user"].get<std::string>() == username) {
                is_account_new = false;
                break;
            }
        }
        if (!is_account_new) continue;

        obj[accs.size()]["user"] = username;
        obj[accs.size()]["pass"] = password;
        obj[accs.size()]["code"] = "nil";
        obj[accs.size()]["t_id"] = 0;
        obj[accs.size()]["w_id"] = 0;
        obj[accs.size()]["s_id"] = "nil";

        acc_info acc = {};
        {
            acc.user = obj[accs.size()]["user"];
            acc.pass = obj[accs.size()]["pass"];
            acc.code = obj[accs.size()]["code"];
            acc.t_id = obj[accs.size()]["t_id"];
            acc.w_id = obj[accs.size()]["w_id"];
            acc.s_id = obj[accs.size()]["s_id"];
        }
        accs.push_back(acc);
        accs[accs.size() - 1].idx = accs.size() - 1;

        username = "";
        password = "";
        num_of_lines++;
    }
}

std::vector<acc_info> cdata::get_accs_team(int t_id, int leader_swap) {
    std::vector<acc_info> temp = {};
    if (leader_swap != -1) {
        temp.push_back(get_accs_team(t_id)[0]);

        auto team_2 = get_accs_team(leader_swap);
        for (size_t j{ 1 }; j < team_2.size(); j++) {
            temp.push_back(team_2[j]);
        }
    }
    else {
        for (size_t i{ 0 }; i < accs.size(); i++) {
            if (accs[i].t_id == t_id) temp.push_back(accs[i]);
        }
    }
    return temp;
}

std::vector<acc_info> cdata::get_accs_wingman(int w_id) {
    std::vector<acc_info> temp = {};
    for (size_t i{ 0 }; i < accs.size(); i++) {
        if (accs[i].w_id == w_id) temp.push_back(accs[i]);
    }
    return temp;
}

acc_info cdata::get(int idx) {
    return accs[idx];
}

acc_info cdata::get(std::string user) {
    for (size_t i = 0; i < accs.size(); i++) {
        if (accs[i].user == user) return accs[i];
    }
    return {};
}

void cdata::delete_accs_from_db(int team_id) {
    std::vector<acc_info> new_data{};
    new_data.clear();
    std::ofstream outfile;
    outfile.open("assets\\panel\\deleted accounts.txt", std::ios_base::app); // append instead of overwrite

    for (size_t i = 0; i < accs.size(); i++) {
        if (accs[i].t_id == team_id) {
            outfile << accs[i].user << ":" << accs[i].pass << ":" << accs[i].code << ":" << accs[i].s_id << "\n";
            continue;
        }
        new_data.push_back(accs[i]);
    }
    outfile.close();

    accs = new_data;
    exp();
}

// i was thinking of removing those buttons and using images there