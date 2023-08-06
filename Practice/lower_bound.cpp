#include <iostream>
#include <algorithm>

// 2023 08 06 ������ home

// key�� ������ value�� �ٸ� �����͸�
// multimap�� �����ؼ� ����Ϸ��� �ߴ�.
// ���� key�� ���ؼ� ���� �����͸� Ž���� ��
// equal_range()�� ����ϴµ�
// equal_range()�� ��ȯ ���� lower_bound�� upper_bound�� ���ȴٱ淡
// �ñ��ؼ� �����غ��Ҵ�.

int* lower_bound(int* arr, int start, int end, int key)
{
	// ������ ���������ٰ�
	// ���۰� ���� �������� ������
	// Ż�� ����
	if (start == end)
	{
		if (arr[start] < key)
		{
			return &arr[start + 1];
		}
		else
		{
			return &arr[start];
		}
	}

	int index = (start + end) / 2;
	int num = arr[index];

	// ������ �����鼭 ��������� ȣ���ϴٰ�
	// Ż�� ���ǿ� �¾Ƽ� ���� return�Ǹ�
	// �� ���� ����ؼ� ȣ������� return ���ش�.
	if (num < key)
	{
		return lower_bound(arr, index + 1, end, key);
	}
	else
	{
		return lower_bound(arr, start, end - 1, key);
	}
}

// ã�����ϴ� key �̻��� ���� ó�� ������ ��ġ
int* lower_bound(int* arr, int size, int key)
{
	int start = 0;
	int end = size - 1;

	while (start < end)
	{
		// n���� �����Ϳ��� �߰��� �ִ� �����Ϳ� ���Ͽ�
		// ���ݾ� �ĳ��� ������ O(log2N)
		int index = (start + end) / 2;
		int num = arr[index];

		// key �̸��̸�
		// �̻��� ���� ã�ƾ� �ϱ� ������ ������ Ž��
		if (num < key)
		{
			start = index + 1;
		}
		// key�� ���ٸ�
		// ���� ���� ���ʿ� ��ġ�Ǿ� ���� ���� �ֱ� ������
		// ���� Ž��
		else if (num == key)
		{
			end = index - 1;
		}
		// key���� ũ�ٸ�
		// ���ʿ� num���� �����鼭 �̻��� ���� ���� �� �ֱ� ������
		// ���� Ž��
		else
		{
			end = index - 1;
		}
	}

	// ������ ������ �����ٰ�
	// �ᱹ �������� key���� �۴ٸ�
	// key ���� ��ġ�� ��ȯ
	// �ε����� 0 ~ size-1 �̸� ����
	// �ε����� size�� ����
	if (arr[start] < key)
	{
		return &arr[start + 1];
	}
	// ��������
	// �̹� key �̻��� ���̸�
	// �� ��ġ�� ��ȯ�ϸ� �ȴ�.
	else
	{
		return &arr[start];
	}
}

int main()
{
	int arr[]{ 1,1,1,2,2,2,3,3,3 };

	int size = sizeof(arr) / sizeof(arr[0]);
	int key = 3;

	int* p1 = lower_bound(arr, size, key);
	printf("%lld\n", p1 - arr);

	int* p2 = lower_bound(arr, 0, size - 1, key);
	printf("%lld\n", p2 - arr);

	int* p3 = std::lower_bound(std::begin(arr), std::end(arr), key);
	printf("%lld\n", p3 - arr);
}