import threading
import time

def sleep_and_append(l, n):
    time.sleep(n*0.001)
    l.append(n)
def sleep_sort(arr):
    sorted_arr = []
    threads = []
    for n in arr:
        t = threading.Thread(target=sleep_and_append, args=(sorted_arr, n))
        threads.append(t)
        t.start()
    for t in threads:
        t.join()
    return sorted_arr

def read_numbers_from_file(file_name):
    """
    从指定文件中读取数字并存放到数组中。

    :param file_name: 包含数字的文件名
    :return: 包含文件中数字的数组
    """
    numbers = []
    try:
        with open(file_name, 'r') as file:
            for line in file:
                # 去除每行的空白字符并转为整数
                numbers.append(int(line.strip()))
        print(f"从 {file_name} 中读取了 {len(numbers)} 个数字：")
        print(numbers)  # 输出读取的数组内容
        return numbers
    except FileNotFoundError:
        print(f"文件 {file_name} 未找到，请检查文件路径。")
    except ValueError as e:
        print(f"文件内容格式错误：{e}")
    return []

input_file = "arr.txt"  # 要读取的文件名
arr = read_numbers_from_file(input_file)

# 验证数组是否已成功加载
if arr:
    print("新数组已成功创建！")
else:
    print("未能从文件中加载数据，请检查问题。")
print(sleep_sort(arr))