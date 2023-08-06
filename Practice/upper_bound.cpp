#include <iostream>
#include <algorithm>

// 2023 08 06 ������ home

// key�� ������ value�� �ٸ� �����͸�
// multimap�� �����ؼ� ����Ϸ��� �ߴ�.
// ���� key�� ���ؼ� ���� �����͸� Ž���� ��
// equal_range()�� ����ϴµ�
// equal_range()�� ��ȯ ���� lower_bound�� upper_bound�� ���ȴٱ淡
// �ñ��ؼ� �����غ��Ҵ�.

int* upper_bound(int* arr, int start, int end, int key)
{
	// ������ �����ٰ�
	// �ε��� �ϳ��� ��������
	// Ż�� ����
	if (start == end)
	{
		// key���� ũ�ٸ� �� ��ġ ��ȯ
		if (arr[start] > key)
		{
			return &arr[start];
		}
		// key���� �۰ų� ���ٸ�
		// �� ������ġ�� ��ȯ�ϴµ�
		// ���� ��ġ�� ��ȿ�� ��ġ��� ����
		// key�� �ʰ��ϴ� ù��° ���̶�� ���̰�
		// ��ȿ�� ��ġ�� �ƴ϶��
		// key�� �ʰ��ϴ� ���� ���ٴ� ��
		else
		{
			return &arr[start + 1];
		}
	}

	int index = (start + end) / 2;
	int num = arr[index];

	// num�� key ���ϸ�
	// �ʰ��� ���� ã�� ���� ������ Ž��
	if (num <= key)
	{
		return upper_bound(arr, index + 1, end, key);
	}
	// num�� key �ʰ���
	// num���� �۰ų� �����鼭 key �ʰ��� ���� �ִ���
	// ���� Ž��
	else
	{
		return upper_bound(arr, start, index - 1, key);
	}
}

// ã�����ϴ� key �ʰ��� ���� ó�� ������ ��ġ
int* upper_bound(int* arr, int size, int key)
{
	int start = 0;
	int end = size - 1;

	while (start < end)
	{
		int index = (start + end) / 2;
		int num = arr[index];

		// num�� key���� �۴ٸ�
		// �ʰ��� ���� ã�� ���� ������ Ž��
		if (num < key)
		{
			start = index + 1;
		}
		// num�� key�� ���ٸ�
		// �ʰ��� ���� ã�� ���� ������ Ž��
		else if (num == key)
		{
			start = index + 1;
		}
		// num�� key���� ũ�ٸ�
		// ���ʿ� num���� �۰ų� �����鼭 key���� ū ���� ã�� ����
		// ���� Ž��
		else
		{
			end = index - 1;
		}
	}

	// ������ �����ٰ�
	// ���� ��ġ ���� key���� ũ�ٸ�
	// upper_bound ����
	if (arr[start] > key)
	{
		return &arr[start];
	}
	// ���� ��ġ ���� key���� �۰ų� ���� ���
	// ��ġ�� �ε��� ���� ����(0 ~ size-1)�� ��� ����
	// �ε��� ��(size)�� ��� ����
	else
	{
		return &arr[start + 1];
	}
}

int main()
{
	int arr[]{ 1,1,1,2,2,2,3,3,3 };

	int size = sizeof(arr) / sizeof(arr[0]);
	int key = 2;

	int* p1 = upper_bound(arr, size, key);
	printf("%lld\n", p1 - arr);

	int* p2 = upper_bound(arr, 0, size - 1, key);
	printf("%lld\n", p2 - arr);

	int* p3 = std::upper_bound(std::begin(arr), std::end(arr), key);
	printf("%lld\n", p3 - arr);
}