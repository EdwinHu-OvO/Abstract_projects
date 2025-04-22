


#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

vector<int> processArray(const vector<int>& input) {
	// 如果输入数组为空，直接返回空数组
	if (input.empty()) {
		return{};
	}

	// 1. 找出数组的最大值和最小值
	int minVal = input[0];
	int maxVal = input[0];

	for (int num : input) {
		if (num < minVal) {
			minVal = num;
		}
		if (num > maxVal) {
			maxVal = num;
		}
	}

	// 2. 创建临时数组，用于标记存在的元素
	// 临时数组的大小为最大值-最小值+1，初始化为false
	vector<bool> exists(maxVal - minVal + 1, false);

	// 4. 标记输入数组中存在的元素
	for (int num : input) {
		exists[num - minVal] = true;
	}

	// 5. 生成最终去重并排序的数组
	vector<int> result;
	for (int i = 0; i < exists.size(); i++) {
		if (exists[i]) {
			result.push_back(i + minVal);
		}
	}

	return result;
}

int main() {
	// 测试用例
	vector<int> arr = { 5, 2, 7, 3, 9, 5, 2, 7, 1, 8 };
	cout << "原始数组: ";
	for (int num : arr) {
		cout << num << " ";
	}
	cout << endl;

	vector<int> processed = processArray(arr);

	cout << "处理后数组: ";
	for (int num : processed) {
		cout << num << " ";
	}
	cout << endl;

	return 0;
}