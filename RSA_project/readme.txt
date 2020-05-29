1) 구현환경 및 구현언어
C++를 이용해 비주얼 스튜디오에서 콘솔 프로그램으로 구현하였다.

2) RSA 암호화
가장 먼저 키 셋업 과정을 만들었다. 소수 p,q,r을 랜덤으로 뽑기 위해 아래 세 함수를 만들었다.

bool MRPT(unsigned short num,unsigned int tester);//밀러 라빈
unsigned int mod_pow(unsigned int num, unsigned int pow, unsigned int mod);//모듈러 거듭제곱
unsigned short pick_prime(int MAX);//소수 선택 알고리즘

Mod_pow()의 경우에는 successive squaring방법을 이용해 모듈러 거듭제곱을 구하는 알고리즘이다.
지수를 이진수로 보고, 각 비트가 0일 경우에는 제곱만 수행하고, 0일 경우에는 제곱과 한번의 곱셈까지 수행하도록 구현하였다.
이때, 최고차항을 알아낼 필요가 있기 때문에 아래의 간단한 함수를 추가적으로 구현하였다.

int degree(unsigned int num) {//차수를 구한다.
	for (int i = 31; i >= 0; --i)
		if ((num & (1 << i)) != 0) return i;
}

그리고 MRPT는 밀러 라빈 알고리즘을 수행한다.
파라미터로 들어온 num가 소수인지 아닌지 판별한다.
이때 파라미터의 tester^q(mod num) 값이 -1(mod num) 혹은 1이거나
(2^i)*(tester^q) (mod num)값이 -1(mod num) 인 경우 소수라고 판별하여, true를 리턴하도록 구현하였다.
이때 물론 모듈러 거듭제곱은 위에서 만들어둔 mod_pow()를 이용하였다.

그 후, MRPT()를 이용해 p,q,r을 선택하는 pick_prime()함수를 구현하였다.
이때 밀러 라빈을 총 20번 수행하게 되는데, 이때마다 사용되는 strong witness로 사용되는 tester의 값이 중복이 된다면
안전성이 떨어지게 되므로 아래와 같이 중복없이 난수를 생성하는 함수를 추가적으로 구현하였다.

void rand_without_duplicate(unsigned int arr[], unsigned int size,unsigned int min, unsigned int max);

이렇게 생성한 소수 p,q,r이 서로 중복이 된다면 키로 사용할 수 없으므로 key_setup() 과정에서
만약 중복일 경우 다시 뽑도록 구현하였다.
그 후에 totient = (p-1)(q-1)(r-1)값을 계산한 뒤에 e를 랜덤으로 택하였다.
이때 d를 뽑기 위해 아래의 확장 유클리드 알고리즘을 이용했다.

unsigned int EEA(unsigned int num, unsigned int mod);

위 함수는 num의 mod에 대한 역원을 계산하는 함수이고, 만약 역권이 존재하지 않으면 0을 리턴한다.
이때, 역원이 존재하지 않는 다는 것은 num과 mod의 gcd가 1이 아니라는 의미이며, 즉 서로소가 아니라는 의미이다.
따라서 d=EEA(e,totient)를 통해 d값을 구할 때,
d가 만약 0 이 된다면 e와 totient가 서로소가 아니라는 의미이므로 e를 잘못 택한 것이다.
따라서 d가 0이 아닐 때까지 while문을 돌며 e를 택하도록 구현하였다.
이렇게 sey_Setup()가 완료되면 그 후 암호화 과정은 아래와 같이 굉장히 단순하게 구현하였다.

void RSA_encoding(unsigned int & message, unsigned int e, unsigned int n)
{
	message = mod_pow(message, e, n);
}

3) RSA 복호화
복호화 과정에서는 이미 암호화 과정 이전에 만들어두었던 키들을 사용한다.
CRT를 이용해 복호화를 하기 위해 아래와 같은 함수를 정의하였다.

unsigned int CRT(unsigned int a,unsigned short p,unsigned int b, unsigned short q, unsigned int c, unsigned short r, unsigned int n);

위 함수는 x=a(mod p), x=b(mod q) , x= c (mod r) 일 때 x의 값이 mod n으로 몇인지 구하는 함수이다. 단, n=pqr이다.
이때  x=Aqr+Bpr+Cpq (mod n) 으로 값이 계산되도록 구현하였으며, 이때 A,B,C는 아래와 같다.

A=a*(qr)^(-1) (mod p)
B=b*(pr)^(-1) (mod q)
C=c*(pq)^(-1) (mod r)

그리고 복호화 과정은 위의 CRT함수를 이용하여 아래와 같이 간단히 구현하였다.

void RSA_decoding(unsigned int &cipher, unsigned int d, unsigned short p, unsigned short q, unsigned short r, unsigned int n)
{
	//CRT를 이용하기 위해 각각의 모듈러 거듭제곱을 구하자.
	unsigned int a = mod_pow(cipher, d, p);
	unsigned int b = mod_pow(cipher, d, q);
	unsigned int c = mod_pow(cipher, d, r);
	cipher = CRT(a, p, b, q, c, r, n);
}
