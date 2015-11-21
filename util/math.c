
unsigned int digits(int num)
{
	unsigned int cnt = 0;
	int n = num;

	if (num < 0)
		cnt++;

	do {
		cnt++;
		n /= 10;
	}
	while (n != 0);
	return cnt;
}
