#include "crow.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

struct IrisSample {
    double sepal_length;
    double sepal_width;
    double petal_length;
    double petal_width;
    string species; 
};

const vector<IrisSample> dataset = {
    {5.1, 3.5, 1.4, 0.2, "setosa"}, {4.9, 3.0, 1.4, 0.2, "setosa"}, {4.7, 3.2, 1.3, 0.2, "setosa"},
    {4.6, 3.1, 1.5, 0.2, "setosa"}, {5.0, 3.6, 1.4, 0.2, "setosa"}, {5.4, 3.9, 1.7, 0.4, "setosa"},
    {4.6, 3.4, 1.4, 0.3, "setosa"}, {5.0, 3.4, 1.5, 0.2, "setosa"}, {4.4, 2.9, 1.4, 0.2, "setosa"},
    {4.9, 3.1, 1.5, 0.1, "setosa"}, {5.4, 3.7, 1.5, 0.2, "setosa"}, {4.8, 3.4, 1.6, 0.2, "setosa"},
    {4.8, 3.0, 1.4, 0.1, "setosa"}, {4.3, 3.0, 1.1, 0.1, "setosa"}, {5.8, 4.0, 1.2, 0.2, "setosa"},
    {7.0, 3.2, 4.7, 1.4, "versicolor"}, {6.4, 3.2, 4.5, 1.5, "versicolor"}, {6.9, 3.1, 4.9, 1.5, "versicolor"},
    {5.5, 2.3, 4.0, 1.3, "versicolor"}, {6.5, 2.8, 4.6, 1.5, "versicolor"}, {5.7, 2.8, 4.5, 1.3, "versicolor"},
    {6.3, 3.3, 4.7, 1.6, "versicolor"}, {4.9, 2.4, 3.3, 1.0, "versicolor"}, {6.6, 2.9, 4.6, 1.3, "versicolor"},
    {5.2, 2.7, 3.9, 1.4, "versicolor"}, {5.0, 2.0, 3.5, 1.0, "versicolor"}, {5.9, 3.0, 4.2, 1.5, "versicolor"},
    {6.0, 2.2, 4.0, 1.0, "versicolor"}, {6.1, 2.9, 4.7, 1.4, "versicolor"}, {5.6, 2.9, 3.6, 1.3, "versicolor"},
    {6.3, 3.3, 6.0, 2.5, "virginica"}, {5.8, 2.7, 5.1, 1.9, "virginica"}, {7.1, 3.0, 5.9, 2.1, "virginica"},
    {6.3, 2.9, 5.6, 1.8, "virginica"}, {6.5, 3.0, 5.8, 2.2, "virginica"}, {7.6, 3.0, 6.6, 2.1, "virginica"},
    {4.9, 2.5, 4.5, 1.7, "virginica"}, {7.3, 2.9, 6.3, 1.8, "virginica"}, {6.7, 2.5, 5.8, 1.8, "virginica"},
    {7.2, 3.6, 6.1, 2.5, "virginica"}, {6.5, 3.2, 5.1, 2.0, "virginica"}, {6.4, 2.7, 5.3, 1.9, "virginica"},
    {6.8, 3.0, 5.5, 2.1, "virginica"}, {5.7, 2.5, 5.0, 2.0, "virginica"}, {5.8, 2.7, 5.1, 1.9, "virginica"}
};

struct DistancePair {
    double distance;
    string species;
};

bool compareDistances(const DistancePair &a, const DistancePair &b) {
    return a.distance < b.distance;
}

double calculateDistance(double sl1, double sw1, double pl1, double pw1, double sl2, double sw2, double pl2, double pw2) {
    return sqrt(pow(sl1 - sl2, 2) + pow(sw1 - sw2, 2) + pow(pl1 - pl2, 2) + pow(pw1 - pw2, 2));
}

void lower(string &s) {
    string ans = "";
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ' ') {
            continue; 
        }
        ans += tolower(s[i]);
    }
    s = ans;
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        auto page = crow::mustache::load("index.html");
        return page.render();
    });

    CROW_ROUTE(app, "/predict")([](const crow::request& req){
        auto sl_p = req.url_params.get("sl");
        auto sw_p = req.url_params.get("sw");
        auto pl_p = req.url_params.get("pl");
        auto pw_p = req.url_params.get("pw");

        if (!sl_p || !sw_p || !pl_p || !pw_p) {
            return crow::response("Error: Missing flower metric parameters.");
        }

        double user_sl = stod(sl_p);
        double user_sw = stod(sw_p);
        double user_pl = stod(pl_p);
        double user_pw = stod(pw_p);

        vector<DistancePair> distances;
        for (const auto& sample : dataset) {
            double dist = calculateDistance(user_sl, user_sw, user_pl, user_pw, sample.sepal_length, sample.sepal_width, sample.petal_length, sample.petal_width);
            distances.push_back({dist, sample.species});
        }

        sort(distances.begin(), distances.end(), compareDistances);

        map<string, int> votes;
        for (int i = 0; i < 5; i++) {
            votes[distances[i].species]++;
        }

        string predicted_species = "";
        int max_votes = -1;
        for (const auto& vote : votes) {
            if (vote.second > max_votes) {
                max_votes = vote.second;
                predicted_species = vote.first;
            }
        }

        int confidence_percentage = (max_votes * 100) / 5;
        string confidence_str = to_string(confidence_percentage) + "%";

        lower(predicted_species);

        // Package both values securely with essential browser CORS clearance rules
        crow::response res(predicted_species + "," + confidence_str);
        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    char* port = getenv("PORT");
    uint16_t app_port = port ? (uint16_t)stoi(port) : 18080;
    app.bindaddr("0.0.0.0").port(app_port).multithreaded().run();
    return 0;
}