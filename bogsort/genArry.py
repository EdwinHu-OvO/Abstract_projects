import random
import os

def generate_unique_random_numbers(count, lower_bound, upper_bound, file_name):
    """
    生成指定数量的不重复随机数并保存到文件中。

    :param count: 要生成的随机数数量
    :param lower_bound: 随机数范围的下界（包含）
    :param upper_bound: 随机数范围的上界（包含）
    :param file_name: 保存结果的文件名
    """
    if count > (upper_bound - lower_bound + 1):
        raise ValueError("范围内的数字不足以生成指定数量的不重复随机数。")

    print("正在生成随机数...")
    random_numbers = random.sample(range(lower_bound, upper_bound + 1), count)
    print("随机数生成完成，开始写入文件。")

    # 写入文件
    try:
        with open(file_name, 'w') as file:
            for number in random_numbers:
                file.write(f"{number}\n")
        print(f"{count} 个不重复随机数已保存到 {file_name}。")
    except Exception as e:
        print(f"文件写入出错：{e}")

# 设置参数并调用函数
if __name__ == "__main__":
    print("想要生成的随机数数量：")
    total_numbers = int(input())
    try:
        # total_numbers = 100  # 要生成的随机数数量
        lower = 1  # 随机数下界
        upper = 1000000  # 随机数上界
        output_file = "arr.txt"  # 输出文件名

        generate_unique_random_numbers(total_numbers, lower, upper, output_file)

        # 检查文件是否存在
        if os.path.exists(output_file):
            print(f"文件 {output_file} 已生成，内容如下：")
            with open(output_file, 'r') as file:
                print(file.read())
        else:
            print("文件未生成，请检查程序逻辑。")
    except Exception as e:
        print(f"程序运行出错：{e}")
