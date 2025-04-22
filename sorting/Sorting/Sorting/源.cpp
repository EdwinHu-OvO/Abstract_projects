#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <unordered_set>
#include <mutex>
#include <random>
#include <ctime>
#include <chrono>
#include <bitset>
#include <memory>
#include <intrin.h> // 用于MSVC的内置函数
#include <future>   // 用于异步任务

// MSVC没有__builtin_ctzll，使用这个函数代替
unsigned long long countTrailingZeros(uint64_t x) {
	if (x == 0) return 64;

	unsigned long index;
#ifdef _WIN64
	_BitScanForward64(&index, x);
	return index;
#else
	// 32位系统需要分两部分处理
	if (static_cast<uint32_t>(x) == 0) {
		_BitScanForward(&index, static_cast<uint32_t>(x >> 32));
		return index + 32;
	}
	else {
		_BitScanForward(&index, static_cast<uint32_t>(x));
		return index;
	}
#endif
}

// 查找数组最大值和最小值
std::pair<int, int> findMinMax(const std::vector<int>& arr) {
	if (arr.empty()) {
		return{ 0, 0 };
	}
	std::pair<std::vector<int>::const_iterator, std::vector<int>::const_iterator> min_max =
		std::minmax_element(arr.begin(), arr.end());
	return{ *min_max.first, *min_max.second };
}

// 多级位图索引结构
class MultiLevelBitmap {
private:
	static const int BITS_PER_WORD = 64;
	static const int L1_BLOCK_SIZE = 64;
	static const int L2_BLOCK_SIZE = 64;

	std::vector<uint64_t> level1; // 每64位为一个块
	std::vector<uint64_t> level2; // 每64个level1块为一组
	int min_value;
	int range;

public:
	MultiLevelBitmap(const std::vector<int>& input_array, int min_val, int max_val)
		: min_value(min_val) {
		range = max_val - min_val + 1;

		// 初始化level1位图
		level1.resize((range + BITS_PER_WORD - 1) / BITS_PER_WORD, 0);

		// 标记所有存在的元素
		for (int num : input_array) {
			int idx = num - min_val;
			level1[idx / BITS_PER_WORD] |= (1ULL << (idx % BITS_PER_WORD));
		}

		// 初始化level2位图
		level2.resize((level1.size() + L2_BLOCK_SIZE - 1) / L2_BLOCK_SIZE, 0);

		// 标记非空的level1块
		for (size_t i = 0; i < level1.size(); ++i) {
			if (level1[i] != 0) {
				level2[i / L2_BLOCK_SIZE] |= (1ULL << (i % L2_BLOCK_SIZE));
			}
		}
	}

	// 快速扫描函数
	void scan(int start_idx, int end_idx, std::vector<int>& result) {
		if (start_idx < 0) start_idx = 0;
		if (end_idx > range) end_idx = range;

		// 计算level1和level2的起始和结束块
		int l1_start = start_idx / BITS_PER_WORD;
		int l1_end = (end_idx + BITS_PER_WORD - 1) / BITS_PER_WORD;

		int l2_start = l1_start / L2_BLOCK_SIZE;
		int l2_end = (l1_end + L2_BLOCK_SIZE - 1) / L2_BLOCK_SIZE;

		// 使用level2快速跳过大块
		for (int l2 = l2_start; l2 < l2_end && l2 < static_cast<int>(level2.size()); ++l2) {
			if (level2[l2] == 0) continue; // 整个大块都是空的，跳过

			// 计算当前大块内level1的范围
			int l1_block_start = std::max(l2 * L2_BLOCK_SIZE, l1_start);
			int l1_block_end = std::min((l2 + 1) * L2_BLOCK_SIZE, l1_end);

			// 遍历当前大块内的level1块
			for (int l1 = l1_block_start; l1 < l1_block_end && l1 < static_cast<int>(level1.size()); ++l1) {
				if (level1[l1] == 0) continue; // 整个小块都是空的，跳过

				// 计算当前块内的起始和结束位
				int bit_start = (l1 == l1_start) ? (start_idx % BITS_PER_WORD) : 0;
				int bit_end = (l1 == l1_end - 1) ? (end_idx % BITS_PER_WORD) : BITS_PER_WORD;

				// 处理当前块内的每一位
				uint64_t word = level1[l1];

				// 清除不需要处理的低位
				if (bit_start > 0) {
					word &= ~((1ULL << bit_start) - 1);
				}

				// 清除不需要处理的高位
				if (bit_end < BITS_PER_WORD) {
					word &= ((1ULL << bit_end) - 1);
				}

				// 快速找出所有设置的位
				while (word) {
					int bit_pos = countTrailingZeros(word); // 使用我们的替代函数
					int global_idx = l1 * BITS_PER_WORD + bit_pos;

					if (global_idx >= start_idx && global_idx < end_idx) {
						result.push_back(global_idx + min_value);
					}

					// 清除已处理的位
					word &= (word - 1);
				}
			}
		}
	}
};

// 优化版的并行处理函数
std::vector<std::vector<int>> processArrayOptimized(const std::vector<int>& input_array) {
	// 1. 找出最大值和最小值
	std::pair<int, int> min_max = findMinMax(input_array);
	int min_val = min_max.first;
	int max_val = min_max.second;
	int range = max_val - min_val + 1;

	// 2. 构建多级位图
	MultiLevelBitmap bitmap(input_array, min_val, max_val);

	// 3. 确定最优线程数
	unsigned int num_cores = std::thread::hardware_concurrency();
	if (num_cores == 0) num_cores = 1;

	// 自适应调整线程数，根据数据规模
	num_cores = std::min(num_cores, static_cast<unsigned int>(range / 10000 + 1));
	num_cores = std::max(num_cores, 1u);

	// 4. 并行处理
	std::vector<std::thread> threads;
	std::vector<std::vector<int>> results(num_cores);

	// 为每个线程预分配内存
	for (auto& result : results) {
		result.reserve(range / num_cores / 10); // 假设10%的数据密度
	}

	// 计算每个线程的工作范围
	int chunk_size = range / num_cores;

	for (unsigned int i = 0; i < num_cores; ++i) {
		int start_idx = i * chunk_size;
		int end_idx = (i == num_cores - 1) ? range : (i + 1) * chunk_size;

		threads.emplace_back([&bitmap, &results, i, start_idx, end_idx]() {
			bitmap.scan(start_idx, end_idx, results[i]);
		});
	}

	// 等待所有线程完成
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	return results;
}

// 快速排序分区函数
int partition(std::vector<int>& arr, int low, int high) {
	int pivot = arr[high]; // 选择最后一个元素作为基准点
	int i = (low - 1);     // 小于基准点的元素的索引

	for (int j = low; j <= high - 1; j++) {
		// 如果当前元素小于或等于基准点，则交换
		if (arr[j] <= pivot) {
			i++;
			std::swap(arr[i], arr[j]);
		}
	}
	std::swap(arr[i + 1], arr[high]);
	return (i + 1);
}

// 多线程快速排序实现
// 定义一个线程池大小，控制创建的线程数量
const unsigned int MAX_THREADS = std::thread::hardware_concurrency();
std::atomic<unsigned int> active_threads(0); // 当前活跃线程计数

// 并行快速排序函数
void parallelQuickSort(std::vector<int>& arr, int low, int high, int depth) {
	// 当数组小于一定大小或递归深度超过阈值时，使用串行排序
	if (high - low < 1000 || depth > 3) {
		// 使用标准的快速排序
		if (low < high) {
			int pi = partition(arr, low, high);
			parallelQuickSort(arr, low, pi - 1, depth + 1);
			parallelQuickSort(arr, pi + 1, high, depth + 1);
		}
		return;
	}

	// 分区
	int pi = partition(arr, low, high);

	// 创建异步任务的条件：活跃线程数小于最大线程数
	bool can_create_thread = active_threads.load() < MAX_THREADS;

	// 左半部分排序
	std::future<void> left_future;
	if (can_create_thread) {
		active_threads++;
		left_future = std::async(std::launch::async, [&arr, low, pi, depth]() {
			parallelQuickSort(arr, low, pi - 1, depth + 1);
			active_threads--;
		});
	}
	else {
		// 如果不能创建新线程，则在当前线程中排序
		parallelQuickSort(arr, low, pi - 1, depth + 1);
	}

	// 右半部分排序（始终在当前线程中完成）
	parallelQuickSort(arr, pi + 1, high, depth + 1);

	// 等待左侧排序完成（如果是异步的）
	if (left_future.valid()) {
		left_future.wait();
	}
}

// 串行快速排序实现
void quickSort(std::vector<int>& arr, int low, int high) {
	if (low < high) {
		// 获取分区点
		int pi = partition(arr, low, high);

		// 递归地对分区点两侧的元素进行排序
		quickSort(arr, low, pi - 1);
		quickSort(arr, pi + 1, high);
	}
}

// 使用多线程快速排序的方法处理数组
std::vector<int> processArrayParallelQuickSort(const std::vector<int>& input_array) {
	// 复制输入数组
	std::vector<int> sorted_array = input_array;

	// 初始化活跃线程计数
	active_threads = 0;

	// 使用并行快速排序算法对数组进行排序
	parallelQuickSort(sorted_array, 0, sorted_array.size() - 1, 0);

	// 去除重复元素
	std::vector<int> result;
	result.reserve(sorted_array.size()); // 预分配空间以提高性能

	for (size_t i = 0; i < sorted_array.size(); ++i) {
		// 如果当前元素与前一个元素不同，或者这是第一个元素，则添加到结果中
		if (i == 0 || sorted_array[i] != sorted_array[i - 1]) {
			result.push_back(sorted_array[i]);
		}
	}

	return result;
}

// 使用常规快速排序的方法处理数组
std::vector<int> processArrayQuickSort(const std::vector<int>& input_array) {
	// 复制输入数组
	std::vector<int> sorted_array = input_array;

	// 使用快速排序算法对数组进行排序
	quickSort(sorted_array, 0, sorted_array.size() - 1);

	// 去除重复元素
	std::vector<int> result;
	for (size_t i = 0; i < sorted_array.size(); ++i) {
		// 如果当前元素与前一个元素不同，或者这是第一个元素，则添加到结果中
		if (i == 0 || sorted_array[i] != sorted_array[i - 1]) {
			result.push_back(sorted_array[i]);
		}
	}

	return result;
}
std::vector<int> processArray(const std::vector<int>& input) {
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
	std::vector<bool> exists(maxVal - minVal + 1, false);

	// 4. 标记输入数组中存在的元素
	for (int num : input) {
		exists[num - minVal] = true;
	}

	// 5. 生成最终去重并排序的数组
	std::vector<int> result;
	for (int i = 0; i < exists.size(); i++) {
		if (exists[i]) {
			result.push_back(i + minVal);
		}
	}

	return result;
}
// 生成随机数组
std::vector<int> generateRandomArray(int size, int min_range, int max_range) {
	std::vector<int> arr;
	arr.reserve(size);

	std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
	std::uniform_int_distribution<int> dist(min_range, max_range);

	for (int i = 0; i < size; ++i) {
		arr.push_back(dist(rng));
	}

	return arr;
}

int main() {
	// 生成随机数组，大小和范围可调整
	int array_size = 10000000; // 一百万个元素
	int min_range = 0;
	int max_range = 10000000;

	std::vector<int> input = generateRandomArray(array_size, min_range, max_range);

	std::cout << "原始数组大小: " << input.size() << " 元素" << std::endl;

	// 测试优化的并行过滤方法并计时
	std::cout << "\n====== 优化的并行过滤方法 ======" << std::endl;
	auto start_optimized = std::chrono::high_resolution_clock::now();
	std::vector<std::vector<int>> results_optimized = processArrayOptimized(input);
	auto end_optimized = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_optimized = end_optimized - start_optimized;
	std::cout << "优化的并行过滤方法耗时: " << elapsed_optimized.count() << " 毫秒" << std::endl;

	// 合并优化方法的所有结果
	std::vector<int> merged_result_optimized;
	size_t total_size = 0;
	for (const auto& result : results_optimized) {
		total_size += result.size();
	}
	merged_result_optimized.reserve(total_size);

	for (const auto& result : results_optimized) {
		merged_result_optimized.insert(merged_result_optimized.end(),
			result.begin(), result.end());
	}
	std::cout << "\n====== 去重排序 ======" << std::endl;
	auto start_process_arry_sort = std::chrono::high_resolution_clock::now();
	std::vector<int> result_process_arry_sort = processArray(input);
	auto end_process_arry_sort = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_process_arry_sort = end_process_arry_sort - start_process_arry_sort;
	std::cout << "去重排序方法耗时: " << elapsed_process_arry_sort.count() << " 毫秒" << std::endl;
	// 测试并行快速排序方法并计时
	std::cout << "\n====== 并行快速排序方法 ======" << std::endl;
	auto start_parallel_quicksort = std::chrono::high_resolution_clock::now();
	std::vector<int> result_parallel_quicksort = processArrayParallelQuickSort(input);
	auto end_parallel_quicksort = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_parallel_quicksort = end_parallel_quicksort - start_parallel_quicksort;
	std::cout << "并行快速排序方法耗时: " << elapsed_parallel_quicksort.count() << " 毫秒" << std::endl;

	// 测试串行快速排序方法并计时
	std::cout << "\n====== 串行快速排序方法 ======" << std::endl;
	auto start_quicksort = std::chrono::high_resolution_clock::now();
	std::vector<int> result_quicksort = processArrayQuickSort(input);
	auto end_quicksort = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_quicksort = end_quicksort - start_quicksort;
	std::cout << "串行快速排序方法耗时: " << elapsed_quicksort.count() << " 毫秒" << std::endl;

	// 比较三种方法的结果
	std::cout << "\n====== 性能比较 ======" << std::endl;
	std::cout << "优化的并行过滤方法元素数量: " << merged_result_optimized.size() << std::endl;
	std::cout << "并行快速排序方法元素数量: " << result_parallel_quicksort.size() << std::endl;
	std::cout << "串行快速排序方法元素数量: " << result_quicksort.size() << std::endl;
	std::cout << "去重排序方法元素数量: " << result_process_arry_sort.size() << std::endl;
	// 排序并行方法的结果以便比较
	std::sort(merged_result_optimized.begin(), merged_result_optimized.end());

	bool results_match1 = (merged_result_optimized == result_quicksort);
	bool results_match2 = (result_parallel_quicksort == result_quicksort);
	bool results_match3 = (result_process_arry_sort == result_quicksort);
	std::cout << "并行过滤方法与串行快速排序结果是否一致: " << (results_match1 ? "是" : "否") << std::endl;
	std::cout << "并行快速排序与串行快速排序结果是否一致: " << (results_match2 ? "是" : "否") << std::endl;
	std::cout << "去重排序与串行快速排序结果是否一致: " << (results_match3 ? "是" : "否") << std::endl;

	// 计算加速比
	double speedup1 = elapsed_quicksort.count() / elapsed_optimized.count();
	double speedup2 = elapsed_quicksort.count() / elapsed_parallel_quicksort.count();
	std::cout << "优化的并行方法相对于串行快速排序的加速比: " << speedup1 << "倍" << std::endl;
	std::cout << "并行快速排序相对于串行快速排序的加速比: " << speedup2 << "倍" << std::endl;

	return 0;
}