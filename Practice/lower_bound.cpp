#include <iostream>
#include <algorithm>

// 2023 08 06 이정모 home

// key는 같은데 value가 다른 데이터를
// multimap에 삽입해서 사용하려고 했다.
// 같은 key에 대해서 여러 데이터를 탐색할 때
// equal_range()를 사용하는데
// equal_range()의 반환 값이 lower_bound와 upper_bound로 계산된다길래
// 궁금해서 구현해보았다.

int* lower_bound(int* arr, int start, int end, int key)
{
	// 범위를 좁혀나가다가
	// 시작과 끝이 같아지는 순간이
	// 탈출 조건
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

	// 범위를 좁히면서 재귀적으로 호출하다가
	// 탈출 조건에 맞아서 값이 return되면
	// 그 값을 계속해서 호출원에게 return 해준다.
	if (num < key)
	{
		return lower_bound(arr, index + 1, end, key);
	}
	else
	{
		return lower_bound(arr, start, end - 1, key);
	}
}

// 찾고자하는 key 이상의 값이 처음 나오는 위치
int* lower_bound(int* arr, int size, int key)
{
	int start = 0;
	int end = size - 1;

	while (start < end)
	{
		// n개의 데이터에서 중간에 있는 데이터와 비교하여
		// 절반씩 쳐내기 때문에 O(log2N)
		int index = (start + end) / 2;
		int num = arr[index];

		// key 미만이면
		// 이상인 값을 찾아야 하기 떄문에 오른쪽 탐색
		if (num < key)
		{
			start = index + 1;
		}
		// key와 같다면
		// 같은 수가 왼쪽에 배치되어 있을 수도 있기 때문에
		// 왼쪽 탐색
		else if (num == key)
		{
			end = index - 1;
		}
		// key보다 크다면
		// 왼쪽에 num보다 작으면서 이상인 수가 있을 수 있기 때문에
		// 왼쪽 탐색
		else
		{
			end = index - 1;
		}
	}

	// 범위를 좁히고 좁히다가
	// 결국 최종값이 key보다 작다면
	// key 다음 위치를 반환
	// 인덱스가 0 ~ size-1 이면 성공
	// 인덱스가 size면 실패
	if (arr[start] < key)
	{
		return &arr[start + 1];
	}
	// 최종값이
	// 이미 key 이상인 값이면
	// 그 위치를 반환하면 된다.
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