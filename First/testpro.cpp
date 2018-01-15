//Prepared by: Hooman Shariati    hooman.shariati@alumni.ubc.ca
#include<iostream>
#include<vector>
#include<string>
#include<unordered_map>
#include<utility>
#include<fstream>
#include<algorithm>
using namespace std;

bool comparator(const string& a, const string& b){
	return a.size() > b.size();
}
bool canBuildWord(string str, bool isOriginalWord, unordered_map<string, bool>& map){
	if (map.count(str) && !isOriginalWord)
		return map[str];
	for (int i = 1; i < str.size(); i++){
		string left = str.substr(0, i);
		string right = str.substr(i);
		if (map.count(left) && map[left] == true && canBuildWord(right, false, map))
			return true;
	} map[str] = false;
	return false;
}
pair<pair<string, string>, int> LongestWordMadeOfOther(string fileName) {
	pair<pair<string, string>, int> out = make_pair(make_pair("", ""), 0);
	ifstream file(fileName);
	if (file.bad())
		return out;
	vector<string> arr;
	string temp;
	while (file.good()){
		getline(file, temp);
		arr.push_back(temp);
	}
	unordered_map<string, bool> map;
	for (auto p = arr.begin(); p != arr.end();p++)
		map[*p] = true;
	sort(arr.begin(), arr.end(), comparator); // Sort by length
	int count = 0;
	for (auto p = arr.begin(); p != arr.end(); p++){
		if (canBuildWord(*p, true, map)){
			count++;
			if (out.first.first == "")
				out.first.first = *p;
			else if (out.first.second == "")
				out.first.second = *p;
		}
	}
	out.second = count;
	return out;
}

int main(){
	auto res = LongestWordMadeOfOther("wordsforproblem.txt");
	cout << res.first.first << "   " << res.first.second << "   " << res.second << "\n";
}