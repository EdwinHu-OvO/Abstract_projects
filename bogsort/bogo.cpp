#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <omp.h> // OpenMP 库
//std::string fileName = "D:\\资料\\烂活\\bogsort";
// 检查数组是否已经排序
bool isSort(const std::vector<int>& arr) {
	for (size_t i = 1; i < arr.size(); ++i) {
		if (arr[i] < arr[i - 1]) {
			return false;
		}
	}
	return true;
}

// 并行 BogoSort 排序算法
void parallelBogosort(std::vector<int>& arr) {
	std::srand(std::time(0)); // 初始化随机种子
	std::vector<int> tempArr = arr; // 临时数组，用于线程间共享

	bool sorted = false;

#pragma omp parallel shared(tempArr, sorted)
	{
		while (!sorted) {
			std::vector<int> localArr = tempArr; // 每个线程独立工作

			// 随机打乱数组
			std::random_shuffle(localArr.begin(), localArr.end());

			// 检查是否排序
			if (isSort(localArr)) {
#pragma omp critical // 线程安全更新结果
				{
					if (!sorted) {
						sorted = true;
						tempArr = localArr;
					}
				}
			}
		}
	}

	arr = tempArr; // 将排序结果写回
}

// 从文件中读取数字并存入数组
std::vector<int> readNumbersFromFile(const std::string& fileName) {
	std::vector<int> numbers;
	std::ifstream file(fileName);
	if (!file) {
		std::cerr << "文件 " << fileName << " 未找到，请检查文件路径。" << std::endl;
		return numbers;
	}

	std::string line;
	while (std::getline(file, line)) {
		try {
			int number = std::stoi(line); // 将字符串转为整数
			numbers.push_back(number);
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "文件内容格式错误：" << e.what() << std::endl;
			return{};
		}
	}
	file.close();

	std::cout << "从 " << fileName << " 中读取了 " << numbers.size() << " 个数字：" << std::endl;
	for (int num : numbers) {
		std::cout << num << " ";
	}
	std::cout << std::endl;

	return numbers;
}

int main() {
	std::string inputFile = "D:\\资料\\烂活\\bogsort\\arr.txt"; // 要读取的文件名
	std::vector<int> arr = readNumbersFromFile(inputFile);

	// 验证数组是否已成功加载
	if (arr.empty()) {
		std::cerr << "未能从文件中加载数据，请检查问题。" << std::endl;
		return 1;
	}
	else {
		std::cout << "新数组已成功创建！" << std::endl;
	}

	// 调用并行 BogoSort 排序
	parallelBogosort(arr);

	// 输出排序结果
	std::cout << "排序后的数组：" << std::endl;
	for (int num : arr) {
		std::cout << num << " ";
	}
	std::cout << std::endl;

	return 0;
}
