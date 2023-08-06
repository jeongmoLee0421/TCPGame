#include <iostream>
#include <algorithm>

// 2023 08 06 이정모 home

// key는 같은데 value가 다른 데이터를
// multimap에 삽입해서 사용하려고 했다.
// 같은 key에 대해서 여러 데이터를 탐색할 때
// equal_range()를 사용하는데
// equal_range()의 반환 값이 lower_bound와 upper_bound로 계산된다길래
// 궁금해서 구현해보았다.

int* upper_bound(int* arr, int start, int end, int key)
{
	// 범위를 좁히다가
	// 인덱스 하나로 좁혀지면
	// 탈출 조건
	if (start == end)
	{
		// key보다 크다면 그 위치 반환
		if (arr[start] > key)
		{
			return &arr[start];
		}
		// key보다 작거나 같다면
		// 그 다음위치를 반환하는데
		// 다음 위치가 유효한 위치라는 것은
		// key를 초과하는 첫번째 값이라는 것이고
		// 유효한 위치가 아니라면
		// key를 초과하는 수가 없다는 뜻
		else
		{
			return &arr[start + 1];
		}
	}

	int index = (start + end) / 2;
	int num = arr[index];

	// num이 key 이하면
	// 초과인 값을 찾기 위해 오른쪽 탐색
	if (num <= key)
	{
		return upper_bound(arr, index + 1, end, key);
	}
	// num이 key 초과면
	// num보다 작거나 같으면서 key 초과인 값이 있는지
	// 왼쪽 탐색
	else
	{
		return upper_bound(arr, start, index - 1, key);
	}
}

// 찾고자하는 key 초과의 값이 처음 나오는 위치
int* upper_bound(int* arr, int size, int key)
{
	int start = 0;
	int end = size - 1;

	while (start < end)
	{
		int index = (start + end) / 2;
		int num = arr[index];

		// num이 key보다 작다면
		// 초과인 값을 찾기 위해 오른쪽 탐색
		if (num < key)
		{
			start = index + 1;
		}
		// num이 key와 같다면
		// 초과인 값을 찾기 위해 오른쪽 탐색
		else if (num == key)
		{
			start = index + 1;
		}
		// num이 key보다 크다면
		// 왼쪽에 num보다 작거나 같으면서 key보다 큰 값을 찾기 위해
		// 왼쪽 탐색
		else
		{
			end = index - 1;
		}
	}

	// 범위를 좁히다가
	// 최종 위치 값이 key보다 크다면
	// upper_bound 성공
	if (arr[start] > key)
	{
		return &arr[start];
	}
	// 최종 위치 값이 key보다 작거나 같은 경우
	// 위치가 인덱스 범위 내부(0 ~ size-1)인 경우 성공
	// 인덱스 끝(size)인 경우 실패
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