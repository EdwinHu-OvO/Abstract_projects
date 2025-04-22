


#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

vector<int> processArray(const vector<int>& input) {
	// �����������Ϊ�գ�ֱ�ӷ��ؿ�����
	if (input.empty()) {
		return{};
	}

	// 1. �ҳ���������ֵ����Сֵ
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

	// 2. ������ʱ���飬���ڱ�Ǵ��ڵ�Ԫ��
	// ��ʱ����Ĵ�СΪ���ֵ-��Сֵ+1����ʼ��Ϊfalse
	vector<bool> exists(maxVal - minVal + 1, false);

	// 4. ������������д��ڵ�Ԫ��
	for (int num : input) {
		exists[num - minVal] = true;
	}

	// 5. ��������ȥ�ز����������
	vector<int> result;
	for (int i = 0; i < exists.size(); i++) {
		if (exists[i]) {
			result.push_back(i + minVal);
		}
	}

	return result;
}

int main() {
	// ��������
	vector<int> arr = { 5, 2, 7, 3, 9, 5, 2, 7, 1, 8 };
	cout << "ԭʼ����: ";
	for (int num : arr) {
		cout << num << " ";
	}
	cout << endl;

	vector<int> processed = processArray(arr);

	cout << "���������: ";
	for (int num : processed) {
		cout << num << " ";
	}
	cout << endl;

	return 0;
}