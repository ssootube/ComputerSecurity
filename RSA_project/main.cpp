#include <iostream>
#include <random>
#include <ctime>
#include <string>
#define bit(num,index) ((num>>index)&0b1)
using namespace std;
void rand_without_duplicate(unsigned int arr[], unsigned int size,unsigned int min, unsigned int max);
bool MRPT(unsigned short num,unsigned int tester);
unsigned int mod_pow(unsigned int num, unsigned int pow, unsigned int mod);
unsigned int CRT(unsigned int a,unsigned short p,unsigned int b, unsigned short q, unsigned int c, unsigned short r, unsigned int n);
unsigned int EEA(unsigned int num, unsigned int mod);
void key_setup(unsigned int &n, unsigned short &p, unsigned short &q, unsigned short &r, unsigned int &e, unsigned int &d,unsigned int &totient);
void RSA_encoding(unsigned int &message, unsigned int e, unsigned int n);
void RSA_decoding(unsigned int &cipher, unsigned int d, unsigned short p, unsigned short q, unsigned short r,unsigned int n);
unsigned short pick_prime(int MAX);

int degree(unsigned int num) {//차수를 구한다.
	for (int i = 31; i >= 0; --i)
		if ((num & (1 << i)) != 0) return i;
}
void swap(unsigned int &x, unsigned int &y) {//x,y를 교환한다.
	unsigned char temp = x;
	x = y;
	y = temp;
}
int main() {
	srand(time(NULL));

	//키 세팅 과정
	unsigned int n, e, d, totient;
	unsigned short p, q, r;
	key_setup(n, p, q, r, e, d,totient);
	cout << "p = " << p << "\n" << "q = " << q << "\n" << "r = " << r << "\n" << "N = " << n << "\n" << "phi = " << totient << "\n" << "e = " << e << "\n" << "d = " << d << "\n";
	//입력 과정
	cout << "\nMessage Input : ";
	string input;
	cin >> input;
	unsigned int message = stoi(input);
	cout << "Message : " << message << "\n";
	//Encoding
	cout << "\n**Encryption\n";
	RSA_encoding(message, e, n);
	cout << "cipher : " << message<<"\n";
	//Decoding
	cout << "\n**Decryption\n";
	RSA_decoding(message, d, p, q, r,n);
	cout << "dycrypted cipher : " << message << "\n";
	system("pause");
}

bool MRPT(unsigned short num,unsigned int tester)//Miller Rabin Primality Test. 합성수이면 false, 결정할 수 없으면 true를 반환한다.
{
	int q = num - 1;
	int k;
	for (k = 1; k < 16; ++k)
		if (((q) & (1 << k)) != 0) break; // shift연산을 이용해 계속 반으로 나누다가 홀수일 때 break 한다.
	q = q >> k;
	unsigned int temp = mod_pow(tester, q, num);
	if (temp == 1 || temp == num-1) return true;
	for (int i = 1; i < k; ++i) {
		if (mod_pow(tester, (1<<i)*q, num) == num-1) return true;
	}
	return false;
}

unsigned int mod_pow(unsigned int num, unsigned int pow, unsigned int mod)//모듈러 거듭제곱을 계산한다.
{
	if (pow == 0) return 1; //0일 경우에는 1을 반환한다.
	int deg = degree(pow);//최고차항을 구한다.
	unsigned long long result = num;
	for (int i = deg; i > 0 ; --i) {
		result = (result*result) % mod;
		if (bit(pow, i-1) == 1) {
			result = (result*num) % mod;
		}
	}
	return result;
}

unsigned int CRT(unsigned int a, unsigned short p, unsigned int b, unsigned short q, unsigned int c, unsigned short r, unsigned int n) //Chinese Remainder Theorem
{
	// x=a(mod p) , x=b(mod q), x=c(mod r)일 때 x(mod pqr)의 값을 구한다.
	unsigned long long result = 0;
	unsigned int A = (a * EEA(q*r, p))%p; // A = a*(qr)^(-1) (mod p)
	unsigned int B = (b * EEA(p*r, q))%q; // B = b*(pr)^(-1) (mod q)
	unsigned int C = (c * EEA(p*q, r))%r; // C = c*(pq)^(-1) (mod r)
	result = (A*q*r) + (B*p*r) + (C*p*q);
	return result%n;
}

unsigned int EEA(unsigned int num, unsigned int mod)//Extended Euclidean Algorithm. num의 모듈러 mod에 대한 역원을 반환한다.
{
	unsigned int  a1 = 0, a2 = mod;
	unsigned int result=1, b=num;

	while (true) {
		if (b == 0) return 0;//역원 없음
		if (b== 1) {
			while (result > mod) result += mod; //오버플로우를 이용하여 모듈러 이하로 맞춘다.
			return result;
		}
		unsigned Q = a2 / b;
		unsigned int  temp1 = a1 - Q * result, temp2 = a2 - Q * b;
		 a1 = result; a2 = b;
		result = temp1; b = temp2;
	}
}

void key_setup(unsigned int &n,unsigned short &p, unsigned short &q , unsigned short &r, unsigned int &e, unsigned int &d, unsigned int &totient)
{
	p = pick_prime(1023);
	q = pick_prime(1023);
	r = pick_prime(1023);
	while ((p == q) || (p == r) || (q == r)) {//p,q,r에 중복이 없도록 소수를 뽑는다.
		if(q == r) 	q = pick_prime(1023);
		else p = pick_prime(1023);
	}
	n = p*q*r;
	totient = (p - 1)*(q - 1)*(r - 1);

	do {
		e = rand() % (totient - 1) + 1; //1부터 totient-1 중에 하나를 랜덤으로 택한다.
		d = EEA(e, totient);
	} while (d == 0); //totient와 서로소일 때 까지 반복한다. 역원이 존재하지 않으면 서로소가 아니라는 의미이다.

}

void RSA_encoding(unsigned int & message, unsigned int e, unsigned int n)
{
	message = mod_pow(message, e, n);
}

void RSA_decoding(unsigned int &cipher, unsigned int d, unsigned short p, unsigned short q, unsigned short r, unsigned int n)
{
	//CRT를 이용하기 위해 각각의 모듈러 거듭제곱을 구하자.
	unsigned int a = mod_pow(cipher, d, p);
	unsigned int b = mod_pow(cipher, d, q);
	unsigned int c = mod_pow(cipher, d, r);
	cipher = CRT(a, p, b, q, c, r, n);
}

unsigned short pick_prime(int MAX)//랜덤으로 MAX이하의 소수를 리턴한다. 단, 이때 리턴한 소수는 밀러 라빈으로 체크하기 때문에 소수가 아닐 확률도 있다.
{
	//밀러 라빈을 20번 수행할 랜덤 변수
	unsigned int random_arr[20];

	//결과값을 저장할 변수
	unsigned short result;
	while (true) {
	pick_prime_again:
		result = rand() % (MAX-1)+2;		//수를 랜덤하게 잡고
		rand_without_duplicate(random_arr, 20, 2, result-1);
		bool is_prime = true;
		for (int i = 0; i < 20; ++i) { //밀러 라빈을 20회 수행한다.
			is_prime &= MRPT(result, random_arr[i]); //'&연산'을 이용해 20회 모두 참일 때 소수라고 간주한다.
			if (!is_prime) goto pick_prime_again;
		}
		break;
	}
	return result;
}

void rand_without_duplicate(unsigned int arr[], unsigned int size,unsigned int min,unsigned int max)//min부터 max까지의 수 중에서 size개의 중복없는 난수를 생성하여 arr에 배열로 저장한다
{
	if (size > max) return; //범위보다 많은 난수를 뽑으면 중복이 없을 수가 없다.
	for (int i = 0; i < size; ++i) {
	rand_without_duplicate_again:
		arr[i] = rand() % (max+1-min)+min;
		for (int j = 0; j < i; ++j) if (arr[j] == arr[i]) goto rand_without_duplicate_again;
	}
}
