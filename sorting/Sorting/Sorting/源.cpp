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
#include <intrin.h> // ����MSVC�����ú���
#include <future>   // �����첽����

// MSVCû��__builtin_ctzll��ʹ�������������
unsigned long long countTrailingZeros(uint64_t x) {
	if (x == 0) return 64;

	unsigned long index;
#ifdef _WIN64
	_BitScanForward64(&index, x);
	return index;
#else
	// 32λϵͳ��Ҫ�������ִ���
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

// �����������ֵ����Сֵ
std::pair<int, int> findMinMax(const std::vector<int>& arr) {
	if (arr.empty()) {
		return{ 0, 0 };
	}
	std::pair<std::vector<int>::const_iterator, std::vector<int>::const_iterator> min_max =
		std::minmax_element(arr.begin(), arr.end());
	return{ *min_max.first, *min_max.second };
}

// �༶λͼ�����ṹ
class MultiLevelBitmap {
private:
	static const int BITS_PER_WORD = 64;
	static const int L1_BLOCK_SIZE = 64;
	static const int L2_BLOCK_SIZE = 64;

	std::vector<uint64_t> level1; // ÿ64λΪһ����
	std::vector<uint64_t> level2; // ÿ64��level1��Ϊһ��
	int min_value;
	int range;

public:
	MultiLevelBitmap(const std::vector<int>& input_array, int min_val, int max_val)
		: min_value(min_val) {
		range = max_val - min_val + 1;

		// ��ʼ��level1λͼ
		level1.resize((range + BITS_PER_WORD - 1) / BITS_PER_WORD, 0);

		// ������д��ڵ�Ԫ��
		for (int num : input_array) {
			int idx = num - min_val;
			level1[idx / BITS_PER_WORD] |= (1ULL << (idx % BITS_PER_WORD));
		}

		// ��ʼ��level2λͼ
		level2.resize((level1.size() + L2_BLOCK_SIZE - 1) / L2_BLOCK_SIZE, 0);

		// ��Ƿǿյ�level1��
		for (size_t i = 0; i < level1.size(); ++i) {
			if (level1[i] != 0) {
				level2[i / L2_BLOCK_SIZE] |= (1ULL << (i % L2_BLOCK_SIZE));
			}
		}
	}

	// ����ɨ�躯��
	void scan(int start_idx, int end_idx, std::vector<int>& result) {
		if (start_idx < 0) start_idx = 0;
		if (end_idx > range) end_idx = range;

		// ����level1��level2����ʼ�ͽ�����
		int l1_start = start_idx / BITS_PER_WORD;
		int l1_end = (end_idx + BITS_PER_WORD - 1) / BITS_PER_WORD;

		int l2_start = l1_start / L2_BLOCK_SIZE;
		int l2_end = (l1_end + L2_BLOCK_SIZE - 1) / L2_BLOCK_SIZE;

		// ʹ��level2�����������
		for (int l2 = l2_start; l2 < l2_end && l2 < static_cast<int>(level2.size()); ++l2) {
			if (level2[l2] == 0) continue; // ������鶼�ǿյģ�����

			// ���㵱ǰ�����level1�ķ�Χ
			int l1_block_start = std::max(l2 * L2_BLOCK_SIZE, l1_start);
			int l1_block_end = std::min((l2 + 1) * L2_BLOCK_SIZE, l1_end);

			// ������ǰ����ڵ�level1��
			for (int l1 = l1_block_start; l1 < l1_block_end && l1 < static_cast<int>(level1.size()); ++l1) {
				if (level1[l1] == 0) continue; // ����С�鶼�ǿյģ�����

				// ���㵱ǰ���ڵ���ʼ�ͽ���λ
				int bit_start = (l1 == l1_start) ? (start_idx % BITS_PER_WORD) : 0;
				int bit_end = (l1 == l1_end - 1) ? (end_idx % BITS_PER_WORD) : BITS_PER_WORD;

				// ����ǰ���ڵ�ÿһλ
				uint64_t word = level1[l1];

				// �������Ҫ����ĵ�λ
				if (bit_start > 0) {
					word &= ~((1ULL << bit_start) - 1);
				}

				// �������Ҫ����ĸ�λ
				if (bit_end < BITS_PER_WORD) {
					word &= ((1ULL << bit_end) - 1);
				}

				// �����ҳ��������õ�λ
				while (word) {
					int bit_pos = countTrailingZeros(word); // ʹ�����ǵ��������
					int global_idx = l1 * BITS_PER_WORD + bit_pos;

					if (global_idx >= start_idx && global_idx < end_idx) {
						result.push_back(global_idx + min_value);
					}

					// ����Ѵ����λ
					word &= (word - 1);
				}
			}
		}
	}
};

// �Ż���Ĳ��д�����
std::vector<std::vector<int>> processArrayOptimized(const std::vector<int>& input_array) {
	// 1. �ҳ����ֵ����Сֵ
	std::pair<int, int> min_max = findMinMax(input_array);
	int min_val = min_max.first;
	int max_val = min_max.second;
	int range = max_val - min_val + 1;

	// 2. �����༶λͼ
	MultiLevelBitmap bitmap(input_array, min_val, max_val);

	// 3. ȷ�������߳���
	unsigned int num_cores = std::thread::hardware_concurrency();
	if (num_cores == 0) num_cores = 1;

	// ����Ӧ�����߳������������ݹ�ģ
	num_cores = std::min(num_cores, static_cast<unsigned int>(range / 10000 + 1));
	num_cores = std::max(num_cores, 1u);

	// 4. ���д���
	std::vector<std::thread> threads;
	std::vector<std::vector<int>> results(num_cores);

	// Ϊÿ���߳�Ԥ�����ڴ�
	for (auto& result : results) {
		result.reserve(range / num_cores / 10); // ����10%�������ܶ�
	}

	// ����ÿ���̵߳Ĺ�����Χ
	int chunk_size = range / num_cores;

	for (unsigned int i = 0; i < num_cores; ++i) {
		int start_idx = i * chunk_size;
		int end_idx = (i == num_cores - 1) ? range : (i + 1) * chunk_size;

		threads.emplace_back([&bitmap, &results, i, start_idx, end_idx]() {
			bitmap.scan(start_idx, end_idx, results[i]);
		});
	}

	// �ȴ������߳����
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	return results;
}

// ���������������
int partition(std::vector<int>& arr, int low, int high) {
	int pivot = arr[high]; // ѡ�����һ��Ԫ����Ϊ��׼��
	int i = (low - 1);     // С�ڻ�׼���Ԫ�ص�����

	for (int j = low; j <= high - 1; j++) {
		// �����ǰԪ��С�ڻ���ڻ�׼�㣬�򽻻�
		if (arr[j] <= pivot) {
			i++;
			std::swap(arr[i], arr[j]);
		}
	}
	std::swap(arr[i + 1], arr[high]);
	return (i + 1);
}

// ���߳̿�������ʵ��
// ����һ���̳߳ش�С�����ƴ������߳�����
const unsigned int MAX_THREADS = std::thread::hardware_concurrency();
std::atomic<unsigned int> active_threads(0); // ��ǰ��Ծ�̼߳���

// ���п���������
void parallelQuickSort(std::vector<int>& arr, int low, int high, int depth) {
	// ������С��һ����С��ݹ���ȳ�����ֵʱ��ʹ�ô�������
	if (high - low < 1000 || depth > 3) {
		// ʹ�ñ�׼�Ŀ�������
		if (low < high) {
			int pi = partition(arr, low, high);
			parallelQuickSort(arr, low, pi - 1, depth + 1);
			parallelQuickSort(arr, pi + 1, high, depth + 1);
		}
		return;
	}

	// ����
	int pi = partition(arr, low, high);

	// �����첽�������������Ծ�߳���С������߳���
	bool can_create_thread = active_threads.load() < MAX_THREADS;

	// ��벿������
	std::future<void> left_future;
	if (can_create_thread) {
		active_threads++;
		left_future = std::async(std::launch::async, [&arr, low, pi, depth]() {
			parallelQuickSort(arr, low, pi - 1, depth + 1);
			active_threads--;
		});
	}
	else {
		// ������ܴ������̣߳����ڵ�ǰ�߳�������
		parallelQuickSort(arr, low, pi - 1, depth + 1);
	}

	// �Ұ벿������ʼ���ڵ�ǰ�߳�����ɣ�
	parallelQuickSort(arr, pi + 1, high, depth + 1);

	// �ȴ����������ɣ�������첽�ģ�
	if (left_future.valid()) {
		left_future.wait();
	}
}

// ���п�������ʵ��
void quickSort(std::vector<int>& arr, int low, int high) {
	if (low < high) {
		// ��ȡ������
		int pi = partition(arr, low, high);

		// �ݹ�ضԷ����������Ԫ�ؽ�������
		quickSort(arr, low, pi - 1);
		quickSort(arr, pi + 1, high);
	}
}

// ʹ�ö��߳̿�������ķ�����������
std::vector<int> processArrayParallelQuickSort(const std::vector<int>& input_array) {
	// ������������
	std::vector<int> sorted_array = input_array;

	// ��ʼ����Ծ�̼߳���
	active_threads = 0;

	// ʹ�ò��п��������㷨�������������
	parallelQuickSort(sorted_array, 0, sorted_array.size() - 1, 0);

	// ȥ���ظ�Ԫ��
	std::vector<int> result;
	result.reserve(sorted_array.size()); // Ԥ����ռ����������

	for (size_t i = 0; i < sorted_array.size(); ++i) {
		// �����ǰԪ����ǰһ��Ԫ�ز�ͬ���������ǵ�һ��Ԫ�أ�����ӵ������
		if (i == 0 || sorted_array[i] != sorted_array[i - 1]) {
			result.push_back(sorted_array[i]);
		}
	}

	return result;
}

// ʹ�ó����������ķ�����������
std::vector<int> processArrayQuickSort(const std::vector<int>& input_array) {
	// ������������
	std::vector<int> sorted_array = input_array;

	// ʹ�ÿ��������㷨�������������
	quickSort(sorted_array, 0, sorted_array.size() - 1);

	// ȥ���ظ�Ԫ��
	std::vector<int> result;
	for (size_t i = 0; i < sorted_array.size(); ++i) {
		// �����ǰԪ����ǰһ��Ԫ�ز�ͬ���������ǵ�һ��Ԫ�أ�����ӵ������
		if (i == 0 || sorted_array[i] != sorted_array[i - 1]) {
			result.push_back(sorted_array[i]);
		}
	}

	return result;
}
std::vector<int> processArray(const std::vector<int>& input) {
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
	std::vector<bool> exists(maxVal - minVal + 1, false);

	// 4. ������������д��ڵ�Ԫ��
	for (int num : input) {
		exists[num - minVal] = true;
	}

	// 5. ��������ȥ�ز����������
	std::vector<int> result;
	for (int i = 0; i < exists.size(); i++) {
		if (exists[i]) {
			result.push_back(i + minVal);
		}
	}

	return result;
}
// �����������
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
	// ����������飬��С�ͷ�Χ�ɵ���
	int array_size = 10000000; // һ�����Ԫ��
	int min_range = 0;
	int max_range = 10000000;

	std::vector<int> input = generateRandomArray(array_size, min_range, max_range);

	std::cout << "ԭʼ�����С: " << input.size() << " Ԫ��" << std::endl;

	// �����Ż��Ĳ��й��˷�������ʱ
	std::cout << "\n====== �Ż��Ĳ��й��˷��� ======" << std::endl;
	auto start_optimized = std::chrono::high_resolution_clock::now();
	std::vector<std::vector<int>> results_optimized = processArrayOptimized(input);
	auto end_optimized = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_optimized = end_optimized - start_optimized;
	std::cout << "�Ż��Ĳ��й��˷�����ʱ: " << elapsed_optimized.count() << " ����" << std::endl;

	// �ϲ��Ż����������н��
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
	std::cout << "\n====== ȥ������ ======" << std::endl;
	auto start_process_arry_sort = std::chrono::high_resolution_clock::now();
	std::vector<int> result_process_arry_sort = processArray(input);
	auto end_process_arry_sort = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_process_arry_sort = end_process_arry_sort - start_process_arry_sort;
	std::cout << "ȥ�����򷽷���ʱ: " << elapsed_process_arry_sort.count() << " ����" << std::endl;
	// ���Բ��п������򷽷�����ʱ
	std::cout << "\n====== ���п������򷽷� ======" << std::endl;
	auto start_parallel_quicksort = std::chrono::high_resolution_clock::now();
	std::vector<int> result_parallel_quicksort = processArrayParallelQuickSort(input);
	auto end_parallel_quicksort = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_parallel_quicksort = end_parallel_quicksort - start_parallel_quicksort;
	std::cout << "���п������򷽷���ʱ: " << elapsed_parallel_quicksort.count() << " ����" << std::endl;

	// ���Դ��п������򷽷�����ʱ
	std::cout << "\n====== ���п������򷽷� ======" << std::endl;
	auto start_quicksort = std::chrono::high_resolution_clock::now();
	std::vector<int> result_quicksort = processArrayQuickSort(input);
	auto end_quicksort = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> elapsed_quicksort = end_quicksort - start_quicksort;
	std::cout << "���п������򷽷���ʱ: " << elapsed_quicksort.count() << " ����" << std::endl;

	// �Ƚ����ַ����Ľ��
	std::cout << "\n====== ���ܱȽ� ======" << std::endl;
	std::cout << "�Ż��Ĳ��й��˷���Ԫ������: " << merged_result_optimized.size() << std::endl;
	std::cout << "���п������򷽷�Ԫ������: " << result_parallel_quicksort.size() << std::endl;
	std::cout << "���п������򷽷�Ԫ������: " << result_quicksort.size() << std::endl;
	std::cout << "ȥ�����򷽷�Ԫ������: " << result_process_arry_sort.size() << std::endl;
	// �����з����Ľ���Ա�Ƚ�
	std::sort(merged_result_optimized.begin(), merged_result_optimized.end());

	bool results_match1 = (merged_result_optimized == result_quicksort);
	bool results_match2 = (result_parallel_quicksort == result_quicksort);
	bool results_match3 = (result_process_arry_sort == result_quicksort);
	std::cout << "���й��˷����봮�п����������Ƿ�һ��: " << (results_match1 ? "��" : "��") << std::endl;
	std::cout << "���п��������봮�п����������Ƿ�һ��: " << (results_match2 ? "��" : "��") << std::endl;
	std::cout << "ȥ�������봮�п����������Ƿ�һ��: " << (results_match3 ? "��" : "��") << std::endl;

	// ������ٱ�
	double speedup1 = elapsed_quicksort.count() / elapsed_optimized.count();
	double speedup2 = elapsed_quicksort.count() / elapsed_parallel_quicksort.count();
	std::cout << "�Ż��Ĳ��з�������ڴ��п�������ļ��ٱ�: " << speedup1 << "��" << std::endl;
	std::cout << "���п�����������ڴ��п�������ļ��ٱ�: " << speedup2 << "��" << std::endl;

	return 0;
}