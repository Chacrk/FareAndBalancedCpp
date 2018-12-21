import random


file_path = 'data/fare.txt'
f_ = open(file_path, 'w')
f_.writelines('')
f_.close()

f = open(file_path, 'a')
count_of_case = 10
max_c = 1000


def make_a_case(case_index, N_, R_):
    if N_ is None and R_ is None:
        print('N and R is None')
        # 默认N与R随机
        N = random.randint(2, 50000)
        R = random.randint(N - 1, 50000)
    else:
        N = N_
        R = R_
    index_of_R = 0
    print('Case {}\nN: {}, R: {}'.format(case_index, N, R))
    f.writelines('{} {}\n'.format(N, R))
    # 连通 1- N
    for i in range(0, N - 1):
        x, y = (i + 1), (i + 2)
        f.writelines('{} {} {}\n'.format(x, y, random.randint(1, max_c)))
        index_of_R += 1
    if R - index_of_R > 0:
        for i in range(0, R - index_of_R):
            x_random = random.randint(1, N - 1)
            while True:
                y_random = random.randint(2, N)
                if y_random > x_random:
                    break
            f.writelines('{} {} {}\n'.format(x_random, 
                                             y_random, 
                                             random.randint(1, max_c)))


def main_fun():
    print('start.')
    for i in range(count_of_case):
        make_a_case(i + 1, 50000, 50000)
        # make_a_case(i+1, None, None)
    f.writelines('0 0')
    f.close()
    print('end.')


if __name__ == '__main__':
    main_fun()